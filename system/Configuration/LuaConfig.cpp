
cLuaBasedConfig::cLuaBasedConfig(const LuaObject &Object)
	: mObject(Object)
	, mState(Object.GetState())
{
	Init();
}

cLuaBasedConfig::cLuaBasedConfig(LuaState *State, bool IsOwned)
	: mObject(State->GetGlobals())
	, mState(State, IsOwned)
{
	Init();
}

void cLuaBasedConfig::Init()
{
	ASSERT(mObject.IsTable());
	for(LuaTableIterator i(mObject);i;i.Next())
	{
		LuaObject SubObject=i.GetValue();
		if(SubObject.IsTable())
		{
			LuaObject KeyObject(i.GetKey());
			if(KeyObject.IsString())
			{
				std::string TableName(KeyObject.GetString());
				if(TableName!="package"&&TableName[0]!='_')
					SetSubConfig(TableName, tIntrusivePtr<cConfig>(new cLuaBasedConfig(SubObject)));
			}
		}
	}
}

cLuaBasedConfig::~cLuaBasedConfig()
{
}

std::vector<std::string> cLuaBasedConfig::GetKeys() const
{
	std::vector<std::string> Keys;
	for(LuaTableIterator i(mObject); i; i.Next())
	{
		LuaObject SubObject=i.GetValue();
		if(SubObject.IsBoolean()|SubObject.IsConvertibleToInteger()||SubObject.IsConvertibleToString())
		{
			LuaObject KeyObject(i.GetKey());
			if(ASSERTTRUE(KeyObject.IsConvertibleToString()))
			{
				Keys.push_back(KeyObject.ToString());
			}
		}
	}
	return Keys;
}

bool cLuaBasedConfig::GetOrCreateSubTable(INOUT LuaObject &Object, const char *TableName, LuaState *State)
{
	LuaObject SubObject=Object[TableName];
	if(SubObject.IsTable())
	{
		Object=SubObject;
		return true;
	}
	if(SubObject.IsNil())
	{
		SubObject.AssignNewTable(State);
		Object.SetObject(TableName,SubObject);
		Object=SubObject;
		return true;
	}
	return false;
}

tIntrusivePtr<cConfig> cLuaBasedConfig::CreateConfig(const char *MainTableKey, LuaState *State)
{
	char NameBuffer[50];
	for(LuaObject CurrentObject=State->GetGlobals();;)
	{
		const char *SeparatorPos=strchr(MainTableKey,'.');
		if(!SeparatorPos)
		{
			if(!GetOrCreateSubTable(CurrentObject, MainTableKey, State))
			{
				ASSERT(false);
				return tIntrusivePtr<cConfig>(new cEmptyConfig);
			}
			return tIntrusivePtr<cConfig>(new cLuaBasedConfig(CurrentObject));
		}
		if(ASSERTFALSE(SeparatorPos-MainTableKey>=50))
		{
			return tIntrusivePtr<cConfig>(new cEmptyConfig);
		}
		memcpy(NameBuffer,MainTableKey,SeparatorPos-MainTableKey);
		NameBuffer[SeparatorPos-MainTableKey]=0;
		MainTableKey=SeparatorPos+1;

		if(ASSERTFALSE(!GetOrCreateSubTable(CurrentObject, NameBuffer, State)))
		{
			return make_intrusive_ptr<cEmptyConfig>();
		}
	}
}

tIntrusivePtr<cConfig> cLuaBasedConfig::CreateConfig(LuaState *State)
{
	return make_intrusive_ptr<cLuaBasedConfig>(State->GetGlobals());
}

tIntrusivePtr<cConfig> cLuaBasedConfig::GetSubConfig(int Index) const
{
	LuaObject SubObject=mObject[Index+1];
	if(ASSERTFALSE(!SubObject.IsTable()))
		return make_intrusive_ptr<cEmptyConfig>();
	return make_intrusive_ptr<cLuaBasedConfig>(SubObject);
}

size_t cLuaBasedConfig::GetValueCount() const
{
	return mObject.GetTableCount();
}

size_t cLuaBasedConfig::GetSubConfigCount() const
{
	return mObject.GetTableCount();
}

