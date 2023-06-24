#pragma once

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class cLuaTable;

struct cLuaException
{
    int mLine = 0;
};

class cLuaScript: public std::enable_shared_from_this<cLuaScript>
{
    lua_State* L;
    bool mLoaded = false;
    static int gcUserData(lua_State* L);
    static const char* userDataMetaTableName;
    static std::vector<std::string> globalTableInternalElements;
public:
    cLuaScript();
    virtual ~cLuaScript();
    void executeFile(const cPath& scriptPath);
    void executeString(const std::string& script);
    static void dumpStack(lua_State* L);
    static void staticInit();
    static std::string valueToString(lua_State* L, int index);
    static bool isGlobalInternalElement(const std::string& key);

    lua_State* state() { return L; }
    cLuaTable globalTable();

    struct cUserDataBase
    {
        virtual ~cUserDataBase() = default;
    };
    template<class T, class... Args> T* pushNewUserData(Args&&... args)
    {
        static_assert(std::is_base_of<cUserDataBase, T>::value, "must use a cUserDataBase derived class");
        void* allocationPlace = lua_newuserdata(L, sizeof(T));
        T* userData = new (allocationPlace) T(std::forward<Args>(args)...);
        luaL_setmetatable(L, userDataMetaTableName);
        return userData;
    }
};



class cLuaTable final
{
public:
    enum class IsRecursive { Yes, No };
private:
    std::shared_ptr<cLuaScript> mScript;
    int mReference = LUA_NOREF;
    bool mIsGlobalTable = false;
    template<class T> static void push(lua_State* L, const T& value);
    template<class T, class... Ts> static void push(lua_State* L, const T& value, const Ts&... values);
    template<class T> static T pop(lua_State* L);
    tIntrusivePtr<cConfig> toConfig_topTable(lua_State* L, IsRecursive isRecursive) const;
public:
    cLuaTable() = default;
    cLuaTable(std::shared_ptr<cLuaScript> script, int reference, bool isGlobalTable);
    ~cLuaTable();
    cLuaTable& operator=(const cLuaTable& src) = default;
    template<class T> T get(const std::string& key) const;
    template<class T> void set(const std::string& key, const T& value);
    template<class R, class... Args, class C> void registerFunction(const std::string& key, const C&& func);
    template<class... Args> void callFunction(const std::string& key, Args... args);
    template<class T> bool isType(const std::string& key) const;
    cLuaTable subTable(const std::string& key) const;

    tIntrusivePtr<cConfig> toConfig(IsRecursive isRecursive = IsRecursive::Yes) const;
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

template<typename T> T cLuaTable::get(const std::string& key) const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return {};
    }
    lua_State* L = mScript->state();

    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack
    lua_gettable(L, -2); // Get the value from the table using the variable name

    FINALLY([L]() { lua_pop(L, 2); });

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
    return {};
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

    struct cCallableHolder : public cLuaScript::cUserDataBase
    {
        C mCallable;
        cCallableHolder(const C& callable) : mCallable(callable) {}
        virtual ~cCallableHolder() = default;
    };
    cCallableHolder* holder = mScript->pushNewUserData<cCallableHolder>(func);

    // Create a C function wrapper that calls the callable object
    lua_CFunction cFunction = [](lua_State* L) -> int
    {
        cCallableHolder* holder = static_cast<cCallableHolder*>(lua_touserdata(L, lua_upvalueindex(1)));

        if constexpr (sizeof...(Args) > 0)
        {
            auto arguments = std::make_tuple(pop<Args>(L)...);
            if constexpr (std::is_same_v<R, void>)
            {
                std::apply(holder->mCallable, arguments);
                return 0;
            }
            else
            {
                auto result = std::apply(holder->mCallable, arguments);
                push(L, result);
                return 1;
            }
        }
        else
        {
            if constexpr (std::is_same_v<R, void>)
            {
                (holder->mCallable)();
                return 0;
            }
            else
            {
                auto result = (holder->mCallable)();
                push(L, result);
                return 1;
            }
        }
    };
    lua_pushcclosure(L, cFunction, 1);

    lua_settable(L, -3); // Set the value in the table using the variable name
    lua_pop(L, 1); // Pop the table from the stack
}