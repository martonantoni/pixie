#include "StdAfx.h"

size_t cSimpleConfig::GetValueCount() const
{
	return mValues.size();
}

void cSimpleConfig::Set(const std::string &Key,int Value)
{
	mValues[Key].Set(Value);
}

void cSimpleConfig::Set(const std::string &Key,bool Value)
{
	mValues[Key].Set(int(Value));
}

void cSimpleConfig::Set(const std::string &Key,double Value)
{
	mValues[Key].Set(Value);
}

void cSimpleConfig::Set(const std::string &Key,const std::string &Value)
{
	mValues[Key].Set(Value);
}

void cSimpleConfig::Set(const std::string &Key,const char *Value)
{
	mValues[Key].Set(std::string(Value));
}

int cSimpleConfig::GetInt(const std::string &Key,const tDefaultValue<int> &Default) const
{
	cValues::const_iterator i=mValues.find(Key);
	return i==mValues.end()?Default.GetValue():i->second.GetIntValue();
}

double cSimpleConfig::GetDouble(const std::string &Key,const tDefaultValue<double> &Default) const
{
	cValues::const_iterator i=mValues.find(Key);
	return i==mValues.end()?Default.GetValue():i->second.GetDoubleValue();
}

std::string cSimpleConfig::GetString(const std::string &Key,const tDefaultValue<std::string> &Default) const
{
	cValues::const_iterator i=mValues.find(Key);
	return i==mValues.end()?Default.GetValue():i->second.GetStringValue();
}

bool cSimpleConfig::GetBool(const std::string &Key,const tDefaultValue<bool> &Default) const
{
	cValues::const_iterator i=mValues.find(Key);
	return i==mValues.end()?Default.GetValue():i->second.GetBoolValue();
}

tIntrusivePtr<cConfig> cSimpleConfig::InternalCreateSubConfig(const std::string &Key)
{
	return tIntrusivePtr<cConfig>(new cSimpleConfig);
}

void cSimpleConfig::ConfigToStream(const cConfig &Config, cMemoryStream &Stream)
{
	for(auto &Key: Config.GetKeys())
	{
		Stream.WriteString(Key);
		Stream.WriteString(Config.GetString(Key));
	}
	Stream.WriteString(std::string());
	for(auto &SubConfigName: Config.GetSubConfigNames())
	{
		Stream.WriteString(SubConfigName);
		ConfigToStream(*Config.GetSubConfig(SubConfigName), Stream);
	}
	Stream.WriteString(std::string());
}

void cSimpleConfig::FromStream(const cMemoryStream &Stream)
{
	mValues.clear();
	for(;;)
	{
		std::string Key=Stream.ReadString();
		if(Key.empty())
			break;
		mValues[Key].Set(Stream.ReadString());
	}
	for(;;)
	{
		std::string Key=Stream.ReadString();
		if(Key.empty())
			break;
		auto SubConfig=make_intrusive_ptr<cSimpleConfig>();
		SubConfig->FromStream(Stream);
		SetSubConfig(Key, std::move(SubConfig));
	}
}

