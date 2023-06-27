#pragma once

class cConfig: public cIntrusiveRefCount
{
	typedef std::unordered_map<std::string, tIntrusivePtr<cConfig>> cSubConfigs;
	cSubConfigs mSubConfigs;
protected:
	virtual tIntrusivePtr<cConfig> InternalCreateSubConfig(const std::string &Key)=0;
public:
	cConfig()=default;
	cConfig(const cConfig &)=delete;
	virtual ~cConfig() {}
	virtual void Set(const std::string &Key, int Value)=0;
	virtual void Set(const std::string &Key, bool Value)=0;
	virtual void Set(const std::string &Key, double Value)=0;
	virtual void Set(const std::string &Key, const char *Value)=0;
	virtual void Set(const std::string &Key, const std::string &Value)=0;
	virtual int GetInt(const std::string &Key, const tDefaultValue<int> &Default=tDefaultValue<int>()) const=0;
	virtual int GetInt(int Index, const tDefaultValue<int> &Default=tDefaultValue<int>()) const { return GetInt(std::to_string(Index), Default); }
	virtual double GetDouble(const std::string &Key, const tDefaultValue<double> &Default=tDefaultValue<double>()) const=0;
	virtual double GetDouble(int Index, const tDefaultValue<double> &Default=tDefaultValue<double>()) const { return GetDouble(std::to_string(Index), Default); }
	virtual std::string GetString(const std::string &Key, const tDefaultValue<std::string> &Default=tDefaultValue<std::string>()) const=0;
	virtual std::string GetString(int Index, const tDefaultValue<std::string> &Default=tDefaultValue<std::string>()) const { return GetString(std::to_string(Index), Default); }
	virtual bool GetBool(const std::string &Key, const tDefaultValue<bool> &Default=tDefaultValue<bool>()) const=0;
	virtual bool GetBool(int Index, const tDefaultValue<bool> &Default=tDefaultValue<bool>()) const { return GetBool(std::to_string(Index), Default); }
	virtual bool IsRealConfig() const { return true; } // usefull for a few optimalizations, currently only cEmptyConfig is not real config.
	template<class T> T Get(const std::string &Key, const tDefaultValue<T> &Default) const;
	template<class T> T Get(int Index, const tDefaultValue<T> &Default) const;

	template<class T> T get(const std::string& keyPath, const tDefaultValue<T>& Default = tDefaultValue<T>()) const;

	tIntrusivePtr<cConfig> GetSubConfig(const std::string &Key) const;
	virtual tIntrusivePtr<cConfig> GetSubConfig(int Index) const { return GetSubConfig(std::to_string(Index)); }
	tIntrusivePtr<cConfig> CreateSubConfig(const std::string &Key);  // does not create if already exists
	virtual std::vector<std::string> GetSubConfigNames() const;
	virtual size_t GetSubConfigCount() const { return mSubConfigs.size(); }
	virtual std::vector<std::string> GetKeys() const=0;
	virtual size_t GetValueCount() const=0;
	void SetSubConfig(const std::string &Key, tIntrusivePtr<cConfig> SubConfig);

	static tIntrusivePtr<cConfig> FromFile(const cPath &Path); // checks file extension for type of config
};

template<> inline int cConfig::Get<int>(const std::string &Key, const tDefaultValue<int> &Default) const
{
	return GetInt(Key, Default);
}
template<> inline std::string cConfig::Get<std::string>(const std::string &Key, const tDefaultValue<std::string> &Default) const
{
	return GetString(Key, Default);
}
template<> inline double cConfig::Get<double>(const std::string &Key, const tDefaultValue<double> &Default) const
{
	return GetDouble(Key, Default);
}
template<> inline bool cConfig::Get<bool>(const std::string &Key, const tDefaultValue<bool> &Default) const
{
	return GetBool(Key, Default);
}

template<> inline int cConfig::Get<int>(int Index, const tDefaultValue<int> &Default) const
{
	return GetInt(Index, Default);
}
template<> inline std::string cConfig::Get<std::string>(int Index, const tDefaultValue<std::string> &Default) const
{
	return GetString(Index, Default);
}
template<> inline double cConfig::Get<double>(int Index, const tDefaultValue<double> &Default) const
{
	return GetDouble(Index, Default);
}
template<> inline bool cConfig::Get<bool>(int Index, const tDefaultValue<bool> &Default) const
{
	return GetBool(Index, Default);
}

template<class T> inline T cConfig::get(const std::string& keyPath, const tDefaultValue<T>& defaultValue) const
{
	cStringVector parts(keyPath, "."s, false);
	if (parts.empty())
		return defaultValue.GetValue();
	auto config = tIntrusivePtr<cConfig>(const_cast<cConfig*>(this)); // https://github.com/martonantoni/pixie/issues/2
	for (size_t i = 0; i < parts.size() - 1; ++i)
	{
		auto j = mSubConfigs.find(parts[i]);
		if (j == mSubConfigs.end())
		{
			return defaultValue.GetValue();
		}
		config = j->second;
	}
	if constexpr (std::is_same_v<T, int>)
	{
		return GetInt(parts.back(), defaultValue);
	}
	else if constexpr (std::is_same_v<T, double>)
	{
        return GetDouble(parts.back(), defaultValue);
	}
    else if constexpr (std::is_same_v<T, bool>)
    {
        return GetBool(parts.back(), defaultValue);
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        return GetString(parts.back(), defaultValue);
    }
}


extern tIntrusivePtr<cConfig> theMainConfig;