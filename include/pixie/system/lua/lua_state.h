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
    inline static std::vector<std::string> globalTableInternalElements;
    static int panicHandler(lua_State* L);
    inline static std::once_flag mStaticInitFlag{};
    static void staticInit();
    static lua_State* createLuaState();
public:
    cLuaState();
    cLuaState(lua_State* l);
    cLuaState(cLuaState&& src);
    cLuaState& operator=(cLuaState&& src);
    cLuaState(const cLuaState&) = delete;
    cLuaState& operator=(const cLuaState&) = delete;
    virtual ~cLuaState();
    void executeFile(const std::filesystem::path& scriptPath);
    void executeString(const std::string& script);
    std::shared_ptr<cLuaState> shareSelf() { return shared_from_this(); }
    static std::string valueToString(lua_State* L, int index);
    static bool isGlobalInternalElement(const std::string& key);
    struct cConfigToScriptStyle { bool singleLine = false; };
    static std::string configToScript(const cConfig& config, const cConfigToScriptStyle& style = {}, const std::string& ident = std::string());

    lua_State* state() { return L; }
    cLuaObject globalTable();
    cLuaObject createTable();
    cLuaObject createObject();

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
    static std::shared_ptr<cConfig> fileToConfig(const std::filesystem::path& scriptPath);

// debug functions:
    int stackSize() const;
    static void dumpStack(lua_State* L);
    [[noreturn]] static void error(lua_State* L, const std::string& message);
};

class cLuaStackGuard final
{
    lua_State* L;
    int mOldTop;
public:
    explicit cLuaStackGuard(lua_State* l) : L(l), mOldTop(lua_gettop(L)) {}
    explicit cLuaStackGuard(cLuaState& state) : L(state.state()), mOldTop(lua_gettop(L)) {}
    ~cLuaStackGuard() { lua_settop(L, mOldTop); }
    int oldTop() const { return mOldTop; }
};
