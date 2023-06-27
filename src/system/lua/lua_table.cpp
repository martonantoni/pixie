#include "StdAfx.h"


cLuaTable::cLuaTable(std::shared_ptr<cLuaScript> script, int reference, bool isGlobalTable)
    : mScript(std::move(script))
    , mReference(reference)
    , mIsGlobalTable(isGlobalTable)
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
    return cLuaTable{ mScript, reference, false };
}

std::string cLuaScript::valueToString(lua_State* L, int index)
{
    auto text = lua_tostring(L, index);
    if (text)
    {
        return text;
    }
    lua_pushvalue(L, index);
    const char* convertedText = lua_tolstring(L, -1, nullptr);
    std::string convertedString = convertedText ? convertedText : std::string{};
    lua_pop(L, 1);
    return convertedString;
}

tIntrusivePtr<cConfig> cLuaTable::toConfig_topTable(lua_State* L, IsRecursive isRecursive) const
{
    if (!lua_istable(L, -1))
    {
        return {};
    }
    tIntrusivePtr<cConfig> config = make_intrusive_ptr<cSimpleConfig>();

    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        // key: -2, value: -1
        std::string key = cLuaScript::valueToString(L, -2);
        if (!mIsGlobalTable || !cLuaScript::isGlobalInternalElement(key))
        {
            switch (lua_type(L, -1))
            {
            case LUA_TBOOLEAN:
                config->Set(key, static_cast<bool>(lua_toboolean(L, -1)));
                break;
            case LUA_TNUMBER:
                if (lua_isinteger(L, -1))
                {
                    config->Set(key, static_cast<int>(lua_tointeger(L, -1)));
                }
                else
                {
                    config->Set(key, lua_tonumber(L, -1));
                }
                break;
            case LUA_TSTRING:
                config->Set(key, lua_tostring(L, -1));
                break;
            case LUA_TTABLE:
                if (isRecursive == IsRecursive::Yes)
                {
                    config->SetSubConfig(key, toConfig_topTable(L, isRecursive));
                }
                break;
            }
        }
        lua_pop(L, 1); // pop the value, keep the key
    }
    return config;
}

tIntrusivePtr<cConfig> cLuaTable::toConfig(IsRecursive isRecursive) const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return {};
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);
    return toConfig_topTable(L, isRecursive);
}