#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cPixelShader::~cPixelShader()
{
    if (mShader)
        mShader->Release();
}

void cPixelShader::compile(std::string_view sourceCode)
{
    if (auto blob =cShader::compile(sourceCode, "PSMain", "ps_5_0"))
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
        extractMetaInfo(sourceCode);
    }
}

void cPixelShader::extractMetaInfo(std::string_view sourceCode)
{
    // meta data is in the form of comments like this:
    // @<id> <name> [data]
    // currently we only support parameter names / indexes:
    // @param <name> <index>

    mParameterNames.fill({});

    std::string configSource;
    bool inConfig = false;

    for (auto&& lineRange : sourceCode | std::views::split('\n'))
    {
        std::string line(lineRange.begin(), lineRange.end());

        auto commentPos = line.find("//");
        if (commentPos == std::string::npos)
            continue;

        std::string_view comment(line.data() + commentPos + 2, line.size() - commentPos - 2);

        if (comment.find("@cfg") != std::string_view::npos)
        {
            inConfig = true;
            continue;
        }

        if (comment.find("@endcfg") != std::string_view::npos)
            break;

        if (inConfig)
        {
            configSource.append(comment);
            configSource += '\n';
        }
    }

    auto config = cLuaState::stringToConfig(configSource);
    mConstants = config->getSubOrEmptyConfig("constants");
    config->forEachSubConfig([&](const std::string& key, const cConfig& subConfig)
        {
            if (key == "texture_slots")
            {
                subConfig.forEachString([&](int idx, const std::string& slotName)
                    {
                        if (idx >= 0 && idx < mTextureSlotNames.size())
                        {
                            mTextureSlotNames[idx] = slotName;
                        }
                    });
            }
            else if(key == "parameters")
            {
                subConfig.forEachString([&](int idx, const std::string& paramName)
                    {
                        if (idx >= 0 && idx < mParameterNames.size())
                        {
                            mParameterNames[idx] = paramName;
                        }
                    });
            }
            else if (key == "textures")
            {
                subConfig.forEachString([&](int idx, const std::string& textureName)
                    {
                        if (idx >= 0 && idx < mDefaultTextures.size())
                        {
                            mDefaultTextures[idx] = textureName;
                        }
                    });
            }
        });
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