#pragma once

// #include "../../LuaPlus/LuaPlus.h"
// 
// using namespace LuaPlus;
// 
// extern LuaState *theLuaState;
// 
// namespace std
// {
// 	template<> class default_delete<LuaState>
// 	{
// 	public:
// 		void operator()(LuaState *Object) const
// 		{
// 			LuaState::Destroy(Object);
// 		}
// 	};
// }
// 
// namespace LPCD
// {
// 	inline void Push(lua_State* L, const std::string &value) { lua_pushstring(L, value.c_str()); }
// }
// 
// #include "InitLua.h"
// #include "LuaObjectSynch.h"
// 
// 
