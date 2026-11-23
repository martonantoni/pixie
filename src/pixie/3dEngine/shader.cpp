#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

class cShaderInclude : public ID3DInclude
{
public:
    cShaderInclude() = default;

    HRESULT Open(D3D_INCLUDE_TYPE, LPCSTR fileName, LPCVOID, LPCVOID* data, UINT* bytes) override
    {
        auto source = theShaderManager->shaderSource(fileName);
        if (source.empty())
            return E_FAIL;

        *data = source.data();
        *bytes = static_cast<UINT>(source.size());

        return S_OK;
    }

    HRESULT Close(LPCVOID data) override
    {
         return S_OK;
    }
};

cShader::~cShader()
{
}

ID3DBlob* cShader::compile(std::string_view sourceCode, const std::string& entryPoint, const std::string& target)
{
    cShaderInclude shaderInclude;

    ID3DBlob* shader = nullptr;
    ID3DBlob* errors = nullptr;
    HRESULT result = D3DCompile(
        sourceCode.data(),
        sourceCode.size(),
        "Pixie pixel shader",
        nullptr,
        &shaderInclude,
        entryPoint.c_str(),
        target.c_str(),
        0,
        0,
        &shader,
        &errors);
    if (FAILED(result))
    {
        std::string errorText = errors
            ? std::string(static_cast<const char*>(errors->GetBufferPointer()), errors->GetBufferSize())
            : "Unknown shader compilation error";
        MainLog->Log("Shader '{}' compilation error: {}", name(), errorText);
        shader = nullptr;
    }
    if (errors)
        errors->Release();

    return shader;
}

void cShader::dumpDebugInfo(cLog& log, ID3DBlob* blob, bool input)
{
    {
        ID3D11ShaderReflection* reflection = nullptr;

        D3DReflect(
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            __uuidof(ID3D11ShaderReflection),
            reinterpret_cast<void**>(&reflection));

        D3D11_SHADER_DESC shaderDesc;
        reflection->GetDesc(&shaderDesc);

        const UINT count = input
            ? shaderDesc.InputParameters
            : shaderDesc.OutputParameters;

        for (UINT i = 0; i < count; ++i)
        {
            D3D11_SIGNATURE_PARAMETER_DESC desc;

            if (input)
                reflection->GetInputParameterDesc(i, &desc);
            else
                reflection->GetOutputParameterDesc(i, &desc);

            log.Log(
                "{}{} register={} mask={:x} type={}",
                desc.SemanticName,
                desc.SemanticIndex,
                desc.Register,
                desc.Mask,
                static_cast<int>(desc.ComponentType));
        }

        reflection->Release();
    }


    //if (shader)
    {
        ID3DBlob* disassembly = nullptr;

        if (SUCCEEDED(D3DDisassemble(
        blob->GetBufferPointer(),
            blob->GetBufferSize(),
                    0,
            nullptr,
            &disassembly)))
        {
            std::string text(
                static_cast<const char*>(disassembly->GetBufferPointer()),
                disassembly->GetBufferSize());

            log.Log("\n\n------------------ Shader disassembly ------------------\n\n\n{}", text.c_str());

            disassembly->Release();
        }
    }

}

cShaderSource::cShaderSource(const std::string& sourceCode)
{
    std::string finalSourceCode;

    // meta data is in the form of comments like this:
    // @cfg
    // lua code (does not need leading //)
    // @endcfg


    std::string configSource;
    bool inConfig = false;
    int cfgPos = 0;
    bool metaInsertionPointAdded = false;

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
                // if this is inside lua code, only treat it as a comment if there is only whitespace before it:
                if (std::all_of(line.begin(), line.begin() + commentPos, [](char c) { return std::isspace(static_cast<unsigned char>(c)); }))
                {
                    line = std::string(comment);
                }
            }
        }
        if (inConfig)
        {
            configSource.append(line);
            if (!metaInsertionPointAdded)
            {
                finalSourceCode.append(mMetaInsertionPointText);
                metaInsertionPointAdded = true;
            }
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

    mMetaData = state->globalTable();
    mSourceCode = finalSourceCode;
}

cShaderSource::cShaderSource(const cShaderSource& src, cLuaObject subMeta)
    : mSourceCode(src.mSourceCode), mMetaData(subMeta)
{
}

void cShaderSource::insertSourceCode(const std::string& additionalCode)
{
    auto pos = mSourceCode.find(mMetaInsertionPointText);
    if (pos != std::string::npos)
    {
        mSourceCode.insert(pos, additionalCode);
    }
}

void cShaderSource::regexReplace(std::string_view pattern, std::string_view replacement)
{
    mSourceCode = std::regex_replace(mSourceCode, std::regex(pattern.data()), replacement.data());
}
