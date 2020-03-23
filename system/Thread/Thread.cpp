
cThreadLocalStorage cThread::mThreadLocalStorage;

cThread::cThread(const std::string &Name, std::unique_ptr<cReactor> Reactor)
: mName(Name)
, mReactor(std::move(Reactor))
{
}

cThread::~cThread()
{
	mProcessCallbacksID.Unregister();
}

void cThread::ThreadFunction()
{
	SetThreadNameForDebugger();
	mThreadLocalStorage.SetValue(this);
	mProcessCallbacksID=AddEventHandler([this]() { ProcessCallbacks(); }, &mCallbackRequestedEvent);
	DWORD CurrentTime=GetTickCount();
	for(;;)
	{
		DWORD TimeToNextTimer=mTimerServer.GetTimeToNextTimer(CurrentTime);
		mReactor->DispatchEvents(TimeToNextTimer);
		CurrentTime=GetTickCount();
		mTimerServer.ProcessTimers(CurrentTime);
	}
}

DWORD WINAPI cThread::StaticThreadFunction(LPVOID lpParameter)
{
	((cThread *)lpParameter)->ThreadFunction();
	return 0;
}

void cThread::ProcessCallbacks()
{
	{
		cMutexGuard Guard(mCallbackMutex);
		std::swap(mCallbackBuffer_Reading,mCallbackBuffer_Writing);
	}
	if(mCallbackBuffer_Reading->empty())
		return;
	for(auto &Calls: *mCallbackBuffer_Reading)
	{
		auto Pos=Calls.data();
		ThreadCallbackHelper::CallbackHelperFunction Function=*(ThreadCallbackHelper::CallbackHelperFunction *)Pos;
		Pos+=sizeof(ThreadCallbackHelper::CallbackHelperFunction);
		(*Function)(Pos);
	}
	mCallbackBuffer_Reading->resize(0);
}

void cThread::Start()
{
	mThreadHandle=::CreateThread(NULL,0,&cThread::StaticThreadFunction,this,CREATE_SUSPENDED,&mThreadID);
	::ResumeThread(mThreadHandle); // so that the mThreadHandle is valid
}

BOOL cThread::IsInThread() const
{
	return ::GetCurrentThreadId()==mThreadID;
}

char *cThread::LockCallbackBuffer(size_t RequiredSize)
{
	ASSERT(RequiredSize);
	mCallbackMutex.Lock();
	mCallbackBuffer_Writing->resize(mCallbackBuffer_Writing->size()+1);
	mCallbackBuffer_Writing->back().resize(RequiredSize);
	return mCallbackBuffer_Writing->back().data();
}

void cThread::ReleaseCallbackBuffer(eCallbackType CallbackType)
{
	ASSERT(!mCallbackBuffer_Writing->empty());
	mCallbackMutex.Release();
	if(CallbackType==eCallbackType::Wait)
	{
		cThread *CurrentThread=GetCurrentThread();
		cNativeEvent *CallbacksDoneEvent=CurrentThread?&CurrentThread->mCallbacksDoneEvent:new cNativeEvent(cNativeEvent::AutoReset);
		::CallBack(this,eCallbackType::Normal,CallbacksDoneEvent,&cNativeEvent::Set);
		CallbacksDoneEvent->Wait(INFINITE);
		if(!CurrentThread)
			delete CallbacksDoneEvent;
		return;
	}
	mCallbackRequestedEvent.Set();
}

cThread *cThread::GetCurrentThread()
{
	return (cThread *)mThreadLocalStorage.GetValue();
}

void cThread::AddEventHandler_Inner(tIntrusivePtr<cIDData> IDData, cNativeEvent *Event)
{
	ASSERT(IsInThread());
	if(IDData->mType==cIDData::Type_Invalid)
		return; // got Unregistered during the callback
	if(ASSERTFALSE(IDData->mType!=cIDData::Type_EventHandler))
		return;
	IDData->mID=mReactor->AddEventListener(IDData->mEventListener, Event);
}

cRegisteredID cThread::AddEventHandler(const cReactor::cEventListener &EventListener, cNativeEvent *Event, cRegisteredID *ID)
{
	cIDData *IDData=new cIDData;
	IDData->mType=cIDData::Type_EventHandler;
	IDData->mEventListener=EventListener;
	IDData->Ref(); // for storing in the cRegisteredID
	cRegisteredID RegisteredID(this,IDData);
	if(ID)
		*ID=std::move(RegisteredID);
	::CallBack(this, eCallbackType::Normal, this, &cThread::AddEventHandler_Inner, tIntrusivePtr<cIDData>(IDData), Event);
	return RegisteredID; // if ID was not null, this will return an empty cRegisteredID
}

void cThread::AddTimer_Inner(tIntrusivePtr<cIDData> IDData, const cTimerRequest &TimerRequest)
{
	ASSERT(IsInThread());
	if(IDData->mType==cIDData::Type_Invalid)
		return; // got Unregistered during the callback
	if(ASSERTFALSE(IDData->mType!=cIDData::Type_Timer))
		return;
	IDData->mID=mTimerServer.AddTimer(IDData->mTimerListener, TimerRequest);
}

cRegisteredID cThread::AddTimer(const cTimerServer::cTimerListener &TimerListener,const cTimerRequest &TimerRequest,cRegisteredID *ID)
{
	cIDData *IDData=new cIDData;
	IDData->mType=cIDData::Type_Timer;
	IDData->mTimerListener=TimerListener;
	IDData->Ref(); // for storing in the cRegisteredID
	cRegisteredID RegisteredID(this,IDData);
	if(ID)
		*ID=std::move(RegisteredID);
	::CallBack(this, eCallbackType::Normal, this, &cThread::AddTimer_Inner, tIntrusivePtr<cIDData>(IDData), TimerRequest);
	return RegisteredID; // if ID was not null, this will return an empty cRegisteredID
}

void cThread::Unregister(const cRegisteredID &RegisteredID,eCallbackType CallbackType)
{
	cIDData *IDData=(cIDData *)RegisteredID.GetIDData();
	if(ASSERTFALSE(!IDData))
		return;
	if(!IsInThread())
	{
		::CallBack(this,CallbackType,this,&cThread::Unregister_Inner,tIntrusivePtr<cIDData>::CreateWithoutAddingRef(IDData));
		return;
	}
	Unregister_Inner(tIntrusivePtr<cIDData>::CreateWithoutAddingRef(IDData));
}

void cThread::Unregister_Inner(tIntrusivePtr<cIDData> IDData)
{
	switch(IDData->mType)
	{
	case cIDData::Type_EventHandler:
		mReactor->RemoveEventListener(IDData->mID);
		break;
	case cIDData::Type_Timer:
		mTimerServer.RemoveTimer(IDData->mID);
		break;
	case cIDData::Type_Invalid:
		break;
	default:
		ASSERT(false);
	}
}

#define MS_VC_EXCEPTION 0x406D1388

void cThread::SetThreadNameForDebugger()
{
	struct THREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	};

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = mName.c_str();
	info.dwThreadID = ~0;
	info.dwFlags = 0;

	__try
	{
		RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info );
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

#undef MS_VC_EXCEPTION

cThread *theMainThread=NULL;