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