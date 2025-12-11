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
    , mIsGlobalTable(src.mIsGlobalTable)
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
    : mIsGlobalTable(src.mIsGlobalTable)
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

cLuaObject::iterator::iterator(const cLuaObject& table)
    : mTable(table)
    , mEnd(false)
{
    if (!mTable.isTable())
    {
        mEnd = true;
        return;
    }
    {
        if (auto L = mTable.retrieveSelf())
        {
            lua_pushnil(L); // first key
            mKey = cLuaObject{ mTable.mState, luaL_ref(L, LUA_REGISTRYINDEX), false }; // pops the nil
        }
        else
            return;
    }
    next();
}

bool cLuaObject::iterator::operator==(const iterator& other) const
{
    return mEnd && other.mEnd; // cannot compare mTable, mKey, mValue easily, so just compare end state
}

void cLuaObject::iterator::next()
{
    if (mEnd)
    {
        return;
    }
    if (auto L = mTable.retrieveSelf())
    {
        do
        {
            mKey.retrieveSelf().release(); // pushes the key onto the stack
            if (lua_next(L, -2) != 0)  // table at -2, key at -1
            {
                mValue = cLuaObject{ mTable.mState, luaL_ref(L, LUA_REGISTRYINDEX), false }; // pops the value
                mKey = cLuaObject{ mTable.mState, luaL_ref(L, LUA_REGISTRYINDEX), false }; // pops the key
            }
            else
            {
                mEnd = true;
            }
        }
        while (!mEnd && mTable.mIsGlobalTable && cLuaState::isGlobalInternalElement(mKey.toString()));
    }
    else
    {
        mEnd = true;
    }
}

cLuaObject::iterator& cLuaObject::iterator::operator++()
{
    next();
    return *this;
}

cLuaObject::iterator cLuaObject::iterator::operator++(int)
{ 
    auto tmp = *this; 
    ++(*this); 
    return tmp; 
}

cLuaObject::iterator cLuaObject::begin() const
{
    return iterator{ *this };
}

cLuaObject::iterator cLuaObject::end() const
{
    return iterator{};
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