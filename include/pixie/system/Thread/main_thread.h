#pragma once

#include "pixie/System/EventSystem/i_EventSystem.h"

class cMainThread: public cThread
{
public:
	cMainThread(const std::string& Name, std::unique_ptr<cReactor> Reactor) :
		cThread(Name, std::move(Reactor)) {}
protected:
	virtual void threadLoop() override;
};

extern cMainThread *theMainThread;