#include "StdAfx.h"


cLuaObject::cLuaObject(std::shared_ptr<cLuaState> state, int reference, bool isGlobalTable)
    : mState(std::move(state))
    , mReference(reference)
    , mIsGlobalTable(isGlobalTable)
{
}

cLuaObject::cLuaObject(cLuaObject&& src)
    : mState(std::move(src.mState))
    , mReference(src.mReference)
{
    src.mReference = LUA_NOREF;
}

cLuaObject::~cLuaObject()
{
    if (mState && mReference != LUA_NOREF)
    {
        luaL_unref(mState->state(), LUA_REGISTRYINDEX, mReference);
    }
}

cLuaObject::cLuaObject(const cLuaObject& src)
{
    if (!src.mState || src.mReference == LUA_NOREF)
    {
        return;
    }
    copy_(src);
}

cLuaObject::cStateWithSelfCleanup cLuaObject::retrieveSelf() const
{
    if (!mState || mReference == LUA_NOREF)
    {
        return {};
    }
    lua_State* L = mState->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference);
    return L;
}

void cLuaObject::retrieveItem(lua_State* L, const cKey& key)
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
            else // cLuaObject:
            {
                key.retrieveSelf().release();
                lua_gettable(L, -2);
            }
        },
        key);
}

void cLuaObject::pushKey(lua_State* L, const cKey& key)
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
            }  // cLuaObject:
            else
            {
                key.retrieveSelf().release();
            }
        },
        key);
}


bool cLuaObject::isNumber() const
{
    if(auto L = retrieveSelf())
    {
        bool result = lua_type(L, -1) == LUA_TNUMBER;
        return result;
    }
    return false;
}

bool cLuaObject::isString() const
{
    if(auto L = retrieveSelf())
    {
        bool result = lua_type(L, -1) == LUA_TSTRING;
        return result;
    }
    return false;
}

bool cLuaObject::isNil() const
{
    if(auto L = retrieveSelf())
    {
        bool result = lua_isnil(L, -1);
        return result;
    }
    return true;
}

bool cLuaObject::isFunction() const
{
    if(auto L = retrieveSelf())
    {
        bool result = lua_isfunction(L, -1);
        return result;
    }
    return false;
}

bool cLuaObject::isTable() const
{
    if(auto L = retrieveSelf())
    {
        bool result = lua_istable(L, -1);
        return result;
    }
    return false;
}

void cLuaObject::copy_(const cLuaObject& src)
{
    mState = src.mState;
    lua_State* L = mState->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, src.mReference);
    mReference = luaL_ref(L, LUA_REGISTRYINDEX);
}

cLuaObject cLuaObject::subTable(const std::string& key) const
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
        return cLuaObject{ mState, reference, false };
    }
    return {};
}

std::string cLuaState::valueToString(lua_State* L, int index)
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

std::shared_ptr<cConfig> cLuaObject::toConfig_topTable(lua_State* L, IsRecursive isRecursive) const
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
            key = cLuaState::valueToString(L, -2);
            skip = mIsGlobalTable && cLuaState::isGlobalInternalElement(std::get<std::string>(key));
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

std::shared_ptr<cConfig> cLuaObject::toConfig(IsRecursive isRecursive) const
{
    if (auto L = retrieveSelf())
    {
        return toConfig_topTable(L, isRecursive);
    }
    return {};
}

int cLuaObject::toInt() const
{
    if (auto L = retrieveSelf())
    {
        return lua_tointeger(L, -1);
    }
    return 0;
}

double cLuaObject::toDouble() const
{
    if (auto L = retrieveSelf())
    {
        return lua_tonumber(L, -1);
    }
    return 0.0;
}

std::string cLuaObject::toString() const
{
    if (auto L = retrieveSelf())
    {
        return cLuaState::valueToString(L, -1);
    }
    return {};
}

int cLuaObject::arraySize() const
{
    if (auto L = retrieveSelf())
    {
        return lua_rawlen(L, -1);
    }
    return 0;
}

bool cLuaObject::has(const cKey& key) const
{
    if (auto L = retrieveSelf())
    {
        retrieveItem(L, key);
        bool result = !lua_isnil(L, -1);
        lua_pop(L, 1); // pop the item
        return result;
    }
    return false;
}

void cLuaObject::remove(const cKey& key)
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

int cLuaObject::luaErrorFunction(lua_State* L)
{
    // Error object is at index 1
    const char* msg = lua_tostring(L, 1);
    if (!msg)
    {
        // Try __tostring metamethod if not a string
        if (luaL_callmeta(L, 1, "__tostring") && lua_isstring(L, -1))
        {
            msg = lua_tostring(L, -1);
            lua_remove(L, 1);  // remove original error object
        }
        else
        {
            msg = "non-string error object";
        }
    }

    // Push traceback with that message
    luaL_traceback(L, L, msg, 1);
    return 1; // return the traceback string}
}