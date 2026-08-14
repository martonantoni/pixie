#pragma once


class cShaderManager final: public tSingleton<cShaderManager>
{
    std::unordered_map<std::string, std::shared_ptr<cPixelShader>> mPixelShaders;
    std::unordered_map<std::string, std::shared_ptr<cVertexShader>> mVertexShaders;
public:
    cShaderManager();
    ~cShaderManager();

    void init(); // loads and compiles all shaders from the "shaders" folder
    void reloadShaders(); 
    std::shared_ptr<cPixelShader> pixelShader(const std::string& name) const;
    std::shared_ptr<cVertexShader> vertexShader(const std::string& name) const;
};

extern cShaderManager* theShaderManager;