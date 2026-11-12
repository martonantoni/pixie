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
            auto [metaData, finalSourceCode] = extractMetaInfo(std::move(shaderSource));
            shaderName = shaderName.substr(0, shaderName.find_last_of('_'));
            if (shaderTypeString == "ps")
            {
                auto pixelShader = std::make_shared<cPixelShader>(shaderName);
                compilePixelShader(pixelShader, finalSourceCode, metaData);
                if (*pixelShader)
                {
                    mPixelShaders[shaderName] = pixelShader;
                    mPixelShadersByFile.emplace_back();
                    mPixelShadersByFile.back().first = entry.path();
                    mPixelShadersByFile.back().second.emplace_back(pixelShader);
                    MainLog->Log("Loaded pixel shader: {}", shaderName);
                }
            }
            else if (shaderTypeString == "vs")
            {
                auto vertexShader = std::make_shared<cVertexShader>(shaderName, finalSourceCode);
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
            loadShadersFromFolder(entry.path(), shaderNamePrefix + entry.path().filename().string() + ".");
        }
    }
}

bool cShaderManager::compilePixelShader(std::shared_ptr<cPixelShader> pixelShader, std::string finalSourceCode, cLuaObject metaData)
{
    if (metaData.has("constants"s))
    {
        metaData.state().executeString(mShadersPreset);
        metaData.state().executeString(R"(
            for name, value in pairs(constants) do
                value.value = value.default
            end)");
        std::string_view shaderName = pixelShader->name();
        for (size_t dotPos = 0; dotPos != std::string::npos;)
        {
            dotPos = shaderName.find('.', dotPos + 1);
            std::string_view groupName = shaderName.substr(0, dotPos);
            metaData.state().executeString(std::format(R"(
            if presets.{0} == nil then
                presets.{0} = {{}}
            end
            local shaderPresets = presets.{0}
            for name, value in pairs(constants) do
                local preset_value = shaderPresets[name]
                if preset_value ~= nil then
                    value.value = preset_value
                end
            end)", groupName));
        }
    }
    if (!pixelShader->compile(std::move(finalSourceCode), cPixelShader::mDefaultEntryPoint, metaData))
    {
        MainLog->Log("Failed to compile pixel shader: {}", pixelShader->name());
        return false;
    }
    return true;
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
    std::ifstream shaderPresetFile(mShaderPresetPath);
    if (!shaderPresetFile)
    {
        MainLog->Log("Failed to open shader preset file: {}", mShaderPresetPath.string());
        return;
    }
    mShadersPreset = std::string((std::istreambuf_iterator<char>(shaderPresetFile)), std::istreambuf_iterator<char>());
}

void cShaderManager::reloadShaders()
{
    bool allSuccess = true;
    for (auto&& [filePath, pixelShaders] : mPixelShadersByFile)
    {
        std::ifstream shaderFile(filePath);
        if (!shaderFile)
        {
            MainLog->Log("Failed to open shader file: {}", filePath.string());
            allSuccess = false;
            continue;
        }
        std::string shaderSource((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
        auto [metaData, finalSourceCode] = extractMetaInfo(shaderSource);
        for (auto&& pixelShader : pixelShaders)
        {
            if (!compilePixelShader(pixelShader, finalSourceCode, metaData))
            {
                allSuccess = false;
            }
        }
    }
    //for (auto&& [name, shader] : mPixelShaders)
    //{

    //    std::filesystem::path shaderFile = std::filesystem::current_path() / "shaders";// / (name + "_ps.hlsl");
    //    for (auto part : cStringViewVector(name, '.'))
    //    {
    //        shaderFile /= part;
    //    }
    //    shaderFile += "_ps.hlsl";
    //    if (!std::filesystem::exists(shaderFile))
    //    {
    //        MainLog->Log("Shader file not found: {}", shaderFile.string());
    //        continue;
    //    }
    //    std::ifstream shaderFileStream(shaderFile);
    //    if (!shaderFileStream)
    //    {
    //        MainLog->Log("Failed to open shader file: {}", shaderFile.string());
    //        continue;
    //    }
    //    std::string shaderSource((std::istreambuf_iterator<char>(shaderFileStream)), std::istreambuf_iterator<char>());
    //    auto [metaData, finalSourceCode] = extractMetaInfo(shaderSource);
    //    if (!compilePixelShader(shader, std::move(finalSourceCode), metaData))
    //    {
    //        allSuccess = false;
    //    }
    //}
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

std::pair<cLuaObject, std::string> cShaderManager::extractMetaInfo(std::string_view sourceCode)
{
    std::string finalSourceCode;

    // meta data is in the form of comments like this:
    // @cfg
    // lua code (does not need leading //)
    // @endcfg


    std::string configSource;
    bool inConfig = false;
    int cfgPos = 0;
    bool metaInsertionPointAdded = false;

    for (auto&& lineRange : sourceCode | std::views::split('\n'))
    {
        std::string line(lineRange.begin(), lineRange.end());

        auto commentPos = line.find("//");
        if (commentPos != std::string::npos)
        {
            std::string_view comment(line.data() + commentPos + 2, line.size() - commentPos - 2);

            if (comment.find("@cfg") != std::string_view::npos)
            {
                inConfig = true;
                cfgPos = static_cast<int>(finalSourceCode.size());
                continue;
            }
            if (comment.find("@endcfg") != std::string_view::npos)
            {
                inConfig = false;
                continue;
            }
            if (inConfig)
            {
                line = std::string(comment);
            }
        }
        if (inConfig)
        {
            configSource.append(line);
            if(!metaInsertionPointAdded)
            {
                finalSourceCode.append(cPixelShader::mMetaInsertionPointText);
                metaInsertionPointAdded = true;
            }
            configSource += '\n';
            finalSourceCode += '\n'; // keep line numbers consistent for error reporting
        }
        else
        {
            finalSourceCode.append(line);
            finalSourceCode += '\n';
        }
    }

    auto state = std::make_shared<cLuaState>();
    state->executeString(configSource);
    return { state->globalTable(), finalSourceCode };
}