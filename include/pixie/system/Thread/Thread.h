#pragma once

class cThread: public cRegistrationHandler
{
protected:
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
	void AddEventHandler_Inner(tIntrusivePtr<cIDData> IDData, cNativeEvent *Event);
	void AddTimer_Inner(tIntrusivePtr<cIDData> IDData, const cTimerRequest &TimerRequest);
	using cInvokerFunctionPtr = void(*)(char*);
	void processEventDispatch();
	virtual void threadLoop();
public:
	cThread(const std::string &Name,std::unique_ptr<cReactor> Reactor=std::make_unique<cReactor>());
	~cThread();
	template<class T> void callback(T callable, eCallbackType callbackType = eCallbackType::Normal);
	void Start();
	BOOL IsInThread() const;
	char *LockCallbackBuffer(size_t RequiredSize);
	void ReleaseCallbackBuffer(eCallbackType CallbackType);
	static cThread *GetCurrentThread();
	virtual void Unregister(const cRegisteredID &RegisteredID,eCallbackType CallbackType) override;

	cRegisteredID AddEventHandler(const cReactor::cEventListener &EventListener,cNativeEvent *Event,cRegisteredID *ID=nullptr);
	cRegisteredID AddTimer(const cTimerServer::cTimerListener &TimerListener,const cTimerRequest &TimerRequest,cRegisteredID *ID=nullptr);
};

template<class T> void cThread::callback(T callable, eCallbackType callbackType)
{
    if (IsInThread() && callbackType != eCallbackType::NoImmediate)
    {
		callable();
        return;
    }
	auto buffer = LockCallbackBuffer(sizeof(T) + sizeof(cInvokerFunctionPtr));
	new(buffer) cInvokerFunctionPtr(
		[](char* callablePtr)
		{
			(*(T*)(callablePtr))();
		});
	new(buffer + sizeof(cInvokerFunctionPtr)) T(std::move(callable));
	ReleaseCallbackBuffer(callbackType);
}
