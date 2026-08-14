#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


cShader::~cShader()
{
}

ID3DBlob* cShader::compile(std::string_view sourceCode, const std::string& entryPoint, const std::string& target)
{
    ID3DBlob* shader = nullptr;
    ID3DBlob* errors = nullptr;
    HRESULT result = D3DCompile(
        sourceCode.data(),
        sourceCode.size(),
        "Pixie pixel shader",
        nullptr,
        nullptr,
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