template<class T>
bool cSimpleConfig::Build(T LineReader)
{
	std::vector<std::pair<std::string, tIntrusivePtr<cSimpleConfig>>> mSubGroupStack;
	bool ExpectingGroup=false;
	const auto IsWS=[](const char c) { return c=='\t'||c==' '; };
	for(unsigned int LineCounter=0;; ++LineCounter)
	{
		cFastFileReader::cLine Line(LineReader());
		if(!Line.IsValid())
			break;
		auto LineStartPos=std::find_if_not(Line, IsWS);
		if(LineStartPos==cend(Line))
			continue; // empty line
		if(*LineStartPos=='#'||*LineStartPos==';')
			continue; // comment
		if(*LineStartPos=='{')
		{
			if(ASSERTTRUE(ExpectingGroup)) // invalid line, check LineCounter and FileName
			{
				ExpectingGroup=false;
				continue;
			}
			mSubGroupStack.pop_back();
		}
		else if(*LineStartPos=='}')
		{
			if(ASSERTTRUE(!mSubGroupStack.empty())) // invalid line, check LineCounter and FileName
			{
				auto NewSubConfigInitizalizer=mSubGroupStack.back();
				mSubGroupStack.pop_back();
				(mSubGroupStack.empty()?this:mSubGroupStack.back().second.get())->
					SetSubConfig(NewSubConfigInitizalizer.first, NewSubConfigInitizalizer.second);
			}
			continue;
		}
		auto equPos=std::find(LineStartPos, cend(Line), '=');
		if(equPos==cend(Line))
		{
			ASSERT(false); // invalid line, check LineCounter and FileName
			continue; // ignore
		}
		std::string Key(LineStartPos, equPos-LineStartPos);
		TrimRight(Key);
		auto FirstValueChar=std::find_if_not(equPos+1, cend(Line), IsWS);
		if(FirstValueChar==cend(Line))
		{
			mSubGroupStack.emplace_back(Key, make_intrusive_ptr<cSimpleConfig>());
			ExpectingGroup=true;
			continue;
		}
		if(*FirstValueChar=='"')
		{
			++FirstValueChar;
			auto LastValueChar=std::find(FirstValueChar, cend(Line), '"');
			if(LastValueChar==cend(Line))
			{
				ASSERT(false); // invalid line, check LineCounter and FileName
				continue; // ignore
			}
			(mSubGroupStack.empty()?this:mSubGroupStack.back().second.get())->
				mValues[Key]=cValue(std::string(FirstValueChar, LastValueChar-FirstValueChar));
		}
		else
		{
			auto LastValueChar=std::find_if(FirstValueChar, cend(Line), IsWS);
			(mSubGroupStack.empty()?this:mSubGroupStack.back().second.get())->
				mValues[Key]=cValue(std::string(FirstValueChar, LastValueChar-FirstValueChar));
		}
	}
	return true;
}

bool cSimpleConfig::FromString(const std::string &ConfigText)
{
	cStringVector MainParts(ConfigText, ",", false);
	return Build([i=MainParts.begin(), iend=MainParts.end()]() mutable
	{
		if(i==iend)
		{
			return cFastFileReader::cLine();
		}
		cFastFileReader::cLine Line(i->c_str(), (int)i->size());
		++i;
		return Line;
	});
}

bool cSimpleConfig::FromFile(const cPath &FileName)
{
	if(!DoesFileExist(FileName))
		return false;
	cFastFileReader File(FileName);
	return Build([&File]() { return File.GetNextLine(); });
}

void cSimpleConfig::ConfigToFile(const cConfig &Config, const cPath &FileName)
{
	ConfigToFile(Config, cLog(FileName, cLog::SINGLE_THREAD|cLog::TRUNCATE));
}

void cSimpleConfig::ConfigToFile(const cConfig &Config, cLog &Log)
{
	ConfigToFile(Config, Log, {});
}

void cSimpleConfig::ConfigToFile(const cConfig &Config, cLog &Log, const std::string &Tabs)
{
	for(auto &Key: Config.GetKeys())
	{
		Log.Log("%s%s = \"%s\"", Tabs.c_str(), Key.c_str(), Config.GetString(Key).c_str());
	}
	for(auto &i: Config.GetSubConfigNames())
	{
		Log.Log("%s%s =", Tabs.c_str(), i.c_str());
		Log.Log("%s{", Tabs.c_str());
		ConfigToFile(*Config.GetSubConfig(i), Log, Tabs+"\t");
		Log.Log("%s}", Tabs.c_str());
	}
}

std::vector<std::string> cSimpleConfig::GetKeys() const
{
	std::vector<std::string> Keys;
	Keys.reserve(mValues.size());
	for(auto &i: mValues)
		Keys.emplace_back(i.first);
	return Keys;
}
