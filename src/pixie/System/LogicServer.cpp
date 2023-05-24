#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
unsigned int gFrameTime=GetTickCount();
cLogicServer theLogicServer;
unsigned int cLogicServer::cLogicUserData::mIDCounter=0;

cLogicServer::cLogicServer()
{
}

cLogicServer::~cLogicServer()
{
}

cRegisteredID cLogicServer::AddLogic(const cLogicListener &Listener,int Order)
{
	auto i=std::lower_bound(mLogicUsers, Order, [](const auto &LogicUser, int Order) { return LogicUser.mOrder<Order; });
	mLogicUsers.emplace(i,Listener,Order);
	mLogicUsersModifiedTrap=true;
	return cRegisteredID(this,cLogicUserData::mIDCounter);
}

void cLogicServer::Tick()
{
	gFrameTime=GetTickCount();
	do
	{
		mLogicUsersModifiedTrap=false;
		for(auto &LogicUser: mLogicUsers)
		{
			if(LogicUser.mLastSeenFrameTime!=gFrameTime)
			{
				LogicUser.mLastSeenFrameTime=gFrameTime;
				LogicUser.mListener();
			}
			if(mLogicUsersModifiedTrap)
				break;
		}
	} while(mLogicUsersModifiedTrap);
	for(;;)
	{
		std::swap(mExecuteOnceWriting, mExecuteOnceReading);
		if(mExecuteOnceReading.empty())
			break;
		for(auto f: mExecuteOnceReading)
			f();
		mExecuteOnceReading.clear();
	}
}

void cLogicServer::Unregister(const cRegisteredID &RegisteredID,eCallbackType CallbackType)
{
	auto i=std::find_if(mLogicUsers, [ID=RegisteredID.GetID()](const auto &LogicUser){ return LogicUser.mID==ID; });
	if(ASSERTTRUE(i!=mLogicUsers.end()))
	{
		mLogicUsers.erase(i);
		mLogicUsersModifiedTrap=true;
	}
}
