#pragma once

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class cLuaObject;

struct cLuaException
{
    int mLine = 0;
};

class cLuaState: public std::enable_shared_from_this<cLuaState>
{
    lua_State* L;
    bool mIsOwningState = true;
    static int gcUserData(lua_State* L);
    static const char* userDataMetaTableName;
    static std::vector<std::string> globalTableInternalElements;
    static int panicHandler(lua_State* L);
public:
    cLuaState();
    cLuaState(lua_State* l);
    cLuaState(cLuaState&& src);
    cLuaState& operator=(cLuaState&& src);
    cLuaState(const cLuaState&) = delete;
    cLuaState& operator=(const cLuaState&) = delete;
    virtual ~cLuaState();
    void executeFile(const cPath& scriptPath);
    void executeString(const std::string& script);
    std::shared_ptr<cLuaState> shareSelf() { return shared_from_this(); }
    static void staticInit();
    static std::string valueToString(lua_State* L, int index);
    static bool isGlobalInternalElement(const std::string& key);
    static std::string configToScript(const cConfig& config, const std::string& ident = std::string());

    lua_State* state() { return L; }
    cLuaObject globalTable();
    cLuaObject createTable();
    cLuaObject createValue();

    struct cUserDataBase
    {
        virtual ~cUserDataBase() = default;
    };
    template<class T, class... Args> T* pushNewUserData(Args&&... args)
    {
        static_assert(std::is_base_of<cUserDataBase, T>::value, "must use a cUserDataBase derived class");
        void* allocationPlace = lua_newuserdata(L, sizeof(T));
        T* userData = new (allocationPlace) T(std::forward<Args>(args)...);
        luaL_setmetatable(L, userDataMetaTableName); // this metatable was created in the constructor
                                                     // it is responsible for custom GC destructor
        return userData;
    }

// shortcuts for creating config from lua script:
    static std::shared_ptr<cConfig> stringToConfig(const std::string& scriptText);
    static std::shared_ptr<cConfig> fileToConfig(const cPath& scriptPath);

// debug functions:
    int stackSize() const;
    static void dumpStack(lua_State* L);
    static void error(lua_State* L, const std::string& message);
};

