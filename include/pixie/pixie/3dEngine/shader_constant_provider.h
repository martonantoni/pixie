#pragma once

class cShaderConstantProvider
{
protected:
    ID3D11Buffer* mShaderConstantBuffer = nullptr;
    int mSlot = -1;
    int mSize = 0;
public:
    cShaderConstantProvider(int slot, int size);

    virtual ~cShaderConstantProvider();

    ID3D11Buffer* shaderConstantBuffer() const
    {
        return mShaderConstantBuffer;
    }
    int slot() const
    {
        return mSlot;
    }
    virtual void load() {}
};

extern std::vector<std::shared_ptr<cShaderConstantProvider>> theShaderConstantProviders;