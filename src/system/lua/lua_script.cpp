#include "StdAfx.h"

const char* cLuaScript::userDataMetaTableName = "destructed_user_data";

cLuaScript::cLuaScript()
{
    mState = luaL_newstate();
    luaL_openlibs(mState);
//    lua_atpanic(mState, panicHandler);

    luaL_newmetatable(mState, userDataMetaTableName);
    lua_pushcfunction(mState, gcUserData);
    lua_setfield(mState, -2, "__gc");
    lua_pop(mState, 1);
}

cLuaScript::~cLuaScript()
{
    lua_close(mState);
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
    lua_pushglobaltable(mState);
    return cLuaTable{ shared_from_this(), luaL_ref(mState, LUA_REGISTRYINDEX) };
}

cLuaTable::cLuaTable(std::shared_ptr<cLuaScript> script, int reference)
    : mScript(std::move(script))
    , mReference(reference)
{

}

cLuaTable::~cLuaTable()
{
    if (mScript && mReference != LUA_NOREF)
    {
        luaL_unref(mScript->state(), LUA_REGISTRYINDEX, mReference);
    }
}

cLuaTable cLuaTable::subTable(const std::string& key) const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return {};
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);

    lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack
    int type = lua_gettable(L, -2); // Get the value from the table using the variable name
    if (type == LUA_TNIL)
    {        
        lua_pop(L, 1); // pops the NIL
        lua_newtable(L); // create new table
        lua_pushstring(L, key.c_str()); // name of the entry in the orig table
        lua_pushvalue(L, -2); 
        lua_settable(L, -4); // pops key and value 
    }
    int reference = luaL_ref(L, LUA_REGISTRYINDEX); // pops the sub-table
    lua_pop(L, 1);  // pops our table
    return cLuaTable{ mScript, reference };
}
