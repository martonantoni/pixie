#pragma once

#include "Log.h"

class cMainLog: public cLog
{
public:
	cMainLog();
	explicit cMainLog(int flags);
};

extern cMainLog *MainLog;