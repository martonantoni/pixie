#include "StdAfx.h"

cMainLog *MainLog=NULL;

cMainLog::cMainLog(int flags) :
    cLog(ProgramArguments ?
        ProgramArguments->GetString("MainLogFile", std::format("{}-{}.log", ProgramName, TodayString.c_str())) :
        std::format("{}-{}.log", ProgramName, TodayString.c_str())
        , flags)
{
}

cMainLog::cMainLog() :
    cLog(ProgramArguments ?
        ProgramArguments->GetString("MainLogFile", std::format("{}-{}.log", ProgramName, TodayString.c_str())) :
        std::format("{}-{}.log", ProgramName, TodayString.c_str())
        , cLog::Flags::USE_MUTEX | cLog::Flags::TIME_STAMP | cLog::Flags::ECHO | cLog::Flags::FLUSH_OVER_TIME)
{
}