void cLuaBasedConfig::Set(const std::string &Key,int Value)
{
	mObject.SetInteger(Key.c_str(),Value);
}

void cLuaBasedConfig::Set(const std::string &Key,bool Value)
{
	mObject.SetBoolean(Key.c_str(),Value);
}

void cLuaBasedConfig::Set(const std::string &Key,double Value)
{
	mObject.SetNumber(Key.c_str(),Value);
}

void cLuaBasedConfig::Set(const std::string &Key,const std::string &Value)
{
	mObject.SetString(Key.c_str(),Value.c_str(),(int)Value.length());
}

void cLuaBasedConfig::Set(const std::string &Key,const char *Value)
{
	mObject.SetString(Key.c_str(),Value);
}

int cLuaBasedConfig::GetInt(const std::string &Key,const tDefaultValue<int> &Default) const
{
	LuaObject SubObject=mObject[Key.c_str()];
	if(SubObject.IsConvertibleToInteger())
		return SubObject.ToInteger();
	return Default.GetValue();
}

int cLuaBasedConfig::GetInt(int Index, const tDefaultValue<int> &Default) const
{
	LuaObject SubObject=mObject[Index+1]; // in Lua, indeces start with 1.
	if(SubObject.IsConvertibleToInteger())
		return SubObject.ToInteger();
	return Default.GetValue();
}

double cLuaBasedConfig::GetDouble(const std::string &Key,const tDefaultValue<double> &Default) const
{
	LuaObject SubObject=mObject[Key.c_str()];
	if(SubObject.IsConvertibleToNumber())
		return SubObject.GetNumber();
	return Default.GetValue();
}

double cLuaBasedConfig::GetDouble(int Index, const tDefaultValue<double> &Default) const
{
	LuaObject SubObject=mObject[Index+1];
	if(SubObject.IsConvertibleToNumber())
		return SubObject.GetNumber();
	return Default.GetValue();
}

std::string cLuaBasedConfig::GetString(const std::string &Key,const tDefaultValue<std::string> &Default) const
{
	LuaObject SubObject=mObject[Key.c_str()];
	if(SubObject.IsConvertibleToString())
		return SubObject.GetString();
	return Default.GetValue();
}

std::string cLuaBasedConfig::GetString(int Index, const tDefaultValue<std::string> &Default) const
{
	LuaObject SubObject=mObject[Index+1];
	if(SubObject.IsConvertibleToString())
		return SubObject.GetString();
	return Default.GetValue();
}

bool cLuaBasedConfig::GetBool(const std::string &Key,const tDefaultValue<bool> &Default) const
{
	LuaObject SubObject=mObject[Key.c_str()];
	if(SubObject.IsBoolean())
		return SubObject.GetBoolean();
	if(SubObject.IsInteger())
		return !!SubObject.GetInteger();
	return Default.GetValue();
}

bool cLuaBasedConfig::GetBool(int Index, const tDefaultValue<bool> &Default) const
{
	LuaObject SubObject=mObject[Index+1];
	if(SubObject.IsBoolean())
		return SubObject.GetBoolean();
	if(SubObject.IsInteger())
		return !!SubObject.GetInteger();
	return Default.GetValue();
}

tIntrusivePtr<cConfig> cLuaBasedConfig::InternalCreateSubConfig(const std::string &Key)
{
	LuaObject SubObject = mObject[Key.c_str()];
	if (SubObject.IsTable())
		return tIntrusivePtr<cConfig>(new cLuaBasedConfig(SubObject));
	if (SubObject.IsNil())
	{
		SubObject = mObject.CreateTable(Key.c_str());
		return tIntrusivePtr<cConfig>(new cLuaBasedConfig(SubObject));
	}
	ASSERT(false);
	return tIntrusivePtr<cConfig>(new cEmptyConfig);
}

tIntrusivePtr<cConfig> cLuaBasedConfig::CreateConfig(const cPath &Path)
{
	auto State=LuaState::Create(false);
	LoadLUAFile(Path.c_str(), State);
	return make_intrusive_ptr<cLuaBasedConfig>(State, true);
}

tIntrusivePtr<cConfig> cLuaBasedConfig::CreateConfig(LuaObject Object)
{
	return make_intrusive_ptr<cLuaBasedConfig>(Object);
}
