#include "StdAfx.h"

const char* cLuaScript::userDataMetaTableName = "destructed_user_data";
std::vector<std::string> cLuaScript::globalTableInternalElements;

void callLuaStaticInit()
{
    cLuaScript::staticInit();
}
REGISTER_AUTO_INIT_FUNCTION_CALL(callLuaStaticInit, eProgramPhases::StaticInit, DefaultInitLabel);

void cLuaScript::staticInit()
{
    auto script = std::make_shared<cLuaScript>();
    auto L = script->L;
    lua_pushglobaltable(L);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        // key: -2, value: -1
        std::string key = valueToString(L, -2);
        globalTableInternalElements.emplace_back(key);
        lua_pop(L, 1); // pop the value, keep the key
    }
    lua_pop(L, 1); // global table
    std::sort(ALL(globalTableInternalElements));
}

bool cLuaScript::isGlobalInternalElement(const std::string& key)
{
    return std::binary_search(ALL(globalTableInternalElements), key);
}

void cLuaScript::dumpStack(lua_State* L)
{
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++) 
    {
        printf("%d\t%s\t", i, luaL_typename(L, i));
        switch (lua_type(L, i))
        {
        case LUA_TNUMBER:
            printf("%g\n", lua_tonumber(L, i));
            break;
        case LUA_TSTRING:
            printf("%s\n", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
            break;
        case LUA_TNIL:
            printf("%s\n", "nil");
            break;
        default:
            printf("%p\n", lua_topointer(L, i));
            break;
        }
    }
    printf("---- STACK END ----\n");
    fflush(stdout);
}

cLuaScript::cLuaScript()
{
    L = luaL_newstate();
    luaL_openlibs(L);
//    lua_atpanic(mState, panicHandler);

    luaL_newmetatable(L, userDataMetaTableName);
    lua_pushcfunction(L, gcUserData);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

cLuaScript::~cLuaScript()
{
    lua_close(L);
}

void cLuaScript::executeFile(const cPath& scriptPath)
{

}

void cLuaScript::executeString(const std::string& script)
{
}

int cLuaScript::gcUserData(lua_State* L)
{
    cUserDataBase* obj = static_cast<cUserDataBase*>(lua_touserdata(L, 1));
    obj->~cUserDataBase();
    return 0;
}

cLuaTable cLuaScript::globalTable()
{
    lua_pushglobaltable(L);
    return cLuaTable{ shared_from_this(), luaL_ref(L, LUA_REGISTRYINDEX), true };
}

cLuaTable cLuaScript::createTable()
{
    lua_newtable(L);
    return cLuaTable{ shared_from_this(), luaL_ref(L, LUA_REGISTRYINDEX), true };
}
