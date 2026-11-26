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

bool cPixelShader::compile(cShaderSource shaderSource, cMetaData metaData)
{
    processMetaData(shaderSource, metaData);
    if (auto blob = cShader::compile(shaderSource.sourceCode(), metaData.mEntryPoint.c_str(), "ps_5_0"))
    {
        if (!reflectTextures(blob, metaData))
        {
            blob->Release();
            return false;
        }

        auto device = cDevice::Get();
        if(mShader)
            mShader->Release();
        mShader = nullptr;
        mMetaData = std::move(metaData);

        D3V(device->GetD3DObject()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mShader));
        {
            cLog debugLog(std::format("debug/{}_ps.txt", name()), cLog::Flags::TRUNCATE);
            dumpDebugInfo(debugLog, blob, true);
        }
        MainLog->Log("Compiled pixel shader: {}", name());
        blob->Release();
        return true;
    }
    else
    {
        MainLog->Log("Failed to compile pixel shader: {}", name());
#ifdef _DEBUG
        cLog debugLog(std::format("debug/src_{}_ps.txt", name()), cLog::Flags::TRUNCATE);
        debugLog.Log("{}", shaderSource.sourceCode());
#endif
        return false;
    }
}


bool cPixelShader::reflectTextures(ID3DBlob* shader, cMetaData& metaData)
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
            MainLog->Log("Shader '{}' has texture array '{}', which is not supported. BindCount: {}", metaData.mShaderName, binding.Name, binding.BindCount);
            success = false;
            continue;
        }
        std::string_view name = binding.Name;
        if (name.ends_with("Texture"))
            name.remove_suffix("Texture"sv.size());
        metaData.mTextureSlotNames[std::string(name)] = binding.BindPoint;
    }

    reflection->Release();

    return success;
}

void cPixelShader::processMetaData(cShaderSource& shaderSource, const cMetaData& metaData)
{
    std::string constantDeclarations;
    for (auto&& [name, constant] : metaData.mConstants)
    {
        if (std::holds_alternative<float>(constant))
        {
            constantDeclarations += std::format("static const float {} = {}; ",
                name, std::get<float>(constant));
        }
        else if (std::holds_alternative<std::array<float, 3>>(constant))
        {
            auto color = std::get<std::array<float, 3>>(constant);
            constantDeclarations += std::format("static const float3 {} = float3({}, {}, {}); ",
                name, color[0], color[1], color[2]);
        }
        else if (std::holds_alternative<int>(constant))
        {
            constantDeclarations += std::format("static const int {} = {}; ",
                name, std::get<int>(constant));
        }
    }
    shaderSource.insertSourceCode(constantDeclarations);
    for (auto&& [idx, paramName] : metaData.mParameterNames | std::views::enumerate)
    {
        if (paramName.empty())
            continue;
        shaderSource.regexReplace(
            "\\binput\\s*\\.\\s*" + paramName + "\\b",
            "input.params."s + "xyzw"[idx]);
    }
}

void cPixelShader::cMetaData::apply(cLuaObject shaderData)
{
// NOTE:
// configuration not supported as a "group override" currently, it should be merging the configs in that case
    if (shaderData.has("entry_point"))
    {
        mEntryPoint = shaderData.get<std::string>("entry_point");
    }
    if (shaderData.has("config"))
    {
        mConfig = Pixie::toConfig(shaderData.get("config"));
    }
    if (shaderData.has("configuration"))
    {
        mConfig = Pixie::toConfig(shaderData.get("configuration"));
    }
    if (shaderData.has("texture_slots"))
    {
        auto textureSlots = shaderData.get("texture_slots");
        for (auto&& [idx, slotValue] : textureSlots |
            std::views::values | std::views::take(4) | std::views::enumerate)
        {
            mTextureSlotNames[slotValue.toString()] = idx;
        }
    }
    if (shaderData.has("textures"))
    {
        auto textures = shaderData.get("textures");
        for (auto&& [idx, textureValue] : textures |
            std::views::values | std::views::take(mDefaultTextures.size()) | std::views::enumerate)
        {
            mDefaultTextures[idx] = textureValue.toString();
        }
    }
    if (shaderData.has("parameters"))
    {
        auto parameters = shaderData.get("parameters");
        for (auto&& [idx, paramValue] : parameters |
            std::views::values | std::views::take(mParameterNames.size()) | std::views::enumerate)
        {
            mParameterNames[idx] = paramValue.toString();
        }
    }

    if (shaderData.has("constants"))
    {
        auto constantsTable = shaderData.get("constants");
        for (auto&& [constantName, constantValue] : constantsTable)
        {
            if (constantValue.isTable())
            {
                auto type = constantValue.get<std::string>("type");
                if (type == "float")
                {
                    float floatValue = constantValue.get<double>("default");
                    mConstants[constantName.toString()] = floatValue;
                }
                else if (type == "color" || type == "normalized_color")
                {
                    std::array<float, 3> colorValue;
                    auto colorTable = constantValue.get("default");
                    for (int i = 0; i < 3; ++i)
                    {
                        colorValue[i] = colorTable.get<double>(i + 1);
                    }
                    mConstants[constantName.toString()] = colorValue;
                }
                else if (type == "int")
                {
                    int intValue = constantValue.get<int>("default");
                    mConstants[constantName.toString()] = intValue;
                }
            }
            else
            {
                MainLog->Log("Constant '{}' in shader metadata is not a table, skipping.", constantName.toString());
            }
        }
    }

    if (shaderData.has("data"))
    {
        auto data = shaderData.get("data");
        for (auto&& [key, value] : data)
        {
            if (value.isTable())
            {
                tIntrusivePtr<cTexture> dataTexture;
                int dataSize = 0;
                if (value.has("floats"))
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
                    dataTexture = cTexture::CreateFromData(cPoint(floats.size(), 1), floats);
                    dataSize = static_cast<int>(floats.size());
                }
                if (dataTexture)
                {
                    if (value.has("slot"))
                    {
                        int slot = value.get<int>("slot");
                        mConstants[std::format("DATA{}_SIZE", slot)] = dataSize;
                        mDataTextures[slot] = std::move(dataTexture);
                    }
                    else if (value.has("name"))
                    {
                        std::string name = value.get<std::string>("name");
                        mConstants[std::format("{}Size", name)] = dataSize;
                        mDataTextures[name] = dataTexture;
                    }
                    else
                    {
                        MainLog->Log("Data entry '{}'  has no 'slot' or 'name' field, skipping.",
                            key.toString());
                    }
                }
            }
        }
    }

}

