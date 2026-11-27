#pragma once

class cPixelShader final: public cShader
{
public:
    using cShaderConstantValue = std::variant<float, std::array<float, 3>, int>;
    struct cShaderConstant
    {
        std::string mName;
        cShaderConstantValue mValue;
    };
    using cVariableKey = std::variant<int, std::string>; // int for slot index, string for name
    struct cMetaData
    {
        std::string mShaderName;
        std::string mNameSpace;
        std::array<std::string, 4> mParameterNames;
        std::unordered_map<std::string, int> mTextureSlotNames; // name -> slot index
        std::shared_ptr<cConfig> mConfig;
        std::unordered_map<std::string, cShaderConstantValue> mConstants;
        std::unordered_map<cVariableKey, tIntrusivePtr<cTexture>> mDataTextures;
        std::unordered_map<cVariableKey, tIntrusivePtr<cTexture>> mFixedTextures;
        std::string mEntryPoint = "PSMain"s;
        cTextureBindings mTextureBindings;

        void setShaderName(std::string_view name) { mShaderName = name; }
        void apply(cLuaObject shaderData);
        void setConstantsFromPreset(cLuaObject preset);
        static void synchronize(std::span<cMetaData> shaderMetaDataList);

        int parameterIndex(std::string_view name) const; // throws if not found
        int textureSlotIndex(std::string_view name) const; // throws if not found

        void mutateSourceCode(cShaderSource& shaderSource) const;
    };
private:
    ID3D11PixelShader* mShader = nullptr;
    cMetaData mMetaData;
    
    static void processMetaData(cShaderSource& shaderSource, const cMetaData& metaData);
    static bool reflectTextures(ID3DBlob* shader, cMetaData& metaData);
public:
    cPixelShader(const std::string& name) : cShader(name) {}
    virtual ~cPixelShader();
    bool compile(cShaderSource shaderSource, cMetaData metaData);

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
    tIntrusivePtr<cTexture> dataTexture(cVariableKey key) const;
    const cConfig& config() const { return *mMetaData.mConfig; }
    cTextureBindings defaultTextureBindings() const;
};