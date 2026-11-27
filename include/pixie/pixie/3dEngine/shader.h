#pragma once

class cShaderSource
{
    std::string mSourceCode;
    cLuaObject mMetaData;
public:
    cShaderSource() = default;
    cShaderSource(const cShaderSource&) = default;
    cShaderSource(cShaderSource&&) = default;
    cShaderSource& operator=(const cShaderSource&) = default;
    cShaderSource& operator=(cShaderSource&&) = default;

    cShaderSource(const std::string& sourceCode);
    cShaderSource(const cShaderSource& src, cLuaObject subMeta);
    void insertSourceCode(const std::string& additionalCode);
    cLuaObject metaData() const { return mMetaData; }
    void regexReplace(std::string_view pattern, std::string_view replacement);
    const std::string& sourceCode() const { return mSourceCode; }

    static inline constexpr std::string_view mMetaInsertionPointText = "// @meta_insertion_point"sv;
};

class cShader
{
    std::string mName;
    class cShaderInclude;
protected:
    using cIncludeHandler = std::function<std::string_view(const std::string& includeFileName)>;
    ID3DBlob* compile(
        std::string_view sourceCode, 
        const std::string& entryPoint, 
        const std::string& target,
        cIncludeHandler includeHandler = nullptr);
    static void dumpDebugInfo(cLog& log, ID3DBlob* blob, bool input);

    // returns true on success
    cShader(const std::string& name) : mName(name) {}
public:
    virtual ~cShader();
    const std::string& name() const { return mName; }
};