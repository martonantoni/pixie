#include "StdAfx.h"

cProgramTitle *cProgramTitle::Get()
{
	static cProgramTitle *Instance=NULL;
	return Instance?Instance:Instance=new cProgramTitle;
}

cProgramTitle::cProgramTitle()
{
	(std::string &)*this=fmt::sprintf("%s v%s, PID: %d",ProgramName,VersionString,GetCurrentProcessId());
}

void cProgramTitle::SetInstanceName(const std::string &InstanceName)
{
	(std::string &)*this=fmt::sprintf("%s(%s) v%s, PID: %d",ProgramName,InstanceName,VersionString,GetCurrentProcessId());
}