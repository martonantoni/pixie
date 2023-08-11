#pragma once

#include "pixie/system/log/i_log.h"

class cTextLogToEvent: public cTextLogPlugin
{
	tIntrusivePtr<cEventDispatcher> mDispatcher;
	tDataHolder<std::string> &mTextHolder=tDataHolder<std::string>::Get();
	void PostMessage(const std::string &Text);
public:
	cTextLogToEvent(tIntrusivePtr<cEventDispatcher> Dispatcher): mDispatcher(std::move(Dispatcher)) {}
	virtual void Log(const std::string &Text) override;
};
