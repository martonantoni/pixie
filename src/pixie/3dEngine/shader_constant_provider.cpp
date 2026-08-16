#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

std::vector<std::shared_ptr<cShaderConstantProvider>> theShaderConstantProviders;

cShaderConstantProvider::cShaderConstantProvider(int slot, int size)
    : mSlot(slot), mSize(size)
{
    ASSERT(mSize % 16 == 0); // Constant buffer size must be a multiple of 16 bytes

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(size);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3V(cDevice::Get()->GetD3DObject()->CreateBuffer(&bufferDesc, nullptr, &mShaderConstantBuffer));
}

cShaderConstantProvider::~cShaderConstantProvider()
{
    if (mShaderConstantBuffer)
    {
        mShaderConstantBuffer->Release();
    }
}
