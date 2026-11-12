#pragma once

struct cShaderConstant
{
    std::string mName;
    std::variant<float, std::array<float, 3>> mValue;
};

class cShaderManager final: public tSingleton<cShaderManager>
{
    std::filesystem::path mShaderFolder;
    std::filesystem::path mShaderPresetPath;
    std::unordered_map<std::string, std::shared_ptr<cPixelShader>> mPixelShaders;
    std::unordered_map<std::string, std::shared_ptr<cVertexShader>> mVertexShaders;
    using cShaderSources = std::unordered_map<std::string, std::string>; // shader name -> source code
    cShaderSources mShaderSources;  // for include
    std::vector<std::unique_ptr<Pixie::cFileChangeNotifier>> mFileChangeNotifiers;
    std::unique_ptr<Pixie::cFileChangeNotifier> mPresetChangeNotifier;
    std::string mShadersPreset; 
    void loadIncludesFromFolder(const std::filesystem::path& folderPath, const std::string& shaderNamePrefix);
    void loadIncludes();
    void loadShaderConstants();
    void loadShadersFromFolder(const std::filesystem::path& folderPath, const std::string& shaderNamePrefix);
    bool compilePixelShader(std::shared_ptr<cPixelShader> pixelShader, std::string finalSourceCode, cLuaObject metaData);
    static std::pair<cLuaObject, std::string> extractMetaInfo(std::string_view sourceCode);
public:
    cShaderManager();
    ~cShaderManager();

    void init(); // loads and compiles all shaders from the "shaders" folder
    void reloadShaders(); 
    std::shared_ptr<cPixelShader> pixelShader(const std::string& name) const;
    std::shared_ptr<cVertexShader> vertexShader(const std::string& name) const;
    std::string_view shaderSource(const std::string& name) const; // returns the source code of the shader, or empty string if not found
};

extern cShaderManager* theShaderManager;