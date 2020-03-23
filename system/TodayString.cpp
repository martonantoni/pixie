#include "StdAfx.h"

#include <time.h>

cTodayString TodayString;

cTodayString::cTodayString()
{
	time_t CurrentTime=time(0);
	tm LocalTime;
	localtime_s(&LocalTime,&CurrentTime);
	std::string::operator=(fmt::sprintf("%02d-%02d-%02d",LocalTime.tm_mon+1,LocalTime.tm_mday,LocalTime.tm_year%100));
}