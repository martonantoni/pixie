#pragma once


class cPixelShader final: public cShader
{
    ID3D11PixelShader* mShader = nullptr;
    std::array<std::string, 4> mParameterNames;
    void extractMetaInfo(std::string_view sourceCode);
public:
    cPixelShader(std::string_view sourceCode) { compile(sourceCode); }
    virtual ~cPixelShader();
    void compile(std::string_view sourceCode);

    ID3D11PixelShader* shader() const
    {
        return mShader;
    }
    operator bool() const
    {
        return mShader != nullptr;
    }

    int parameterIndex(std::string_view name) const; // throws if not found
};