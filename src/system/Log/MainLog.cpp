#include "StdAfx.h"

cMainLog *MainLog=NULL;

cMainLog::cMainLog(int flags): 
    cLog(ProgramArguments?
        ProgramArguments->GetString("MainLogFile",fmt::sprintf("%s-%s.log",ProgramName,TodayString)):
        fmt::sprintf("%s-%s.log",ProgramName,TodayString)
        ,flags)
{
}

cMainLog::cMainLog(): 
	cLog(ProgramArguments?
		ProgramArguments->GetString("MainLogFile",fmt::sprintf("%s-%s.log",ProgramName,TodayString)):
		fmt::sprintf("%s-%s.log",ProgramName,TodayString)
		,USE_MUTEX|TIME_STAMP|ECHO|FLUSH_OVER_TIME)
{
}