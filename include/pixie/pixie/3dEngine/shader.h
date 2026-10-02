#pragma once


class cShader
{
    std::string mName;
protected:
    const std::string& name() const { return mName; }
    ID3DBlob* compile(std::string_view sourceCode, const std::string& entryPoint, const std::string& target);
    static void dumpDebugInfo(cLog& log, ID3DBlob* blob, bool input);

    // returns true on success
    cShader(const std::string& name) : mName(name) {}
public:
    virtual ~cShader();
};