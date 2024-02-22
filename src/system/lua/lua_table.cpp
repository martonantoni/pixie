#include "StdAfx.h"


cLuaValue::cLuaValue(std::shared_ptr<cLuaScript> script, int reference, bool isGlobalTable)
    : mScript(std::move(script))
    , mReference(reference)
    , mIsGlobalTable(isGlobalTable)
{
}

cLuaValue::cLuaValue(cLuaValue&& src)
    : mScript(std::move(src.mScript))
    , mReference(src.mReference)
{
    src.mReference = LUA_NOREF;
}

cLuaValue& cLuaValue::operator=(cLuaValue&& src)
{
    if (this == &src)
        return *this;
    mReference = src.mReference;
    src.mReference = LUA_NOREF;
    mScript = std::move(src.mScript);
    return *this;
}

cLuaValue::cLuaValue(const cLuaValue& src)
{
    if (!src.mScript || src.mReference == LUA_NOREF)
    {
        return;
    }
    copy_(src);
}

void cLuaValue::retriveWithKey(lua_State* L, cKey key) // assumes the table is on top of the stack
{
    std::visit(
        overloaded
        {
            [L](std::reference_wrapper<const std::string> key) 
            { 
                lua_pushstring(L, key.get().c_str()); 
                lua_gettable(L, -2);
            },
            [L](const char* key)
            {
                lua_pushstring(L, key); 
                lua_gettable(L, -2);
            },
            [L](int index) 
            {
                lua_rawgeti(L, -1, index);
            } 
        }, 
        key);
}

void cLuaValue::copy_(const cLuaValue& src)
{
    mScript = src.mScript;
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, src.mReference);
    mReference = luaL_ref(L, LUA_REGISTRYINDEX);
}

cLuaValue& cLuaValue::operator=(const cLuaValue& src)
{
    if (&src == this)
    {
        return *this;
    }
    cLuaValue toDiscard(std::move(*this));
    if (!src.mScript || src.mReference == LUA_NOREF)
    {
        return *this;
    }
    copy_(src);
    return *this;
}

bool cLuaValue::isFunction() const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return false;
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    bool result = lua_isfunction(L, -1);
    lua_pop(L, 1); // Pop the table from the stack
    return result;
}

cLuaValue::~cLuaValue()
{
    if (mScript && mReference != LUA_NOREF)
    {
        luaL_unref(mScript->state(), LUA_REGISTRYINDEX, mReference);
    }
}

cLuaValue cLuaValue::subTable(const std::string& key) const
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
    return cLuaValue{ mScript, reference, false };
}

std::string cLuaScript::valueToString(lua_State* L, int index)
{
    if (lua_type(L, index) == LUA_TSTRING)
    {
        auto text = lua_tostring(L, index);
        return text;
    }
    lua_pushvalue(L, index);
    const char* convertedText = lua_tolstring(L, -1, nullptr);
    std::string convertedString = convertedText ? convertedText : std::string{};
    lua_pop(L, 1);
    return convertedString;
}

tIntrusivePtr<cConfig> cLuaValue::toConfig_topTable(lua_State* L, IsRecursive isRecursive) const
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
        std::string key;
        if (lua_type(L, -2) == LUA_TNUMBER)
        {
            lua_pushvalue(L, -2);
            key = std::to_string(lua_tointeger(L, -1) - 1);
            lua_pop(L, 1);
        }
        else
        {
            key = cLuaScript::valueToString(L, -2);
        }
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

tIntrusivePtr<cConfig> cLuaValue::toConfig(IsRecursive isRecursive) const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return {};
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);
    return toConfig_topTable(L, isRecursive);
}

int cLuaValue::toInt() const
{
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);

    auto result = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return result;
}

std::string cLuaValue::toString() const
{
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);

    auto result = cLuaScript::valueToString(L, -1);
    lua_pop(L, 1);
    return result;
}
