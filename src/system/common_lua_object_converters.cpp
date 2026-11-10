#include "StdAfx.h"

cStringVector cLuaConverter<cStringVector>::convert(const cLuaObject& object)
{
    if (ASSERTFALSE(!object.isTable()))
    {
        return {};
    }
    cStringVector result;
    for (auto&& element : object | std::views::values)
    {
        result.push_back(element.toString());
    }
    return result;
}

std::vector<std::string> cLuaConverter<std::vector<std::string>>::convert(const cLuaObject& object)
{
    if (ASSERTFALSE(!object.isTable()))
    {
        return {};
    }
    std::vector<std::string> result;
    for (auto&& element : object | std::views::values)
    {
        result.push_back(element.toString());
    }
    return result;
}

std::vector<int> cLuaConverter<std::vector<int>>::convert(const cLuaObject& object)
{
    if (ASSERTFALSE(!object.isTable()))
    {
        return {};
    }
    std::vector<int> result;
    for (auto&& element : object | std::views::values)
    {
        result.push_back(element.toInt());
    }
    return result;
}

std::vector<float> cLuaConverter<std::vector<float>>::convert(const cLuaObject& object)
{
    if (ASSERTFALSE(!object.isTable()))
    {
        return {};
    }
    std::vector<float> result;
    for (auto&& element : object | std::views::values)
    {
        result.push_back(static_cast<float>(element.toDouble()));
    }
    return result;
}

std::vector<double> cLuaConverter<std::vector<double>>::convert(const cLuaObject& object)
{
    if (ASSERTFALSE(!object.isTable()))
    {
        return {};
    }
    std::vector<double> result;
    for (auto&& element : object | std::views::values)
    {
        result.push_back(element.toDouble());
    }
    return result;
}