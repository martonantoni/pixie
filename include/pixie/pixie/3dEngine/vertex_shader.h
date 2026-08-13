#pragma once


class cVertexShader final : public cShader
{
    ID3D11VertexShader* mShader = nullptr;
    ID3D11InputLayout* mInputLayout = nullptr;
public:
    cVertexShader(std::string_view sourceCode) { compile(sourceCode); }
    virtual ~cVertexShader();
    void compile(std::string_view sourceCode);

    ID3D11VertexShader* shader() const
    {
        return mShader;
    }
    ID3D11InputLayout* inputLayout() const
    {
        return mInputLayout;
    }
    operator bool() const
    {
        return mShader != nullptr && mInputLayout != nullptr;
    }
};