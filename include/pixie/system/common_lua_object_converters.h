#pragma once

template<> struct cLuaConverter<cStringVector>
{
    static cStringVector convert(const cLuaObject& object);
};

template<> struct cLuaConverter<std::vector<std::string>>
{
    static std::vector<std::string> convert(const cLuaObject& object);
};

template<> struct cLuaConverter<std::vector<int>>
{
    static std::vector<int> convert(const cLuaObject& object);
};

template<> struct cLuaConverter<std::vector<float>>
{
    static std::vector<float> convert(const cLuaObject& object);
};

template<> struct cLuaConverter<std::vector<double>>
{
    static std::vector<double> convert(const cLuaObject& object);
};