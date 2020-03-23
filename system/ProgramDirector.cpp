#include "StdAfx.h"

cProgramDirector *theProgramDirector=nullptr;

cProgramDirector::cProgramDirector()
{
	theProgramDirector=this;
}

cRegisteredID cProgramDirector::WaitForPhase(const cWaitRequest &WaitRequest)
{
	if(mCurrentPhase>size_t(WaitRequest.mPhase))
	{
		WaitRequest.mFunction();
		return cRegisteredID(nullptr, nullptr);
	}
	cPhaseData &Phase=mPhases[size_t(WaitRequest.mPhase)];
	auto i=std::find_if(Phase.mWaiters.begin(), Phase.mWaiters.end(), [&](const cWaitIDData *Data){ return Data->mRequest.mOrder>=WaitRequest.mOrder; });
	bool IsFirst=i==Phase.mWaiters.begin();
	if(IsFirst&&mCurrentPhase==size_t(WaitRequest.mPhase))
	{
		WaitRequest.mFunction();
		return cRegisteredID(nullptr, nullptr);
	}
	cWaitIDData *IDData=new cWaitIDData(WaitRequest);
	Phase.mWaiters.insert(i, IDData);
	return cRegisteredID(IDData, nullptr);
}

cRegisteredID cProgramDirector::LockPhase(eProgramPhases Phase)
{
	if(mCurrentPhase>=size_t(Phase))
	{
		ASSERT(false);
		return cRegisteredID(nullptr, nullptr);
	}
	++mPhases[size_t(Phase)].mLockCount;
	return cRegisteredID(new cLockIDData(Phase), nullptr);
}

void cProgramDirector::CurrentPhaseUnlocked()
{
	cPhaseData &Phase=mPhases[mCurrentPhase];

	while(!Phase.mWaiters.empty())
	{
		cWaitIDData *WaitData=Phase.mWaiters.pop_front();
		// WaitData will get deleted when the cRegisteredID::Unregister is called
		WaitData->mRequest.mFunction();
	}

	if(mCurrentPhase<size_t(eProgramPhases::NumberOfPhases)-1)
	{
		// not the last phase
		Unlock(eProgramPhases(mCurrentPhase+1));
	}
}

void cProgramDirector::Unlock(eProgramPhases Phase)
{
	if(!--mPhases[size_t(Phase)].mLockCount&&mCurrentPhase==size_t(Phase)-1)
	{
		++mCurrentPhase;
		::CallBack(theMainThread, eCallbackType::NoImmediate, this, &cProgramDirector::CurrentPhaseUnlocked);
	}
}

void cProgramDirector::Unregister(cWaitIDData &IDData)
{
	mPhases[int(IDData.mRequest.mPhase)].mWaiters.remove(&IDData);
}

void cProgramDirector::Unregister(cLockIDData &IDData)
{
	Unlock(IDData.mPhase);
}


void cProgramDirector::Start()
{
	ASSERT(mCurrentPhase==0);
	::CallBack(theMainThread, eCallbackType::NoImmediate, this, &cProgramDirector::CurrentPhaseUnlocked);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cProgramDirector::cLockIDData::Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType)
{
	theProgramDirector->Unlock(mPhase);
	delete this;
}

void cProgramDirector::cWaitIDData::Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType)
{
	theProgramDirector->Unregister(*this);
	delete this;
}
