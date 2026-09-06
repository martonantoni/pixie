#include "StdAfx.h"

cProgramTitle *cProgramTitle::Get()
{
	static cProgramTitle *Instance=NULL;
	return Instance?Instance:Instance=new cProgramTitle;
}

cProgramTitle::cProgramTitle()
{
	(std::string &)*this=std::format("{} v{}, PID: {}",ProgramName,VersionString,GetCurrentProcessId());
}

void cProgramTitle::SetInstanceName(const std::string &InstanceName)
{
	(std::string &)*this=std::format("{}({}) v{}, PID: {}",ProgramName,InstanceName,VersionString,GetCurrentProcessId());
}