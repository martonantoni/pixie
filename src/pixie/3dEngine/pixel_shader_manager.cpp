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
    mFileChangeNotifier = std::make_unique<Pixie::cFileChangeNotifier>(
        mShaderFolder, "*.hlsl", 
        [this]() 
        { 
            loadIncludes(); 
            reloadShaders(); 
        });
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

    for (const auto& entry : std::filesystem::directory_iterator(mShaderFolder))
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
    }
}

void cShaderManager::loadIncludes()
{
    for (const auto& entry : std::filesystem::directory_iterator(mShaderFolder))
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
    mShaderConstants.clear();
    for (auto&& [groupName, groupTable] : state->globalTable())
    {
        if (!groupTable.isTable())
        {
            continue;
        }
        std::vector<cShaderConstant> constants;
        for (auto&& [constantName, constantValue] : groupTable)
        {
            cShaderConstant constant;
            constant.mName = constantName.toString();
            if (constantValue.isNumber())
                constant.mValue = static_cast<float>(constantValue.toDouble());
            else
            {
                if(constantValue.isTable() && constantValue.arraySize() == 3)
                {
                    std::array<float, 3> f3;
                    for (int i = 0; i < 3; i++)
                    {
                        auto item = constantValue.get(i + 1);
                        if (!item.isNumber())
                        {
                            MainLog->Log("Invalid shader constant value for {}.{}: array element {} is not a number", groupName.toString(), constantName.toString(), i + 1);
                            break;
                        }
                        f3[i] = static_cast<float>(item.toDouble());
                    }
                    constant.mValue = f3;
                }
                else
                    MainLog->Log("Invalid shader constant value for {}.{}: not a number or array of 3 numbers   ", groupName.toString(), constantName.toString());
            }
            constants.push_back(constant);
        }
        mShaderConstants[groupName.toString()] = std::move(constants);
    }
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

const std::vector<cShaderConstant>& cShaderManager::shaderConstants(const std::string& groupName) const
{
    auto it = mShaderConstants.find(groupName);
    if (it == mShaderConstants.end())
    {
        static const std::vector<cShaderConstant> empty;
        return empty;
    }
    return it->second;
}