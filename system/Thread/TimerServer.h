#pragma once

class cTimerServer
{
public:
	typedef std::function<void()> cTimerListener;
private:
	struct cTimerData: public tIntrusiveListItemBase<cTimerData>
	{
		cTimerRequest mRequest;
		cTimerListener mListener;
		DWORD mNextTrigger;
		size_t mID;
		cTimerData() {}
	};
	typedef tIntrusiveList<cTimerData> cTimers;
	cTimers mTimers;
	cTimerData *mCurrentTimer;
	size_t mIDCounter=0;
	void InsertTimer(cTimerData *TimerData);
public:
	cTimerServer();
	~cTimerServer();
	size_t AddTimer(const cTimerListener &Listener,const cTimerRequest &Request);
	void RemoveTimer(size_t ID);
	DWORD GetTimeToNextTimer(DWORD CurrentTime) const;
	void ProcessTimers(DWORD CurrentTime);
};