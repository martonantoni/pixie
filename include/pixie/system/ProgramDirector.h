#pragma once

enum class eProgramPhases
{
	NotStarted,    // DO NOT USE
	StaticInit,
	Startup,
	Running,

	NumberOfPhases
};

class cProgramDirector: public tSingleton<cProgramDirector>
{
public:
	struct cWaitRequest
	{
		eProgramPhases mPhase;
		int mOrder;
		std::function<void()> mFunction;
		cWaitRequest() {}
		cWaitRequest(eProgramPhases Phase, int Order, const std::function<void()> &Function):
			mPhase(Phase), mOrder(Order), mFunction(Function) {}
	};
private:
	struct cLockIDData: public cRegistrationHandler
	{
		eProgramPhases mPhase;
		virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType=eCallbackType::Wait) override;
		cLockIDData(eProgramPhases Phase): mPhase(Phase) {}
	};
	struct cWaitIDData: public tIntrusiveListItemBase<cWaitIDData>, public cRegistrationHandler
	{
		cWaitRequest mRequest;
		virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType=eCallbackType::Wait) override;
		cWaitIDData(const cWaitRequest &Request): mRequest(Request) {}
	};
	typedef tIntrusiveList<cWaitIDData> cPhaseWaiters;
	struct cPhaseData
	{
		int mLockCount=1;
		cPhaseWaiters mWaiters;
	};
	cPhaseData mPhases[eProgramPhases::NumberOfPhases];
	size_t mCurrentPhase=0;
	void CurrentPhaseUnlocked(); // when this is called mCurrentPhase is already set
	void Unlock(eProgramPhases Phase);
	void Unregister(cWaitIDData &IDData);
	void Unregister(cLockIDData &IDData);
public:
	cProgramDirector();
	cRegisteredID WaitForPhase(const cWaitRequest &WaitRequest);  // does not lock!
	cRegisteredID LockPhase(eProgramPhases Phase);
	
	void Start();
};

template<class Label> struct tProgramDirectorLabel;

#define DEFAULT_INIT_LABEL(Label) \
	struct Label_##Label; \
	template<> struct tProgramDirectorLabel<Label_##Label> { static const int mOrder=100; };

DEFAULT_INIT_LABEL(DefaultInitLabel);

extern cProgramDirector *theProgramDirector;

#define REGISTER_AUTO_INIT_FUNCTION_CALL(Function,Phase,Label) \
	struct RegistratorFor_##Function \
	{  \
		cRegisteredID mID; \
		RegistratorFor_##Function():  \
			mID(cProgramDirector::Get().WaitForPhase(cProgramDirector::cWaitRequest(Phase, tProgramDirectorLabel<Label_##Label>::mOrder, \
			[this](){mID.Unregister(); Function();}))) \
		{} \
	} Instance_RegistratorFor_##Function; 

#define REGISTER_AUTO_SINGLETON_INIT(SingletonClass,Phase,Label) \
	void SingletonGetCaller_##SingletonClass() { SingletonClass::Get(); } \
	REGISTER_AUTO_INIT_FUNCTION_CALL(SingletonGetCaller_##SingletonClass,Phase,Label);

