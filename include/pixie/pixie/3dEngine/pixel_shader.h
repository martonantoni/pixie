#pragma once


class cPixelShader final: public cShader
{
    ID3D11PixelShader* mShader = nullptr;
    std::array<std::string, 4> mParameterNames;
    std::array<std::string, 4> mTextureSlotNames;
    std::array<std::string, 4> mDefaultTextures;
    std::array<tIntrusivePtr<cTexture>, 4> mDataTextures;
    std::shared_ptr<cConfig> mConfig;
    std::string processMetaData(std::string sourceCode, cLuaObject metaData);
public:
    cPixelShader(const std::string& name) : cShader(name) {}
    virtual ~cPixelShader();
    static inline constexpr std::string_view mMetaInsertionPointText = "// @meta_insertion_point"sv;
    bool compile(std::string sourceCode, cLuaObject metaData);

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
    const cConfig& config() const { return *mConfig; }
};