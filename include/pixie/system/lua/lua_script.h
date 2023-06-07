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

    lua_State* state() { return mState; }
    cLuaTable globalTable();
};

class cLuaTable final
{
    std::shared_ptr<cLuaScript> mScript;
    int mReference = LUA_NOREF;
    template<class T> static void pushFunction(lua_State* L, const T& callable);
    template<class T> static void push(lua_State* L, const T& value);
    template<class T, class... Ts> static void push(lua_State* L, const T& value, const Ts&... values);
    template<class T> static T pop(lua_State* L);
public:
    cLuaTable() = default;
    cLuaTable(std::shared_ptr<cLuaScript> script, int reference);
    ~cLuaTable();
    cLuaTable& operator=(const cLuaTable& src) = default;
    template<class T> std::pair<T, bool> get(const std::string& key) const;
    template<class T> void set(const std::string& key, const T& value);
    template<class R, class... Args, class C> void registerFunction(const std::string& key, const C&& func);
    template<class... Args> void callFunction(const std::string& key, Args... args);
    template<class T> bool isType(const std::string& key) const;
    cLuaTable subTable(const std::string& key) const;
};

template<class T> void cLuaTable::push(lua_State* L, const T& value)
{
    if constexpr (std::is_same_v<T, int>)
    {
        lua_pushinteger(L, value);
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        lua_pushboolean(L, value);
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        lua_pushnumber(L, value);
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        lua_pushstring(L, value.c_str()); 
    }   
}

template<class T, class... Ts> void cLuaTable::push(lua_State* L, const T& value, const Ts&... values)
{
    push(L, value);
    push(L, values...);
}

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

    push(L, value);

    lua_settable(L, -3); // Set the value in the table using the variable name
    lua_pop(L, 1); // Pop the table from the stack
}

template<class T>
T cLuaTable::pop(lua_State* L)
{
    FINALLY([&]() { lua_pop(L, 1); });
    if constexpr (std::is_same_v<T, int>)
    {
        if (lua_isinteger(L, -1))
        {
            return static_cast<T>(lua_tointeger(L, -1));
        }
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        if (lua_isnumber(L, -1))
        {
            return static_cast<T>(lua_tonumber(L, -1));
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (lua_isstring(L, -1))
        {
            return static_cast<T>(lua_tostring(L, -1));
        }
    }
    // handle error
    return T{};
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

template<class... Args> void cLuaTable::callFunction(const std::string& key, Args... args)
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return;
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack
    lua_gettable(L, -2); // Get the value from the table using the variable name
    if (!lua_isfunction(L, -1))
    {
        // handle error
        return;
    }
    if constexpr (sizeof...(args) > 0)
    {
        push(L, args...);
    }
    int status = lua_pcall(L, sizeof...(args), 0, 0);
}


template<class T> void cLuaTable::pushFunction(lua_State* L, const T& callable)
{
    lua_State* L = mScript->state();
    // Create a light userdata to hold the callable object
    void* userdata = lua_newuserdata(L, sizeof(Callable));
    new (userdata) Callable(std::forward<Callable>(callable));

    // Create a C function wrapper that calls the callable object
    lua_CFunction cFunction = [](lua_State* L) -> int
    {
        Callable* callable = static_cast<Callable*>(lua_touserdata(L, lua_upvalueindex(1)));

        // Invoke the callable object
        if constexpr (std::is_same_v<decltype((*callable)()), void>)
        {
            (*callable)();
            return 0;
        }
        else
        {
            auto result = (*callable)();
            push(L, result);
            return 1;
        }
    };
    lua_pushcclosure(L, cFunction, 1);
}

template<class R, class... Args, class C>
void cLuaTable::registerFunction(const std::string& key, const C&& func)
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return;
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack

    using FuncType = std::function<R(Args...)>;

    void* userdata = lua_newuserdata(L, sizeof(FuncType));
    new (userdata) FuncType(func);

    // Create a C function wrapper that calls the callable object
    lua_CFunction cFunction = [](lua_State* L) -> int
    {
        FuncType* f = static_cast<FuncType*>(lua_touserdata(L, lua_upvalueindex(1)));

        if constexpr (sizeof...(Args) > 0)
        {
            auto arguments = std::make_tuple(pop<Args>(L)...);
            if constexpr (std::is_same_v<R, void>)
            {
                std::apply(*f, arguments);
                return 0;
            }
            else
            {
                auto result = std::apply(*f, arguments);
                push(L, result);
                return 1;
            }
        }
        else
        {
            if constexpr (std::is_same_v<R, void>)
            {
                (*f)();
                return 0;
            }
            else
            {
                auto result = (*f)();
                push(L, result);
                return 1;
            }
        }
    };
    lua_pushcclosure(L, cFunction, 1);

    lua_settable(L, -3); // Set the value in the table using the variable name
    lua_pop(L, 1); // Pop the table from the stack
}