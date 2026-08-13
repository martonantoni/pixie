#pragma once


class cShader
{
protected:
    ID3DBlob* compile(std::string_view sourceCode, const std::string& entryPoint, const std::string& target);
    // returns true on success
    cShader() = default;
public:
    virtual ~cShader();
};