void cPixelShader::cMetaData::setConstantsFromPreset(cLuaObject preset)
{
    for (auto&& [name, value] : mConstants)
    {
        if (preset.has(name))
        {
            auto presetValue = preset.get(name);
            // has to make sure it is the same type:
            if (std::holds_alternative<float>(value) && presetValue.isNumber())
            {
                mConstants[name] = static_cast<float>(presetValue.toDouble());
            }
            else if (std::holds_alternative<std::array<float, 3>>(value) && presetValue.isTable() && presetValue.arraySize() == 3)
            {
                std::array<float, 3> colorValue;
                auto colorTable = presetValue;
                for (int i = 0; i < 3; ++i)
                {
                    colorValue[i] = colorTable.get<double>(i + 1);
                }
                mConstants[name] = colorValue;
            }
            else if (std::holds_alternative<int>(value) && presetValue.isNumber())
            {
                mConstants[name] = static_cast<int>(presetValue.toInt());
            }
            else
            {
                MainLog->Log("Preset value for constant '{}' has incompatible type, skipping.", name);
            }
        }
    }
}

 


int cPixelShader::parameterIndex(std::string_view name) const
{
    return mMetaData.parameterIndex(name);
}
int cPixelShader::textureSlotIndex(std::string_view name) const
{
    return mMetaData.textureSlotIndex(name);
}
tIntrusivePtr<cTexture> cPixelShader::dataTexture(cVariableKey key) const 
{
    auto it = mMetaData.mDataTextures.find(key);
    if (it == mMetaData.mDataTextures.end())
    {
        return nullptr;
    }
    return it->second;
}


int cPixelShader::cMetaData::parameterIndex(std::string_view name) const
{
    auto it = std::find(mParameterNames.begin(), mParameterNames.end(), name);
    if (it == mParameterNames.end())
    {
        throw std::runtime_error(std::format("Parameter name '{}' not found in pixel shader", name));
    }
    return static_cast<int>(std::distance(mParameterNames.begin(), it));
}

int cPixelShader::cMetaData::textureSlotIndex(std::string_view name) const
{
    auto it = mTextureSlotNames.find(std::string(name));
    if (it == mTextureSlotNames.end())
    {
        throw std::runtime_error(std::format("Texture slot name '{}' not found in pixel shader", name));
    }
    return it->second;
}

void cPixelShader::cMetaData::synchronize(std::span<cMetaData> shaderMetaDataList)
{
    // all shaders need to know about all constants, for these missing constants we'll use a fixed default value
    // also, if the same constant has different types we need to throw an error
    std::unordered_map<std::string, cShaderConstantValue> allConstants;
    for (auto&& metaData : shaderMetaDataList)
    {
        for (auto&& [name, value] : metaData.mConstants)
        {
            auto it = allConstants.find(name);
            if (it != allConstants.end())
            {
                if (it->second.index() != value.index())
                {
                    throw std::runtime_error(std::format("Constant '{}' has different types in different shaders", name));
                }
            }
            else
            {
                if (std::holds_alternative<float>(value))
                {
                    allConstants[name] = 0.0f;
                }
                else if (std::holds_alternative<std::array<float, 3>>(value))
                {
                    allConstants[name] = std::array<float, 3>{1.0f, 0.0f, 1.0f};
                }
                else if (std::holds_alternative<int>(value))
                {
                    allConstants[name] = 0;
                }
                else
                {
                    throw std::runtime_error(std::format("Constant '{}' has unsupported type", name));
                }
            }
        }
    }
    // now add missing constants to each shader's metadata:
    for (auto&& metaData : shaderMetaDataList)
    {
        for (auto&& [name, value] : allConstants)
        {
            if (metaData.mConstants.find(name) == metaData.mConstants.end())
            {
                metaData.mConstants[name] = value;
            }
        }
    }
}
