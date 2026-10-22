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
    std::unique_ptr<Pixie::cFileChangeNotifier> mFileChangeNotifier, mPresetChangeNotifier;
    std::unordered_map<std::string, std::vector<cShaderConstant>> mShaderConstants; // group name -> constants
    void loadIncludes();
    void loadShaderConstants();
public:
    cShaderManager();
    ~cShaderManager();

    void init(); // loads and compiles all shaders from the "shaders" folder
    void reloadShaders(); 
    std::shared_ptr<cPixelShader> pixelShader(const std::string& name) const;
    std::shared_ptr<cVertexShader> vertexShader(const std::string& name) const;
    std::string_view shaderSource(const std::string& name) const; // returns the source code of the shader, or empty string if not found
    const std::vector<cShaderConstant>& shaderConstants(const std::string& groupName) const;
};

extern cShaderManager* theShaderManager;