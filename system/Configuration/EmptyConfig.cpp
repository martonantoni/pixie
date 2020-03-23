
size_t cEmptyConfig::GetValueCount() const
{
	return 0;
}

void cEmptyConfig::Set(const std::string &Key,int Value)
{
	ASSERT(false);
}

void cEmptyConfig::Set(const std::string &Key,bool Value)
{
	ASSERT(false);
}

void cEmptyConfig::Set(const std::string &Key,double Value)
{
	ASSERT(false);
}

void cEmptyConfig::Set(const std::string &Key,const std::string &Value)
{
	ASSERT(false);
}

void cEmptyConfig::Set(const std::string &Key,const char *Value)
{
	ASSERT(false);
}

int cEmptyConfig::GetInt(const std::string &Key,const tDefaultValue<int> &Default) const
{
	return Default.GetValue();
}

double cEmptyConfig::GetDouble(const std::string &Key,const tDefaultValue<double> &Default) const
{
	return Default.GetValue();
}

std::string cEmptyConfig::GetString(const std::string &Key,const tDefaultValue<std::string> &Default) const
{
	return Default.GetValue();
}

bool cEmptyConfig::GetBool(const std::string &Key,const tDefaultValue<bool> &Default) const
{
	return Default.GetValue();
}

tIntrusivePtr<cConfig> cEmptyConfig::InternalCreateSubConfig(const std::string &Key) 
{
	return tIntrusivePtr<cConfig>(new cEmptyConfig);
}

tIntrusivePtr<cConfig> cEmptyConfig::GetEmptyConfig()
{
	static tIntrusivePtr<cConfig> GlobalEmptyConfig(new cEmptyConfig);
	return GlobalEmptyConfig;
}
