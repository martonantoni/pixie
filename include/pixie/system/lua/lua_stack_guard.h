#pragma once

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
