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
        MainLog->Log("Pixel shader compilation error: {}", errorText);
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
