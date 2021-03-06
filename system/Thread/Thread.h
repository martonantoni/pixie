#pragma once

class cThread: public cRegistrationHandler
{
	struct cIDData: public cIntrusiveThreadsafeRefCount
	{
		enum eIDType { Type_EventHandler, Type_Timer, Type_Invalid } mType=Type_Invalid;
		cReactor::cEventListener mEventListener;
		cTimerServer::cTimerListener mTimerListener;
		size_t mID;
	};
	std::string mName;
	std::unique_ptr<cReactor> mReactor;
	cTimerServer mTimerServer;
	HANDLE mThreadHandle;
	DWORD mThreadID;
	std::unique_ptr<std::vector<std::vector<char>>> mCallbackBuffer_Reading=std::make_unique<std::vector<std::vector<char>>>();
	std::unique_ptr<std::vector<std::vector<char>>> mCallbackBuffer_Writing=std::make_unique<std::vector<std::vector<char>>>();
	cMutex mCallbackMutex;
	cNativeEvent mCallbacksDoneEvent    = { cNativeEvent::AutoReset };
	cNativeEvent mCallbackRequestedEvent= { cNativeEvent::AutoReset };
	cRegisteredID mProcessCallbacksID;
	static cThreadLocalStorage mThreadLocalStorage;
	void ThreadFunction();
	void ProcessCallbacks();
	void Unregister_Inner(tIntrusivePtr<cIDData> IDData);
	static DWORD WINAPI StaticThreadFunction(LPVOID lpParameter);
	void SetThreadNameForDebugger();
	void cThread::AddEventHandler_Inner(tIntrusivePtr<cIDData> IDData, cNativeEvent *Event);
	void cThread::AddTimer_Inner(tIntrusivePtr<cIDData> IDData, const cTimerRequest &TimerRequest);
public:
	cThread(const std::string &Name,std::unique_ptr<cReactor> Reactor=std::make_unique<cReactor>());
	~cThread();
	void Start();
	BOOL IsInThread() const;
	char *LockCallbackBuffer(size_t RequiredSize);
	void ReleaseCallbackBuffer(eCallbackType CallbackType);
	static cThread *GetCurrentThread();
	virtual void Unregister(const cRegisteredID &RegisteredID,eCallbackType CallbackType) override;

	cRegisteredID AddEventHandler(const cReactor::cEventListener &EventListener,cNativeEvent *Event,cRegisteredID *ID=nullptr);
	cRegisteredID AddTimer(const cTimerServer::cTimerListener &TimerListener,const cTimerRequest &TimerRequest,cRegisteredID *ID=nullptr);
};

extern cThread *theMainThread;