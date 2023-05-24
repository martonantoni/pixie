#pragma once

class cLuaStateKeeper: public cIntrusiveRefCount
{
	LuaState *mLuaState=nullptr;
	virtual void ReferenceCounterReachedZero() const override
	{
		LuaState::Destroy(mLuaState);
	}
public:
	cLuaStateKeeper(): mLuaState(LuaState::Create(true)) {}
	LuaState *GetState() const { return mLuaState; }
	LuaObject DoString(const std::string &Script);
};

class cLuaFunctionRegistrator: public tSingleton<cLuaFunctionRegistrator>
{
	std::vector<void(*)()> mFunctions;
public:
	void RegisterFunction(void(*Function)());
	void CallRegisteredFunctions();
};

#define REGISTER_LUA_FUNCTION(Function, LuaName) \
struct cRegistratorFor_##LuaName \
{ \
	static void RegisterFunction() { theLuaState->GetGlobals().RegisterDirect(#LuaName,Function); } \
	cRegistratorFor_##LuaName() \
	{ \
		cLuaFunctionRegistrator::Get().RegisterFunction(&cRegistratorFor_##LuaName::RegisterFunction); \
	} \
} InstanceOf_RegistratorFor_##LuaName

void InitLua();

bool LoadLUAFile(const char *FileName,LuaState *State=theLuaState);
void LoadLUAFromBuffer(const std::vector<uint8_t> &Data, LuaState *State);

namespace LuaHelpers
{
	std::string ToString(LuaObject &Object); // cannot be const &, because LuaObject's ToString is not const func.
	LuaObject DoString(LuaState *State, const std::string &Script);
}