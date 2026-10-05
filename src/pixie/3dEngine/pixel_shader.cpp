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

    auto state = std::make_shared<cLuaState>();
    state->executeString(configSource);
    auto rootTable = state->globalTable();
    for (auto&& [key, value] : rootTable)
    {
        if (key.toString() == "constants")
        {
            mConstants = Pixie::toConfig(value);
        }
        else if (key.toString() == "texture_slots")
        {
            for (auto&& [idx, slotValue] : value | std::views::values | std::views::enumerate)
            {
                if (idx >= 0 && idx < mTextureSlotNames.size())
                {
                    mTextureSlotNames[idx] = slotValue.toString();
                }
            }
        }
        else if (key.toString() == "parameters")
        {
            for (auto&& [idx, paramValue] : value | std::views::values | std::views::enumerate)
            {
                if (idx >= 0 && idx < mParameterNames.size())
                {
                    mParameterNames[idx] = paramValue.toString();
                }
            }
        }
        else if (key.toString() == "textures")
        {
            for (auto&& [idx, textureValue] : value | std::views::values | std::views::enumerate)
            {
                if (idx >= 0 && idx < mDefaultTextures.size())
                {
                    mDefaultTextures[idx] = textureValue.toString();
                }
            }
        }
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
    auto it = std::find(mTextureSlotNames.begin(), mTextureSlotNames.end(), name);
    if (it == mTextureSlotNames.end())
    {
        throw std::runtime_error(std::format("Texture slot name '{}' not found in pixel shader", name));
    }
    return static_cast<int>(std::distance(mTextureSlotNames.begin(), it));
}