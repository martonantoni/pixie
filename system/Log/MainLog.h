#pragma once

#include "Log.h"

class cMainLog: public cLog
{
public:
	cMainLog();
};

extern cMainLog *MainLog;