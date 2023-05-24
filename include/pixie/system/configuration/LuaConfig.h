#pragma once

#ifdef DFSDFDSF

class cLuaBasedConfig: public cConfig
{
	class cLuaStateHolder
	{
		LuaState *mState=nullptr;
		bool mIsOwned=false;
	public:
		LuaState *operator->() const { return mState; }
		LuaState *get() const { return mState; }
		cLuaStateHolder(LuaState *State, bool IsOwned=false): mState(State), mIsOwned(IsOwned) {}
		~cLuaStateHolder() { if(mIsOwned) LuaState::Destroy(mState); }
	} mState;           // -+
	LuaObject mObject;  // -+-- order is very important between these two. We want the StateHolder's destructor to run after the LuaObject's destructor

	static bool GetOrCreateSubTable(INOUT LuaObject &Object, const char *TableName, LuaState *State);
	virtual tIntrusivePtr<cConfig> InternalCreateSubConfig(const std::string &Key) override;
	void Init();
public:
	cLuaBasedConfig(const LuaObject &Object);
	cLuaBasedConfig(LuaState *State, bool IsStateOwned);
	~cLuaBasedConfig();
	virtual void Set(const std::string &Key,int Value) override;
	virtual void Set(const std::string &Key,bool Value) override;
	virtual void Set(const std::string &Key,double Value) override;
	virtual void Set(const std::string &Key,const std::string &Value) override;
	virtual void Set(const std::string &Key,const char *Value) override;
	virtual int GetInt(const std::string &Key,const tDefaultValue<int> &Default=tDefaultValue<int>()) const override;
	virtual int GetInt(int Index, const tDefaultValue<int> &Default=tDefaultValue<int>()) const override;
	virtual double GetDouble(const std::string &Key,const tDefaultValue<double> &Default=tDefaultValue<double>()) const override;
	virtual double GetDouble(int Index, const tDefaultValue<double> &Default=tDefaultValue<double>()) const override;
	virtual std::string GetString(const std::string &Key,const tDefaultValue<std::string> &Default=tDefaultValue<std::string>()) const override;
	virtual std::string GetString(int Index, const tDefaultValue<std::string> &Default=tDefaultValue<std::string>()) const override;
	virtual bool GetBool(const std::string &Key,const tDefaultValue<bool> &Default=tDefaultValue<bool>()) const override;
	virtual bool GetBool(int Index, const tDefaultValue<bool> &Default=tDefaultValue<bool>()) const override;
	virtual std::vector<std::string> GetKeys() const override;
	virtual size_t GetValueCount() const override;

//	virtual std::unique_ptr<cSubConfigMap> GetSubConfigMap() override;

	virtual size_t GetSubConfigCount() const override;
	virtual tIntrusivePtr<cConfig> GetSubConfig(int Index) const override;

	static tIntrusivePtr<cConfig> CreateConfig(const char *MainTableKey, LuaState *State);
	static tIntrusivePtr<cConfig> CreateConfig(LuaState *State);
	static tIntrusivePtr<cConfig> CreateConfig(const cPath &Path);
	static tIntrusivePtr<cConfig> CreateConfig(LuaObject Object);
};


#endif