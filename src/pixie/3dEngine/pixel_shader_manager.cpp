#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include <fstream>

cShaderManager* theShaderManager = nullptr;

cShaderManager::cShaderManager()
{
    theShaderManager = this;
}

cShaderManager::~cShaderManager()
{
    theShaderManager = nullptr;
}

void cShaderManager::init()
{
    // Load and compile all pixel shaders from the "shaders" folder
    std::filesystem::path shaderFolder = std::filesystem::current_path() / "shaders";
    if (!std::filesystem::exists(shaderFolder) || !std::filesystem::is_directory(shaderFolder))
    {
        MainLog->Log("Shader folder not found: %s", shaderFolder.string().c_str());
        return;
    }
    for (const auto& entry : std::filesystem::directory_iterator(shaderFolder))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".hlsl")
        {
            std::string shaderName = entry.path().stem().string();
            std::ifstream shaderFile(entry.path());
            if (!shaderFile)
            {
                MainLog->Log("Failed to open shader file: %s", entry.path().string().c_str());
                continue;
            }
            std::string shaderSource((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
            if (shaderName.ends_with("_ps"))
            {
                auto pixelShader = std::make_shared<cPixelShader>(shaderSource);
                if (*pixelShader)
                {
                    mPixelShaders[shaderName] = pixelShader;
                    MainLog->Log("Loaded pixel shader: %s", shaderName.c_str());
                }
            }
            else if (shaderName.ends_with("_vs"))
            {
                auto vertexShader = std::make_shared<cVertexShader>(shaderSource);
                if (*vertexShader)
                {
                    mVertexShaders[shaderName] = vertexShader;
                    MainLog->Log("Loaded vertex shader: %s", shaderName.c_str());
                }
            }
            else
            {
                MainLog->Log("Unknown shader type for file: %s", entry.path().string().c_str());
            }
        }
    }
}

std::shared_ptr<cPixelShader> cShaderManager::pixelShader(const std::string& name) const
{
    auto it = mPixelShaders.find(name);
    if (it == mPixelShaders.end())
        return nullptr;
    return it->second;
}

std::shared_ptr<cVertexShader> cShaderManager::vertexShader(const std::string& name) const
{
    auto it = mVertexShaders.find(name);
    if (it == mVertexShaders.end())
        return nullptr;
    return it->second;
}


