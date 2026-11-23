#pragma once

class cShaderSource
{
    std::string mSourceCode;
    cLuaObject mMetaData;
public:
    cShaderSource() = default;
    cShaderSource(const cShaderSource&) = delete;
    cShaderSource(cShaderSource&&) = default;
    cShaderSource& operator=(const cShaderSource&) = delete;
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
protected:
    ID3DBlob* compile(std::string_view sourceCode, const std::string& entryPoint, const std::string& target);
    static void dumpDebugInfo(cLog& log, ID3DBlob* blob, bool input);

    // returns true on success
    cShader(const std::string& name) : mName(name) {}
public:
    virtual ~cShader();
    const std::string& name() const { return mName; }
};