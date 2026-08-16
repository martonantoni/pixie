#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

void cVertexShader::compile(std::string_view sourceCode)
{
    using cSpriteVertexData = cSpriteRenderer::cSpriteVertexData;
    if(auto blob = cShader::compile(sourceCode, "VSMain", "vs_5_0"))
    {
        if(mShader)
            mShader->Release();
        if (mInputLayout)
            mInputLayout->Release();
        auto device = cDevice::Get();
        D3V(device->GetD3DObject()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mShader));
        D3D11_INPUT_ELEMENT_DESC inputLayout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(cSpriteVertexData, x), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32_UINT, 0, offsetof(cSpriteVertexData, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(cSpriteVertexData, u), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "PARAM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(cSpriteVertexData, mShaderParameters), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(cSpriteVertexData, edgeDistances), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        D3V(device->GetD3DObject()->CreateInputLayout(
            inputLayout,
            static_cast<UINT>(sizeof(inputLayout) / sizeof(inputLayout[0])),
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            &mInputLayout));
    }
}

cVertexShader::~cVertexShader()
{
    if (mShader)
        mShader->Release();
    if (mInputLayout)
        mInputLayout->Release();
}
