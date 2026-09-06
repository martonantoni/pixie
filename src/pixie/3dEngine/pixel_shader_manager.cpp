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
        MainLog->Log("Shader folder not found: {}", shaderFolder.string());
        return;
    }
    // first pass: gather the include files:
    for (const auto& entry : std::filesystem::directory_iterator(shaderFolder))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".hlsl")
        {
            std::string shaderName = entry.path().stem().string();
            auto shaderTypeString = shaderName.substr(shaderName.find_last_of('_') + 1);
            if (shaderTypeString != "inc")
                continue; // not an include file

            std::ifstream shaderFile(entry.path());
            if (!shaderFile)
            {
                MainLog->Log("Failed to open shader file: {}", entry.path().string());
                continue;
            }
            std::string shaderSource((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
            shaderName = shaderName.substr(0, shaderName.find_last_of('_'));
            mShaderSources[shaderName] = shaderSource;
        }
    }
    for (const auto& entry : std::filesystem::directory_iterator(shaderFolder))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".hlsl")
        {
            std::string shaderName = entry.path().stem().string();
            auto shaderTypeString = shaderName.substr(shaderName.find_last_of('_') + 1);
            if (shaderTypeString == "inc")
            {
                // include file, skip
                continue;
            }
            std::ifstream shaderFile(entry.path());
            if (!shaderFile)
            {
                MainLog->Log("Failed to open shader file: {}", entry.path().string());
                continue;
            }
            std::string shaderSource((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
            shaderName = shaderName.substr(0, shaderName.find_last_of('_'));
            if (shaderTypeString == "ps")
            {
                auto pixelShader = std::make_shared<cPixelShader>(shaderSource);
                if (*pixelShader)
                {
                    mPixelShaders[shaderName] = pixelShader;
                    MainLog->Log("Loaded pixel shader: {}", shaderName);
                }
            }
            else if (shaderTypeString == "vs")
            {
                auto vertexShader = std::make_shared<cVertexShader>(shaderSource);
                if (*vertexShader)
                {
                    mVertexShaders[shaderName] = vertexShader;
                    MainLog->Log("Loaded vertex shader: {}", shaderName);
                }
            }
            else
            {
                MainLog->Log("Unknown shader type for file: {}", entry.path().string());
            }
        }
    }
}

void cShaderManager::reloadShaders()
{
    for (auto&& [name, shader] : mPixelShaders)
    {
        std::filesystem::path shaderFile = std::filesystem::current_path() / "shaders" / (name + "_ps.hlsl");
        if (!std::filesystem::exists(shaderFile))
        {
            MainLog->Log("Shader file not found: {}", shaderFile.string());
            continue;
        }
        std::ifstream shaderFileStream(shaderFile);
        if (!shaderFileStream)
        {
            MainLog->Log("Failed to open shader file: {}", shaderFile.string());
            continue;
        }
        std::string shaderSource((std::istreambuf_iterator<char>(shaderFileStream)), std::istreambuf_iterator<char>());
        shader->compile(shaderSource);
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


std::string_view cShaderManager::shaderSource(const std::string& name) const
{
    auto it = mShaderSources.find(name);
    if (it == mShaderSources.end())
        return {};
    return it->second;
}
