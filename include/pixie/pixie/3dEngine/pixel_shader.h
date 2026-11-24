#pragma once


class cPixelShader final: public cShader
{
    ID3D11PixelShader* mShader = nullptr;
    std::array<std::string, 4> mParameterNames;
    std::array<std::string, 4> mDefaultTextures;
    std::array<tIntrusivePtr<cTexture>, 4> mDataTextures;
    std::shared_ptr<cConfig> mConfig;
    std::unordered_map<std::string, int> mTextureSlotNames; // name -> slot index

    std::unordered_map<std::string, tIntrusivePtr<cTexture>> mBoundTextures; 

    void processMetaData(cShaderSource& shaderSource);
    bool reflectTextures(ID3DBlob* shader);
public:
    cPixelShader(const std::string& name) : cShader(name) {}
    virtual ~cPixelShader();
    bool compile(cShaderSource& shaderSource);

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