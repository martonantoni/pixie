#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include <regex>

cPixelShader::~cPixelShader()
{
    if (mShader)
        mShader->Release();
}

bool cPixelShader::compile(std::string_view sourceCode)
{
    std::string finalSourceCode = extractMetaInfo(sourceCode);
    for (auto&& [idx, paramName] : mParameterNames | std::views::enumerate)
    {
        if (paramName.empty())
            continue;
        const std::regex pattern("\\binput\\s*\\.\\s*" + paramName + "\\b");
        auto replacement = "input.params."s + "xyzw"[idx];
        finalSourceCode = std::regex_replace(finalSourceCode, pattern, replacement);
    }
    if (auto blob = cShader::compile(finalSourceCode, "PSMain", "ps_5_0"))
    {
        auto device = cDevice::Get();
        if(mShader)
            mShader->Release();
        mShader = nullptr;
        D3V(device->GetD3DObject()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mShader));
        {
            cLog debugLog(std::format("debug/{}_ps.txt", name()), cLog::Flags::TRUNCATE);
            dumpDebugInfo(debugLog, blob, true);
        }
        return true;
    }
    return false;
}
 
std::string cPixelShader::extractMetaInfo(std::string_view sourceCode)
{
    std::string finalSourceCode;

    // meta data is in the form of comments like this:
    // @cfg
    // lua code (does not need leading //)
    // @endcfg

    mParameterNames.fill({});

    std::string configSource;
    bool inConfig = false;
    int cfgPos = 0;

    for (auto&& lineRange : sourceCode | std::views::split('\n'))
    {
        std::string line(lineRange.begin(), lineRange.end());

        auto commentPos = line.find("//");
        if (commentPos != std::string::npos)
        {
            std::string_view comment(line.data() + commentPos + 2, line.size() - commentPos - 2);

            if (comment.find("@cfg") != std::string_view::npos)
            {
                inConfig = true;
                cfgPos = static_cast<int>(finalSourceCode.size());
                continue;
            }
            if (comment.find("@endcfg") != std::string_view::npos)
            {
                inConfig = false;
                continue;
            }
            if (inConfig)
            {
                line = std::string(comment);
            }
        }
        if (inConfig)
        {
            configSource.append(line);
            configSource += '\n';
            finalSourceCode += '\n'; // keep line numbers consistent for error reporting
        }
        else
        {
            finalSourceCode.append(line);
            finalSourceCode += '\n';
        }
    }

    auto state = std::make_shared<cLuaState>();
    state->executeString(configSource);
    std::vector<cShaderConstant> constants;
    auto rootTable = state->globalTable();
    for (auto&& [key, value] : rootTable)
    {
        if (key == "config" || key == "configuration")
        {
            mConfig = Pixie::toConfig(value);
        }
        else if (key == "texture_slots")
        {
            for (auto&& [idx, slotValue] : value | std::views::values | std::views::take(mTextureSlotNames.size()) | std::views::enumerate)
            {
                mTextureSlotNames[idx] = slotValue.toString();
            }
        }
        else if (key == "parameters")
        {
            for (auto&& [idx, paramValue] : value | std::views::values | std::views::take(mParameterNames.size()) | std::views::enumerate)
            {
                mParameterNames[idx] = paramValue.toString();            
            }
        }
        else if (key == "textures")
        {
            for (auto&& [idx, textureValue] : value | std::views::values | std::views::take(mDefaultTextures.size()) | std::views::enumerate)
            {
                mDefaultTextures[idx] = textureValue.toString();
            }
        }
        else if (key == "data")
        {
            auto slot = value.get<int>("slot");
            if (slot >= 0 && slot < mDataTextures.size() && value.has("floats"))
            {
                auto floatsTable = value.get("floats");
                auto tableSize = floatsTable.arraySize();
                if (tableSize == 0)
                    continue;
                std::vector<float> floats(tableSize);
                for (int i = 0; i < tableSize; ++i)
                {
                    floats[i] = floatsTable.get<double>(i + 1);
                }
                mDataTextures[slot] = cTexture::CreateFromData(cPoint(floats.size(), 1), floats);
            }
        }
        else if (key == "constants")
        {
            for (auto&& [constantName, constantValue] : value)
            {
                if (constantValue.isTable())
                {
                    auto type = constantValue.get<std::string>("type");
                    if (type == "float")
                    {
                        float floatValue = constantValue.get<double>("default");
                        constants.emplace_back( constantName.toString(),  floatValue );
                    }
                    else if (type == "color")
                    {
                        std::array<float, 3> colorValue;
                        auto colorTable = constantValue.get("default");
                        for (int i = 0; i < 3; ++i)
                        {
                            colorValue[i] = colorTable.get<double>(i + 1);
                        }
                        constants.emplace_back( constantName.toString(),  colorValue );
                    }
                }
            }
        }
    }
    auto presetConstants = theShaderManager->shaderConstants(name());
    for (auto&& presetConstant : presetConstants)
    {
        auto it = std::find_if(constants.begin(), constants.end(), [&](const cShaderConstant& c) { return c.mName == presetConstant.mName; });
        if (it == constants.end())
            constants.push_back(presetConstant);
        else
            *it = presetConstant; 
    }
    std::string constantDeclarations;
    for(auto&& constant : constants)
    {
        if (std::holds_alternative<float>(constant.mValue))
        {
            constantDeclarations += std::format("static const float {} = {}; ", constant.mName, std::get<float>(constant.mValue));
        }
        else if (std::holds_alternative<std::array<float, 3>>(constant.mValue))
        {
            auto color = std::get<std::array<float, 3>>(constant.mValue);
            constantDeclarations += std::format("static const float3 {} = float3({}, {}, {}); ", constant.mName, color[0], color[1], color[2]);
        }
    }
    finalSourceCode.insert(cfgPos, constantDeclarations);
    return finalSourceCode;
}

int cPixelShader::parameterIndex(std::string_view name) const
{
    auto it = std::find(mParameterNames.begin(), mParameterNames.end(), name);
    if (it == mParameterNames.end())
    {
        throw std::runtime_error(std::format("Parameter name '{}' not found in pixel shader", name));
    }
    return static_cast<int>(std::distance(mParameterNames.begin(), it));
}

int cPixelShader::textureSlotIndex(std::string_view name) const
{
    auto it = std::find(mTextureSlotNames.begin(), mTextureSlotNames.end(), name);
    if (it == mTextureSlotNames.end())
    {
        throw std::runtime_error(std::format("Texture slot name '{}' not found in pixel shader", name));
    }
    return static_cast<int>(std::distance(mTextureSlotNames.begin(), it));
}