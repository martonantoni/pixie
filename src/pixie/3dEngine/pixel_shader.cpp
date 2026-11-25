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

void cPixelShader::cMetaData::apply(cLuaObject shaderData)
{
// NOTE:
// configuration not supported as a "group override" currently, it should be merging the configs in that case
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
        for (auto&& [idx, slotValue] : textureSlots |
            std::views::values | std::views::take(mTextureSlotNames.size()) | std::views::enumerate)
        {
            mTextureSlotNames[slotValue.toString()] = idx;
        }
    }
    if (metaData.has("parameters"))
    {
        auto parameters = metaData.get("parameters");
        for (auto&& [idx, paramValue] : parameters |
            std::views::values | std::views::take(mParameterNames.size()) | std::views::enumerate)
        {
            mParameterNames[idx] = paramValue.toString();
        }
    }
    if (metaData.has("textures"))
    {
        auto textures = metaData.get("textures");
        for (auto&& [idx, textureValue] : textures | 
            std::views::values | std::views::take(mDefaultTextures.size()) | std::views::enumerate)
        {
            mDefaultTextures[idx] = textureValue.toString();
        }
    }
    std::vector<std::pair<std::string, int>> dataSizes;
    if (metaData.has("data"))
    {
        auto data = metaData.get("data");
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
                if(dataTexture)
                {
                    if (value.has("slot"))
                    {
                        int slot = value.get<int>("slot");
                        dataSizes.emplace_back(std::format("DATA{}_SIZE", slot), dataSize);
                        mDataTextures[slot] = dataTexture;
                    }
                    else if (value.has("name"))
                    {
                        std::string name = value.get<std::string>("name");
                        dataSizes.emplace_back(std::format("{}Size", name), dataSize);
                        mBoundTextures[name] = dataTexture;
                    }
                    else
                    {
                        MainLog->Log("Data entry '{}' in shader '{}' has no 'slot' or 'name' field, skipping.", 
                            key.toString(), name());
                    }
                }
            }
        }
    }
    for (auto&& [slot, dataSize] : dataSizes)
    {
        constants.emplace_back(slot, dataSize);
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
            constantDeclarations += std::format("static const float {} = {}; ", 
                constant.mName, std::get<float>(constant.mValue));
        }
        else if (std::holds_alternative<std::array<float, 3>>(constant.mValue))
        {
            auto color = std::get<std::array<float, 3>>(constant.mValue);
            constantDeclarations += std::format("static const float3 {} = float3({}, {}, {}); ", 
                constant.mName, color[0], color[1], color[2]);
        }
        else if (std::holds_alternative<int>(constant.mValue))
        {
            constantDeclarations += std::format("static const int {} = {}; ", 
                constant.mName, std::get<int>(constant.mValue));
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