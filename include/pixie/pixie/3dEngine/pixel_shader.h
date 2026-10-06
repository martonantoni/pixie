#pragma once


class cPixelShader final: public cShader
{
    ID3D11PixelShader* mShader = nullptr;
    std::array<std::string, 4> mParameterNames;
    std::array<std::string, 4> mTextureSlotNames;
    std::array<std::string, 4> mDefaultTextures;
    std::array<tIntrusivePtr<cTexture>, 4> mDataTextures;
    std::shared_ptr<cConfig> mConstants;
    std::string extractMetaInfo(std::string_view sourceCode);
public:
    cPixelShader(const std::string& name, std::string_view sourceCode) : cShader(name) { compile(sourceCode); }
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
    int textureSlotIndex(std::string_view name) const; // throws if not found
    const std::string& defaultTextureName(int index) const { return mDefaultTextures[index]; }
    tIntrusivePtr<cTexture> dataTexture(int index) const { return mDataTextures[index]; }
    const cConfig& constants() const { return *mConstants; }
};