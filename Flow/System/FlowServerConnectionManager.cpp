#include "StdAfx.h"

#include "LoginMessages.h"

std::shared_ptr<cFlowServerConnection> cFlowServerConnectionManager::GetConnection(const cFlowLoginMessage &LoginMessage)
{
	cMutexGuard Guard(mMutex);

	auto i=mConnections.find(LoginMessage.mConnectionID);
	if(i==mConnections.end())
		return nullptr;
	auto Connection=i->second.lock();
	if(!Connection)
	{
		mConnections.erase(i); // mind you, ConnectionDied should have removed it already.
		return nullptr;
	}
	if(ASSERTFALSE(Connection->GetMessageSuiteID()!=LoginMessage.mMessageSuiteID))
		return nullptr;
	return Connection;
}

void cFlowServerConnectionManager::ConnectionDied(cFlowConnectionID ConnectionID)
{
	cMutexGuard Guard(mMutex);

	mConnections.erase(ConnectionID);
}

std::shared_ptr<cFlowServerConnection> cFlowServerConnectionManager::CreateConnection(const cFlowLoginMessage &LoginMessage)
{
	cMutexGuard Guard(mMutex);

	for(;;) // loop until there is no collision
	{
		auto ID=cFlowConnectionID::GenerateRandom();
		auto i=mConnections.lower_bound(ID);
		if(i==mConnections.end()||i->first!=ID)
		{
			auto j=mServerCreators.find(LoginMessage.mMessageSuiteID);
			auto NewConnection=j->second(ID, LoginMessage.mUserID);
			if(NewConnection)
			{
				mConnections.insert(i, { ID,NewConnection });
			}
			return NewConnection;
		}
	}
}

void cFlowServerConnectionManager::RegisterServerCreatorFunction(uint32_t MessageSuiteID, const cServerConnectionCreatorFunction &Function)
{
	cMutexGuard Guard(mMutex);

	auto i=mServerCreators.lower_bound(MessageSuiteID);
	if(ASSERTTRUE(i==mServerCreators.end()||i->first!=MessageSuiteID)) // there can be only one creator function / message suite
	{
		mServerCreators.insert(i, { MessageSuiteID, Function });
	}
}
