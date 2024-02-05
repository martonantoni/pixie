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

std::unique_ptr<cReactor> cThreadServer::createReactor(eReactorType reactorType)
{
    if (reactorType == eReactorType::MessagePump)
    {
        return std::make_unique<cReactor_MessagePump>();
    }
    else
    {
        return std::make_unique<cReactor>();
    }
}

cThread *cThreadServer::GetThread(const std::string &name, eReactorType reactorType)
{
	mThreadMapMutex.lock();
	auto &thread=mThreadMap[name];
	if(!thread)
	{
		thread=std::make_unique<cThread>(name, createReactor(reactorType));
		mThreadMapMutex.unlock();
		thread->Start();
	}
	else
	{
		mThreadMapMutex.unlock();
	}
	return thread.get();
}

void cThreadServer::createMainThread(eReactorType reactorType)
{
	ASSERT(!theMainThread);
    auto thread = std::make_unique<cMainThread>("main"s, createReactor(reactorType));
	theMainThread = thread.get();
	thread->Start();
	mThreadMap["main"s] = std::move(thread);
}

cThreadServer *theThreadServer=NULL;