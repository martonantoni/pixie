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
        MainLog->Log("Pixel shader compilation error: %s", errorText.c_str());
        shader = nullptr;
    }
    if (errors)
        errors->Release();

    return shader;
}
