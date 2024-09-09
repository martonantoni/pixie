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

cLuaValue::~cLuaValue()
{
    if (mScript && mReference != LUA_NOREF)
    {
        luaL_unref(mScript->state(), LUA_REGISTRYINDEX, mReference);
    }
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

cLuaValue& cLuaValue::operator=(cLuaValue&& src)
{
    if (this == &src)
        return *this;
    cLuaValue toDiscard(std::move(*this));
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

cLuaValue::cStateWithSelfCleanup cLuaValue::retrieveSelf() const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return {};
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);
    return L;
}

void cLuaValue::retrieveItem(lua_State* L, const cKey& key)
{
    std::visit(
        [&](auto&& key)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(key)>, int>)
            {
                lua_rawgeti(L, -1, key);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(key)>, std::string_view>)
            {
                lua_getfield(L, -1, key.data());
            }
            else // cLuaValue:
            {
                key.retrieveSelf().release();
                lua_gettable(L, -2);
            }
        },
        key);
}

void cLuaValue::pushKey(lua_State* L, const cKey& key)
{
    std::visit(
        [&](auto&& key)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(key)>, int>)
            {
                lua_pushinteger(L, key);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(key)>, std::string_view>)
            {
                lua_pushstring(L, key.data());
            }  // cLuaValue:
            else
            {
                key.retrieveSelf().release();
            }
        },
        key);

}


bool cLuaValue::isNumber() const
{
    if(auto L = retrieveSelf())
    {
        bool result = lua_type(L, -1) == LUA_TNUMBER;
        return result;
    }
    return false;
}

bool cLuaValue::isString() const
{
    if(auto L = retrieveSelf())
    {
        bool result = lua_type(L, -1) == LUA_TSTRING;
        return result;
    }
    return false;
}

bool cLuaValue::isFunction() const
{
    if(auto L = retrieveSelf())
    {
        bool result = lua_isfunction(L, -1);
        return result;
    }
    return false;
}

bool cLuaValue::isTable() const
{
    if(auto L = retrieveSelf())
    {
        bool result = lua_istable(L, -1);
        return result;
    }
    return false;
}

void cLuaValue::copy_(const cLuaValue& src)
{
    mScript = src.mScript;
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, src.mReference);
    mReference = luaL_ref(L, LUA_REGISTRYINDEX);
}

cLuaValue cLuaValue::subTable(const std::string& key) const
{
    if (auto L = retrieveSelf())
    {
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
        return cLuaValue{ mScript, reference, false };
    }
    return {};
}

std::string cLuaScript::valueToString(lua_State* L, int index)
{
    if (lua_type(L, index) == LUA_TSTRING)
    {
        auto text = lua_tostring(L, index);
        return text;
    }
    lua_pushvalue(L, index);
    FINALLY([&]() { lua_pop(L, 1); });
    if(lua_isnumber(L, -1))
    {
        double number = lua_tonumber(L, -1);
        if (number == static_cast<int>(number))
        {
            return std::to_string(static_cast<int>(number));
        }
        return std::format("{:.5g}", number);
    }
    const char* convertedText = lua_tolstring(L, -1, nullptr);
    std::string convertedString = convertedText ? convertedText : std::string{};
    return convertedString;
}

std::shared_ptr<cConfig> cLuaValue::toConfig_topTable(lua_State* L, IsRecursive isRecursive) const
{
    if (!lua_istable(L, -1))
    {
        return {};
    }
    std::shared_ptr<cConfig> config = std::make_shared<cConfig>();

    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        // key: -2, value: -1
        std::variant<std::string, int> key;
        bool skip = false;
        if (lua_type(L, -2) == LUA_TNUMBER)
        {
            key = static_cast<int>(lua_tointeger(L, -2) - 1);
        }
        else
        {
            key = cLuaScript::valueToString(L, -2);
            skip = mIsGlobalTable && cLuaScript::isGlobalInternalElement(std::get<std::string>(key));
        }
        if (!skip)
        {
            switch (lua_type(L, -1))
            {
            case LUA_TBOOLEAN:
                std::visit([&](auto&& key) { config->set(key, static_cast<bool>(lua_toboolean(L, -1))); }, key);
                break;
            case LUA_TNUMBER:
                if (lua_isinteger(L, -1))
                {
                    std::visit([&](auto&& key) { config->set(key, static_cast<int>(lua_tointeger(L, -1))); }, key);
                }
                else
                {
                    std::visit([&](auto&& key) { config->set(key, lua_tonumber(L, -1)); }, key);
                }
                break;
            case LUA_TSTRING:
                std::visit([&](auto&& key) { config->set(key, lua_tostring(L, -1)); }, key);
                break;
            case LUA_TTABLE:
                if (isRecursive == IsRecursive::Yes)
                {
                    std::visit([&](auto&& key) { config->set(key, toConfig_topTable(L, isRecursive)); }, key);
                }
                break;
            }
        }
        lua_pop(L, 1); // pop the value, keep the key
    }
    return config;
}

std::shared_ptr<cConfig> cLuaValue::toConfig(IsRecursive isRecursive) const
{
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);
    auto config = toConfig_topTable(L, isRecursive);
    lua_pop(L, 1);
    return config;
}

int cLuaValue::toInt() const
{
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);

    auto result = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return result;
}

double cLuaValue::toDouble() const
{
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);

    auto result = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return result;
}

std::string cLuaValue::toString() const
{
    if (auto L = retrieveSelf())
    {
        auto result = cLuaScript::valueToString(L, -1);
        return result;
    }
    return {};
}

int cLuaValue::arraySize() const
{
    if (auto L = retrieveSelf())
    {
        auto result = lua_rawlen(L, -1);
        return result;
    }
    return 0;
}

bool cLuaValue::has(const cKey& key) const
{
    if (auto L = retrieveSelf())
    {
        retrieveItem(L, key);
        bool result = !lua_isnil(L, -1);
        lua_pop(L, 1);
        return result;
    }
    return false;
}

void cLuaValue::remove(const cKey& key)
{
    if (auto L = retrieveSelf())
    {
        if (lua_rawlen(L, -1) > 0)
        {
            // array, use table.remove
            lua_getglobal(L, "table");
            lua_getfield(L, -1, "remove");
            lua_pushvalue(L, -3); // table to remove from
            pushKey(L, key);
            if (lua_pcall(L, 2, 0, 0) != LUA_OK)
            {
                const char* error = lua_tostring(L, -1);
                lua_pop(L, 2); // global table + error message
                throw std::runtime_error(std::format("table.remove failed: {}", error));
            }
            lua_pop(L, 1); // global table
        }
        else
        {
            pushKey(L, key);
            lua_pushnil(L);
            lua_settable(L, -3);
        }
    }
}