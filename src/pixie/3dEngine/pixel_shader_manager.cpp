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
            auto lastUnderscorePos = shaderName.find_last_of('_');
            auto shaderTypeString = shaderName.substr(lastUnderscorePos + 1);    
            if (shaderTypeString == "inc")
            {
                // include file, skip
                continue;
            }
            mShaderFiles.emplace_back(entry.path(), shaderNamePrefix, shaderNamePrefix + shaderName.substr(0, lastUnderscorePos));
            auto [success, metaData] = processShaderFile(entry.path(), shaderTypeString, shaderNamePrefix);
            if(success)
                mShaderFiles.back().mScript = std::make_shared<cShaderFileScript>(cShaderFileScript{ std::move(metaData) });

        }
        else if (entry.is_directory())
        {
            loadShadersFromFolder(entry.path(), shaderNamePrefix + entry.path().filename().string() + ".");
        }
    }
}
//
//bool cShaderManager::compilePixelShader(std::shared_ptr<cPixelShader> pixelShader, cShaderSource& shaderSource)
//{
//    auto metaData = shaderSource.metaData();
//    auto presets = metaData.state().globalTable().get("presets");
//    for (auto&& [key, constantTable] : metaData.get("constants"))
//    {
//        constantTable.set("value", constantTable.get("default"));
//        std::string shaderName = pixelShader->name();
//        auto subPreset = presets;
//        while(!shaderName.empty())
//        {
//            auto dotPos = shaderName.find('.');
//            std::string groupName = shaderName.substr(0, dotPos);
//            if (!subPreset.has(groupName))
//                break;
//            subPreset = subPreset.get(groupName);
//            if (subPreset.has(key))
//            {
//                constantTable.set("value", subPreset.get(key));
//            }
//            if (dotPos == std::string::npos)
//                break;
//            shaderName = shaderName.substr(dotPos + 1);
//        }
//    }
//    if (!pixelShader->compile(shaderSource))
//    {
//        MainLog->Log("Failed to compile pixel shader: {}", pixelShader->name());
//        return false;
//    }
//    return true;
//}

void cShaderManager::loadIncludes()
{
    mIncludeFiles.clear();
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
            mIncludeFiles[shaderName] = cShaderSource(shaderSource);
        }
        else if (entry.is_directory())
        {
            loadIncludesFromFolder(entry.path(), shaderNamePrefix + entry.path().filename().string() + "/");
        }
    }
}

void cShaderManager::loadShaderConstants()
{
    if (std::filesystem::exists(mShaderPresetPath))
    {
        auto state = std::make_shared<cLuaState>();
        state->executeFile(mShaderPresetPath);
        mShaderPreset = state->globalTable().get("presets");
    }
}

std::pair<bool, cLuaObject> cShaderManager::processShaderFile(
    const std::filesystem::path& path, const std::string& shaderTypeString, const std::string& shaderNamePrefix)
{
    std::ifstream shaderFile(path);
    if (!shaderFile)
    {
        MainLog->Log("Failed to open shader file: {}", path.string());
        return {false, {}};
    }
    bool success = true;
    std::string originalShaderSource((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
    cShaderSource shaderSource(originalShaderSource);
    auto metaData = shaderSource.metaData();
    std::string shaderName = shaderNamePrefix + path.stem().string();
    shaderName = shaderName.substr(0, shaderName.find_last_of('_'));
    cLuaObject preset;
    if (mShaderPreset.has(shaderName))
    {
        preset = mShaderPreset.get(shaderName);
    }
    if (shaderTypeString == "ps")
    {
        metaData.state().executeString(std::format(R"(
            if constants == nil then
                constants = {{}}
            end)")); // the global constants
        if (metaData.has("shaders"))
        {
            cShaderGroup shaderGroup;
            for (auto&& [key, subShaderMeta] : metaData.get("shaders"))
            {
                auto subShaderName = std::format("{}.{}", shaderName, subShaderMeta.get<std::string>("name"));
                auto& pixelShader = mPixelShaders[subShaderName];
                if (!pixelShader)
                    pixelShader = std::make_shared<cPixelShader>(subShaderName);                
                shaderGroup.mShaders.push_back(pixelShader);
                auto& metaObj = shaderGroup.mMetaData.emplace_back();
                metaObj.setShaderName(subShaderName);
                metaObj.apply(metaData);
                metaObj.apply(subShaderMeta);

                metaObj.setConstantsFromPreset(preset);
                if (preset.has(subShaderMeta.get<std::string>("name")))
                {
                    metaObj.setConstantsFromPreset(preset.get(subShaderMeta.get<std::string>("name")));
                }
            }
            cPixelShader::cMetaData::synchronize(shaderGroup.mMetaData);
            for (auto&& [pixelShader, metaObj] : std::views::zip(shaderGroup.mShaders, shaderGroup.mMetaData))
            {
                if (!pixelShader->compile(shaderSource, std::move(metaObj)))
                {
                    MainLog->Log("Failed to compile pixel shader: {}", pixelShader->name());
                    success = false;
                }
            }
        }
        else
        {
            auto& pixelShader = mPixelShaders[shaderName];
            if (!pixelShader)
                pixelShader = std::make_shared<cPixelShader>(shaderName);
            cPixelShader::cMetaData shaderMetaData;
            shaderMetaData.setShaderName(shaderName);
            shaderMetaData.apply(metaData);
            shaderMetaData.setConstantsFromPreset(preset);
            if(!pixelShader->compile(shaderSource, std::move(shaderMetaData)))
            {
                MainLog->Log("Failed to compile pixel shader: {}", pixelShader->name());
                success = false;
            }
        }
    }
    else if (shaderTypeString == "vs")
    {
        auto vertexShader = std::make_shared<cVertexShader>(shaderName, shaderSource.sourceCode());
        if (*vertexShader)
        {
            mVertexShaders[shaderName] = vertexShader;
            MainLog->Log("Loaded vertex shader: {}", shaderName);
        }
        else
            success = false;
    }
    else
    {
        MainLog->Log("Unknown shader type for file: {}", path.string());
        return {true, {} }; // not a shader file, not a shader error
    }
    return { success, metaData };
}

void cShaderManager::reloadShaders()
{
    bool allSuccess = true;
    for (auto&& [shaderFile, shaderNamePrefix, shaderFileID,   scriptObj] : mShaderFiles)
    {
        std::string shaderName = shaderFile.stem().string();
        auto shaderTypeString = shaderName.substr(shaderName.find_last_of('_') + 1);
        auto [success, metaData] = processShaderFile(shaderFile, shaderTypeString, shaderNamePrefix);
        if (success)
        {
            if (!scriptObj)
            {
                scriptObj = std::make_shared<cShaderFileScript>(cShaderFileScript{ std::move(metaData) });
            }
            else
            {
                scriptObj->mMetaData = std::move(metaData);
            }
        }
        else
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


cShaderSource& cShaderManager::includeFile(const std::string& name) 
{
    static cShaderSource emptySource;
    auto it = mIncludeFiles.find(name);
    if (it == mIncludeFiles.end())
        return emptySource;
    return it->second;
}

std::shared_ptr<cShaderFileScript> cShaderManager::shaderScript(const std::string& shaderFileID) const
{
    auto i = std::find_if(mShaderFiles.begin(), mShaderFiles.end(),
        [&shaderFileID](const auto& shaderFile)
        {
            return shaderFile.mShaderFileID == shaderFileID;
        });
    return ASSERTTRUE(i != mShaderFiles.end()) ? i->mScript : nullptr;
}
