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
        D3V(device->GetD3DObject()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mShader));
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

    for (auto&& lineRange : sourceCode | std::views::split('\n'))
    {
        std::string line(lineRange.begin(), lineRange.end());

        auto commentPos = line.find("//");
        if (commentPos == std::string::npos)
            continue;

        std::istringstream input(line.substr(commentPos + 2));

        std::string id;
        input >> id;

        if (id == "@param")
        {
            std::string name;
            int index;

            if (input >> name >> index &&
                index >= 0 &&
                index < static_cast<int>(mParameterNames.size()))
            {
                mParameterNames[index] = std::move(name);
            }
        }
    }
}


int cPixelShader::parameterIndex(std::string_view name) const
{
    auto it = std::find(mParameterNames.begin(), mParameterNames.end(), name);
    if (it == mParameterNames.end())
    {
        throw std::runtime_error(fmt::sprintf("Parameter name '%s' not found in pixel shader", name));
    }
    return static_cast<int>(std::distance(mParameterNames.begin(), it));
}