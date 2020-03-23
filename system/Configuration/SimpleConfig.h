#pragma once

class cSimpleConfig: public cConfig
{
	typedef cSimpleValue cValue;
	typedef std::unordered_map<std::string,cValue> cValues;
	cValues mValues;
	static void ConfigToFile(const cConfig &Config, cLog &Log, const std::string &Tabs);
	template<class T> bool Build(T LineReader);
protected:
	virtual tIntrusivePtr<cConfig> InternalCreateSubConfig(const std::string &Key) override;
public:
	cSimpleConfig() {}
	template<typename V,typename... T> cSimpleConfig(const std::string &Key, const V &Value, T &&...Objects): cSimpleConfig(Objects...) { Set(Key, Value);	}
	virtual void Set(const std::string &Key,int Value) override;
	virtual void Set(const std::string &Key,bool Value) override;
	virtual void Set(const std::string &Key,double Value) override;
	virtual void Set(const std::string &Key,const std::string &Value) override;
	virtual void Set(const std::string &Key,const char *Value) override;
	virtual int GetInt(const std::string &Key,const tDefaultValue<int> &Default=tDefaultValue<int>()) const override;
	virtual double GetDouble(const std::string &Key,const tDefaultValue<double> &Default=tDefaultValue<double>()) const override;
	virtual std::string GetString(const std::string &Key,const tDefaultValue<std::string> &Default=tDefaultValue<std::string>()) const override;
	virtual bool GetBool(const std::string &Key,const tDefaultValue<bool> &Default=tDefaultValue<bool>()) const override;
	virtual std::vector<std::string> GetKeys() const override;
	virtual size_t GetValueCount() const override;

	bool FromString(const std::string &ConfigText);
	bool FromFile(const cPath &FileName);
	static void ConfigToFile(const cConfig &Config, const cPath &FileName);
	static void ConfigToFile(const cConfig &Config, cLog &Log);
	void FromStream(const cMemoryStream	&Stream);
	static void ConfigToStream(const cConfig &Config, cMemoryStream &Stream);
};
