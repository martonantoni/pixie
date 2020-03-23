#include "StdAfx.h"

#include "ProgramArguments.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

cProgramArguments *ProgramArguments=NULL;

cProgramArguments::cProgramArguments(int ArgNo,char **Args)
{
	for(int i=1;i<ArgNo;++i)
	{
		cStringVector ArgTokens(Args[i],"=",false);
		if(ArgTokens.size()!=2)
		{
			::MessageBox(NULL,fmt::sprintf("Invalid argument: %s",Args[i]).c_str(),ProgramName,MB_ICONSTOP|MB_OK);
			::exit(0);
		}
		(*this)[ArgTokens[0]]=ArgTokens[1];
	}
}

std::string cProgramArguments::GetString(const std::string &Key,const std::string &DefaultValue) const
{
	const_iterator i=find(Key);
	return i==end()?DefaultValue:i->second;
}
