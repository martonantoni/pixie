#include "StdAfx.h"


#include "Reactor_MessagePump.h"

cThreadServer::cThreadServer()
{
	theThreadServer=this;
}

cThreadServer::~cThreadServer()
{
	theThreadServer=NULL;
}

cThread *cThreadServer::GetThread(const std::string &Name,BOOL UseMessageQueueReactor)
{
	mMutex.Lock();
	auto &Thread=mThreadMap[Name];
	if(!Thread)
	{
		Thread=std::make_unique<cThread>(Name,UseMessageQueueReactor?std::make_unique<cReactor_MessagePump>():std::make_unique<cReactor>());
		mMutex.Release();
		Thread->Start();
	}
	else
	{
		mMutex.Release();
	}
	return Thread.get();
}

void cThreadServer::createMainThread()
{
	ASSERT(!theMainThread);
    auto thread = std::make_unique<cMainThread>("main"s, std::make_unique<cReactor_MessagePump>());
	theMainThread = thread.get();
	thread->Start();
	mThreadMap["main"s] = std::move(thread);
}

cThreadServer *theThreadServer=NULL;