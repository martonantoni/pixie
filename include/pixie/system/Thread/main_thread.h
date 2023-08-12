#pragma once


class cMainThread: public cThread
{
public:
	cMainThread(const std::string& Name, std::unique_ptr<cReactor> Reactor) :
		cThread(Name, std::move(Reactor)) {}
protected:
	virtual void threadLoop() override;
private:
    cNativeEvent mGotEventToDispatch = { cNativeEvent::AutoReset };
	cRegisteredID mGotEventToDispatchID;
	DWORD mTimeToNextTimer;
	void processEventDispatch();
};

extern cMainThread *theMainThread;