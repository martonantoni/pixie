#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include <regex>
#include <d3d11shader.h>
#include <d3dcompiler.h>

cPixelShader::~cPixelShader()
{
    if (mShader)
        mShader->Release();
}

bool cPixelShader::compile(cShaderSource& shaderSource)
{
    processMetaData(shaderSource);
    for (auto&& [idx, paramName] : mParameterNames | std::views::enumerate)
    {
        if (paramName.empty())
            continue;
        shaderSource.regexReplace(
            "\\binput\\s*\\.\\s*" + paramName + "\\b", 
            "input.params."s + "xyzw"[idx]);
    }
    std::string entryPoint = shaderSource.metaData().get<std::string>("entry_point", "PSMain"s);
    if (auto blob = cShader::compile(shaderSource.sourceCode(), entryPoint.c_str(), "ps_5_0"))
    {
        if (!reflectTextures(blob))
            return false;

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


bool cPixelShader::reflectTextures(ID3DBlob* shader)
{

    ID3D11ShaderReflection* reflection = nullptr;

    HRESULT hr = D3DReflect(
        shader->GetBufferPointer(),
        shader->GetBufferSize(),
        IID_PPV_ARGS(&reflection));

    if (FAILED(hr))
        return false;

    bool success = true;

    D3D11_SHADER_DESC shaderDesc{};
    if (FAILED(reflection->GetDesc(&shaderDesc)))
        throw std::runtime_error("Shader reflection GetDesc failed");

    for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC binding{};
        if (FAILED(reflection->GetResourceBindingDesc(i, &binding)))
            continue;
        if (binding.Type != D3D_SIT_TEXTURE)
            continue;
        if (binding.BindCount > 1)
        {
            MainLog->Log("Shader '{}' has texture array '{}', which is not supported. BindCount: {}", name(), binding.Name, binding.BindCount);
            success = false;
            continue;
        }
        std::string_view name = binding.Name;
        if (name.ends_with("Texture"))
            name.remove_suffix("Texture"sv.size());
        mTextureSlotNames[std::string(name)] = binding.BindPoint;
    }

    reflection->Release();

    return success;
}
 
void cPixelShader::processMetaData(cShaderSource& shaderSource)
{
    mTextureSlotNames.clear();
    mConfig.reset();

    std::vector<cShaderConstant> constants;
    mParameterNames.fill({});
    auto metaData = shaderSource.metaData();
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
            mTextureSlotNames[slotValue.toString()] = idx;
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
    std::array<int, 4> dataSizes = { 0, 0, 0, 0 };
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
                    dataSizes[slot] = static_cast<int>(floats.size());
                }
            }
        }
    }
    for (auto&& [slot, dataSize] : dataSizes | std::views::enumerate)
    {
        constants.emplace_back(std::format("DATA{}_SIZE", slot), static_cast<int>(dataSize));
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
                else if (type == "color" || type == "normalized_color")
                {
                    std::array<float, 3> colorValue;
                    auto colorTable = constantValue.get("value");
                    for (int i = 0; i < 3; ++i)
                    {
                        colorValue[i] = colorTable.get<double>(i + 1);
                    }
                    constants.emplace_back(constantName.toString(), colorValue);
                }
                else if (type == "int")
                {
                    int intValue = constantValue.get<int>("value");
                    constants.emplace_back(constantName.toString(), intValue);
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
        else if (std::holds_alternative<int>(constant.mValue))
        {
            constantDeclarations += std::format("static const int {} = {}; ", constant.mName, std::get<int>(constant.mValue));
        }
    }
    shaderSource.insertSourceCode(constantDeclarations);
    if (!mConfig)
    {
        mConfig = std::make_shared<cConfig>();
    }
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
    auto it = mTextureSlotNames.find(std::string(name));
    if (it == mTextureSlotNames.end())
    {
        throw std::runtime_error(std::format("Texture slot name '{}' not found in pixel shader", name));
    }
    return it->second;
}