#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include <regex>

cPixelShader::~cPixelShader()
{
    if (mShader)
        mShader->Release();
}

bool cPixelShader::compile(std::string sourceCode, std::string_view entryPoint, cLuaObject metaData)
{
    sourceCode = processMetaData(std::move(sourceCode), metaData);
    for (auto&& [idx, paramName] : mParameterNames | std::views::enumerate)
    {
        if (paramName.empty())
            continue;
        const std::regex pattern("\\binput\\s*\\.\\s*" + paramName + "\\b");
        auto replacement = "input.params."s + "xyzw"[idx];
        sourceCode = std::regex_replace(sourceCode, pattern, replacement);
    }
    if (auto blob = cShader::compile(sourceCode, entryPoint.data(), "ps_5_0"))
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
 
std::string cPixelShader::processMetaData(std::string sourceCode, cLuaObject metaData)
{
    mConfig.reset();
    std::vector<cShaderConstant> constants;
    mParameterNames.fill({});
    if (metaData.has("config"))
    {
        mConfig = Pixie::toConfig(metaData.get("config"));
    }
    if (metaData.has("configuration"))
    {
        mConfig = Pixie::toConfig(metaData.get("configuration"));
    }
    if (metaData.has("texture_slots"))
    {
        auto textureSlots = metaData.get("texture_slots");
        for (auto&& [idx, slotValue] : textureSlots | std::views::values | std::views::take(mTextureSlotNames.size()) | std::views::enumerate)
        {
            mTextureSlotNames[idx] = slotValue.toString();
        }
    }
    if (metaData.has("parameters"))
    {
        auto parameters = metaData.get("parameters");
        for (auto&& [idx, paramValue] : parameters | std::views::values | std::views::take(mParameterNames.size()) | std::views::enumerate)
        {
            mParameterNames[idx] = paramValue.toString();
        }
    }
    if (metaData.has("textures"))
    {
        auto textures = metaData.get("textures");
        for (auto&& [idx, textureValue] : textures | std::views::values | std::views::take(mDefaultTextures.size()) | std::views::enumerate)
        {
            mDefaultTextures[idx] = textureValue.toString();
        }
    }
    if (metaData.has("data"))
    {
        auto data = metaData.get("data");
        for (auto&& [key, value] : data)
        {
            if (value.isTable())
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
        }
    }
    if (metaData.has("constants"))
    {
        auto constantsTable = metaData.get("constants");
        for (auto&& [constantName, constantValue] : constantsTable)
        {
            if (constantValue.isTable())
            {
                auto type = constantValue.get<std::string>("type");
                if (type == "float")
                {
                    float floatValue = constantValue.get<double>("value");
                    constants.emplace_back(constantName.toString(), floatValue);
                }
                else if (type == "color")
                {
                    std::array<float, 3> colorValue;
                    auto colorTable = constantValue.get("value");
                    for (int i = 0; i < 3; ++i)
                    {
                        colorValue[i] = colorTable.get<double>(i + 1);
                    }
                    constants.emplace_back(constantName.toString(), colorValue);
                }
            }
        }
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
    auto cfgPos = sourceCode.find(mMetaInsertionPointText);
    if (cfgPos != std::string::npos)
    {
        sourceCode.insert(cfgPos, constantDeclarations);
    }
    if (!mConfig)
    {
        mConfig = std::make_shared<cConfig>();
    }
    return sourceCode;
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