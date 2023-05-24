
LuaState *theLuaState=nullptr;

void cLuaFunctionRegistrator::RegisterFunction(void(*Function)())
{
	mFunctions.push_back(Function);
}

void cLuaFunctionRegistrator::CallRegisteredFunctions()
{
	for(auto Function: mFunctions)
		Function();
	mFunctions= {};
}

LuaObject cLuaStateKeeper::DoString(const std::string &Script)
{
	return LuaHelpers::DoString(mLuaState, Script);
}

void InitLua()
{
	theLuaState=LuaState::Create(true);
	cLuaFunctionRegistrator::Get().CallRegisteredFunctions();
}

bool LoadLUAFile(const char *FileName, LuaState *State)
{
	MainLog->Log("Processing file: %s",FileName);
	if(State->DoFile(FileName))
	{
		std::string LuaError(LuaStackObject(State, -1).GetString());
		MainLog->Log("%s",LuaError.c_str());
		RELEASE_ASSERT_EXT(false, LuaError);
		return false;
	}	
	return true;
}

void LoadLUAFromBuffer(const std::vector<uint8_t> &Data, LuaState *State)
{
	if(State->DoBuffer((char *)Data.data(), Data.size(), "N/A"))
	{
		std::string LuaError(LuaStackObject(State, -1).GetString());
		MainLog->Log("%s", LuaError.c_str());
		RELEASE_ASSERT_EXT(false,LuaError);
	}
}

namespace LuaHelpers
{
	std::string ToString(LuaObject &Object)
	{
		if(!Object.IsConvertibleToString())
			return std::string();
		const char *cs=Object.ToString();
		return ASSERTTRUE(cs)?std::string(cs):std::string();
	}

	LuaObject DoString(LuaState *State, const std::string &Script)
	{
		if(ASSERTFALSE(!State))
			return {};
		int t=State->GetTop();
		State->DoString(Script.c_str());
		int diff=State->GetTop()-t;
		if(diff==0)
			return {};
		auto ReturnedObject=State->StackTop();
	//	State->SetTop(t);
		return ReturnedObject;
	}

}