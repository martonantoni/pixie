#include "../StdAfx.h"

tIntrusivePtr<cConfig> theMainConfig;

tIntrusivePtr<cConfig> cConfig::GetSubConfig(const std::string &Key) const
{	
	auto i=mSubConfigs.find(Key);
	if(i==mSubConfigs.end())
	{
		cStringVector Parts(Key, ".", false);
		if(Parts.size()>=2)
		{
			tIntrusivePtr<cConfig> PartConfig(const_cast<cConfig *>(this));  // temp hack
			for(auto &Part: Parts)
			{
				PartConfig=PartConfig->GetSubConfig(Part);
				if(!PartConfig->IsRealConfig())
					break;
			}
			return PartConfig;
		}
		return tIntrusivePtr<cConfig>(new cEmptyConfig);
	}
	return i->second;
}

tIntrusivePtr<cConfig> cConfig::CreateSubConfig(const std::string &Key)
{
	auto &Config=mSubConfigs[Key];
	if(!Config)
		Config=InternalCreateSubConfig(Key);
	return Config;
}

void cConfig::SetSubConfig(const std::string &Key, tIntrusivePtr<cConfig> SubConfig)
{
	mSubConfigs[Key]=SubConfig;
}

std::vector<std::string> cConfig::GetSubConfigNames() const
{
	std::vector<std::string> ConfigNames;
	for(auto i: mSubConfigs)
		ConfigNames.push_back(i.first);
	return ConfigNames;
}

tIntrusivePtr<cConfig> cConfig::FromFile(const cPath &Path)
{
// #todolua
//	if(Path.GetExtension()=="lua")
//		return cLuaBasedConfig::CreateConfig(Path);
	if(Path.GetExtension()=="cfg")
	{
		auto NewConfig=make_intrusive_ptr<cSimpleConfig>();
		NewConfig->FromFile(Path);
		return NewConfig;
	}
	ASSERT(false);
	return nullptr;
}