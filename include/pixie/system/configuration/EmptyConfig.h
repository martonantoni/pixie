#pragma once

class cEmptyConfig: public cConfig
{
protected:
	virtual tIntrusivePtr<cConfig> InternalCreateSubConfig(const std::string &Key) override;
public:
	virtual void Set(const std::string &Key,int Value) override;
	virtual void Set(const std::string &Key,bool Value) override;
	virtual void Set(const std::string &Key,double Value) override;
	virtual void Set(const std::string &Key,const std::string &Value) override;
	virtual void Set(const std::string &Key,const char *Value) override;
	virtual int GetInt(const std::string &Key,const tDefaultValue<int> &Default=tDefaultValue<int>()) const override;
	virtual double GetDouble(const std::string &Key,const tDefaultValue<double> &Default=tDefaultValue<double>()) const override;
	virtual std::string GetString(const std::string &Key,const tDefaultValue<std::string> &Default=tDefaultValue<std::string>()) const override;
	virtual bool GetBool(const std::string &Key,const tDefaultValue<bool> &Default=tDefaultValue<bool>()) const override;
	virtual size_t GetValueCount() const override;
// 	virtual void SetSubConfig(const std::string &Key,tIntrusivePtr<cConfig> SubConfig) override;
//	virtual std::unique_ptr<cSubConfigMap> GetSubConfigMap() override;
	virtual std::vector<std::string> GetKeys() const override { return std::vector<std::string>(); }
	virtual bool IsRealConfig() const { return false; }
	static tIntrusivePtr<cConfig> theEmptyConfig();
};
