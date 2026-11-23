#pragma once

struct cShaderConstant
{
    std::string mName;
    std::variant<float, std::array<float, 3>, int> mValue;
};

struct cShaderFileScript
{
    cLuaObject mMetaData;
};

class cShaderManager final: public tSingleton<cShaderManager>
{
    std::filesystem::path mShaderFolder;
    std::filesystem::path mShaderPresetPath;
    std::unordered_map<std::string, std::shared_ptr<cPixelShader>> mPixelShaders;
    struct cShaderFile
    {
        std::filesystem::path mPath;
        std::string mNamePrefix;
        std::string mShaderFileID; // in a dot separated format, e.g. "folder.subfolder.shadername"
        std::shared_ptr<cShaderFileScript> mScript;
    };
    std::vector<cShaderFile> mShaderFiles; // path, shader name prefix
    std::unordered_map<std::string, std::shared_ptr<cVertexShader>> mVertexShaders;
    using cIncludeFiles = std::unordered_map<std::string, cShaderSource>; // shader name -> source code
    cIncludeFiles mIncludeFiles;  // for include
    std::vector<std::unique_ptr<Pixie::cFileChangeNotifier>> mFileChangeNotifiers;
    std::unique_ptr<Pixie::cFileChangeNotifier> mPresetChangeNotifier;
    std::string mShadersPreset; 
    void loadIncludesFromFolder(const std::filesystem::path& folderPath, const std::string& shaderNamePrefix);
    void loadIncludes();
    void loadShaderConstants();
    void loadShadersFromFolder(const std::filesystem::path& folderPath, const std::string& shaderNamePrefix);
    bool compilePixelShader(std::shared_ptr<cPixelShader> pixelShader, cShaderSource& shaderSource);
    std::pair<bool, cLuaObject> processShaderFile(const std::filesystem::path& path, const std::string& shaderTypeString, const std::string& shaderNamePrefix);
public:
    cShaderManager();
    ~cShaderManager();

    void init(); // loads and compiles all shaders from the "shaders" folder
    void reloadShaders(); 
    std::shared_ptr<cPixelShader> pixelShader(const std::string& name) const;
    std::shared_ptr<cVertexShader> vertexShader(const std::string& name) const;
    cShaderSource& includeFile(const std::string& name); 
    std::shared_ptr<cShaderFileScript> shaderScript(const std::string& shaderFileID) const; 
};

extern cShaderManager* theShaderManager;