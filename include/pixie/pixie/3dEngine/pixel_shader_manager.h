#pragma once


class cShaderManager final: public tSingleton<cShaderManager>
{
    std::unordered_map<std::string, std::shared_ptr<cPixelShader>> mPixelShaders;
    std::unordered_map<std::string, std::shared_ptr<cVertexShader>> mVertexShaders;
    using cShaderSources = std::unordered_map<std::string, std::string>; // shader name -> source code
    cShaderSources mShaderSources;  // for include
    std::filesystem::path mShaderFolder;
#ifdef _DEBUG
    cRegisteredID mReloadTimerID;
    std::filesystem::file_time_type mLastShaderWriteTime;
    bool checkIfReloadNeeded();
#endif
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