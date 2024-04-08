#include "StdAfx.h"

cCSVConfig::cCSVConfig(const cPath &Path, const std::string &Separator)
{
	cFastFileReader File(Path);
	auto Line=File.GetNextLine();
	if(!Line.IsValid())
		return;
	mKeys.FromString(Line, Separator, false);
	for(;;)
	{
		auto Line=File.GetNextLine();
		if(!Line.IsValid())
			return;
		if(Line.Length>0)
			mRows.emplace_back(Line, Separator, true);
	}
}

size_t cCSVConfig::GetKeyIndex(const std::string &Key) const
{
	auto i=std::ranges::find(mKeys, Key);
	return i==mKeys.end()?~0:i-mKeys.begin();
}

bool cCSVConfig::IsFieldEmpty(size_t RowIndex, size_t KeyIndex) const
{
	return GetString(RowIndex, KeyIndex).empty();
}

int cCSVConfig::GetInt(size_t RowIndex, size_t KeyIndex) const
{
	return atoi(GetString(RowIndex, KeyIndex).c_str());
}

double cCSVConfig::GetDouble(size_t RowIndex, size_t KeyIndex) const
{
	return atof(GetString(RowIndex, KeyIndex).c_str());
}

std::string cCSVConfig::GetString(size_t RowIndex, size_t KeyIndex) const
{
	auto &Row=mRows[RowIndex];
	if(ASSERTFALSE(Row.size()<=KeyIndex))
		return std::string(); 
	return mRows[RowIndex][KeyIndex];
}

bool cCSVConfig::GetBool(size_t RowIndex, size_t KeyIndex) const
{
	auto StringValue=GetString(RowIndex, KeyIndex);
	if(StringValue=="true")
		return true;
	ASSERT(StringValue=="false");
	return false;
}

