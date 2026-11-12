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
    mShaderFolder = std::filesystem::current_path() / "shaders";
    mShaderPresetPath = theConfigurationFolder / "shader_constants.lua";
#ifdef _DEBUG
    mPresetChangeNotifier = std::make_unique<Pixie::cFileChangeNotifier>(
        mShaderPresetPath,
        [this]()
        {
            loadShaderConstants();
            reloadShaders();
        });
#endif
    // Load and compile all pixel shaders from the "shaders" folder
    if (!std::filesystem::exists(mShaderFolder) || !std::filesystem::is_directory(mShaderFolder))
    {
        MainLog->Log("Shader folder not found: {}", mShaderFolder.string());
        return;
    }
    loadShaderConstants();
    loadIncludes(); // first pass: gather the include files

    loadShadersFromFolder(mShaderFolder, {}); // second pass: compile the shaders
}

void cShaderManager::loadShadersFromFolder(const std::filesystem::path& folderPath, const std::string& shaderNamePrefix)
{
#ifdef _DEBUG
    mFileChangeNotifiers.emplace_back(std::make_unique<Pixie::cFileChangeNotifier>(
        folderPath, "*.hlsl",
        [this]()
        {
            loadIncludes();
            reloadShaders();
        }));
#endif
    for (const auto& entry : std::filesystem::directory_iterator(folderPath))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".hlsl")
        {
            std::string shaderName =  shaderNamePrefix + entry.path().stem().string();
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
                auto pixelShader = std::make_shared<cPixelShader>(shaderName, shaderSource);
                if (*pixelShader)
                {
                    mPixelShaders[shaderName] = pixelShader;
                    MainLog->Log("Loaded pixel shader: {}", shaderName);
                }
            }
            else if (shaderTypeString == "vs")
            {
                auto vertexShader = std::make_shared<cVertexShader>(shaderName, shaderSource);
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
        else if (entry.is_directory())
        {
            loadShadersFromFolder(entry.path(), shaderNamePrefix + entry.path().filename().string() + "/");
        }
    }
}

void cShaderManager::loadIncludes()
{
    mShaderSources.clear();
    loadIncludesFromFolder(mShaderFolder, {});
}

void cShaderManager::loadIncludesFromFolder(const std::filesystem::path& folderPath, const std::string& shaderNamePrefix)
{
    for (const auto& entry : std::filesystem::directory_iterator(folderPath))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".hlsl")
        {
            std::string shaderName = shaderNamePrefix + entry.path().stem().string();
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
        else if (entry.is_directory())
        {
            loadIncludesFromFolder(entry.path(), shaderNamePrefix + entry.path().filename().string() + "/");
        }
    }
}

void cShaderManager::loadShaderConstants()
{
    auto state = std::make_shared<cLuaState>();
    try
    {
        state->executeFile(mShaderPresetPath);
    }
    catch (const cLuaException& e)
    {
        MainLog->Log("Failed to load shader constants from {}: {}", mShaderPresetPath.string(), e.what());
        return;
    }
    mShaderConstants = state->globalTable();
}

void cShaderManager::reloadShaders()
{
    bool allSuccess = true;
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
        if (!shader->compile(shaderSource))
        {
            allSuccess = false;
        }
    }
    if (allSuccess)
    {
        theMessageCenter.post("pixie.shaders_reloaded");
        theMessageCenter.post("pixie.system_message", "info"s, "shaders reloaded"s);
    }
    else
        theMessageCenter.post("pixie.system_message", "warning"s, "shader reload failed"s);
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

cLuaObject cShaderManager::shaderConstants(const std::string& shaderName) const
{
    if (mShaderConstants.has(shaderName))
        return mShaderConstants.get<cLuaObject>(shaderName);
    else
        return const_cast<cLuaState&>(mShaderConstants.state()).createTable(); // return empty table if not found
}
