#pragma once

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class cLuaValue;

struct cLuaException
{
    int mLine = 0;
};

class cLuaScript: public std::enable_shared_from_this<cLuaScript>
{
    lua_State* L;
    bool mIsOwningState = true;
    static int gcUserData(lua_State* L);
    static const char* userDataMetaTableName;
    static std::vector<std::string> globalTableInternalElements;
    static int panicHandler(lua_State* L);
public:
    cLuaScript();
    cLuaScript(lua_State* l);
    cLuaScript(cLuaScript&& src);
    cLuaScript& operator=(cLuaScript&& src);
    cLuaScript(const cLuaScript&) = delete;
    cLuaScript& operator=(const cLuaScript&) = delete;
    virtual ~cLuaScript();
    void executeFile(const cPath& scriptPath);
    void executeString(const std::string& script);
    static void staticInit();
    static std::string valueToString(lua_State* L, int index);
    static bool isGlobalInternalElement(const std::string& key);
    static std::string configToScript(const cConfig2& config, const std::string& ident = std::string());

    lua_State* state() { return L; }
    cLuaValue globalTable();
    cLuaValue createTable();

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

// shortcuts for creating config from lua script:
    static tIntrusivePtr<cConfig> stringToConfig(const std::string& scriptText);
    static tIntrusivePtr<cConfig> fileToConfig(const cPath& scriptPath);

// debug functions:
    int stackSize() const;
    static void dumpStack(lua_State* L);
    static void error(lua_State* L, const std::string& message);
};

