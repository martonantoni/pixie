#pragma once

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class cLuaTable;

class cLuaScript: public std::enable_shared_from_this<cLuaScript>
{
    lua_State* mState;
    bool mLoaded = false;
public:
    cLuaScript();
    virtual ~cLuaScript();
    void executeFile(const cPath& path);

    lua_State* state() const { return mState; }
    cLuaTable globalTable() const;
};

class cLuaTable final
{
    std::shared_ptr<cLuaScript> mScript;
    int mReference = LUA_NOREF;
public:
    cLuaTable() = default;
    cLuaTable(std::shared_ptr<cLuaScript> script, int reference);
    ~cLuaTable();
    cLuaTable& operator=(const cLuaTable& src) = default;
    template<class T> std::pair<T, bool> get(const std::string& key) const;
    template<class T> void set(const std::string& key, const T& value);
    template<class T> bool isType(const std::string& key) const;
    cLuaTable subTable(const std::string& key) const;
};


template <typename T>
void cLuaTable::set(const std::string& key, const T& value)
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return;
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack

    if constexpr (std::is_same_v<T, int>) 
    {
        lua_pushinteger(L, value); // Push the integer value onto the Lua stack
    }
    else if constexpr (std::is_same_v<T, double>) 
    {
        lua_pushnumber(L, value); // Push the double value onto the Lua stack
    }
    else if constexpr (std::is_same_v<T, std::string>) 
    {
        lua_pushstring(L, value.c_str()); // Push the string value onto the Lua stack
    }
    else 
    {
        // Handle unsupported types here
        // You can throw an exception or provide an appropriate error message
        // depending on your specific use case
    }

    lua_settable(L, -3); // Set the value in the table using the variable name
    lua_pop(L, 1); // Pop the table from the stack
}


template <typename T>
std::pair<T, bool> cLuaTable::get(const std::string& key) const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return { {}, false };
    }
    lua_State* L = mScript->state();
    std::pair<T, bool> result;

    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack
    lua_gettable(L, -2); // Get the value from the table using the variable name

    if constexpr (std::is_same_v<T, int>)
    {
        if (lua_isinteger(L, -1))
        {
            result.first = static_cast<T>(lua_tointeger(L, -1));
            result.second = true;
        }
        else
        {
            result.second = false;
        }
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        if (lua_isnumber(L, -1))
        {
            result.first = static_cast<T>(lua_tonumber(L, -1));
            result.second = true;
        }
        else
        {
            result.second = false;
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (lua_isstring(L, -1))
        {
            result.first = static_cast<T>(lua_tostring(L, -1));
            result.second = true;
        }
        else
        {
            result.second = false;
        }
    }
    else
    {
        // Handle unsupported types here
        // You can throw an exception or provide an appropriate error message
        // depending on your specific use case
        result.second = false;
    }

    lua_pop(L, 2); // Pop the value and the table from the stack
    return result;
}


template <typename T>
bool cLuaTable::isType(const std::string& variableName) const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return false;
    }
    lua_State* L = mScript->state();
    bool result = false;

    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, variableName.c_str()); // Push the variable name onto the Lua stack
    lua_gettable(L, -2); // Get the value from the table using the variable name

    if constexpr (std::is_same_v<T, int>)
    {
        result = lua_isinteger(L, -1);
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        result = lua_isnumber(L, -1);
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        result = lua_isstring(L, -1);
    }
    else
    {
        // Handle unsupported types here
        // You can throw an exception or provide an appropriate error message
        // depending on your specific use case
        result = false;
    }

    lua_pop(L, 2); // Pop the value and the table from the stack
    return result;
}
