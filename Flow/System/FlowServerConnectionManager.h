#pragma once

struct cFlowLoginMessage;

class cFlowServerConnectionManager final: public tSingleton<cFlowServerConnectionManager>
{
	mutable cMutex mMutex;
public:
	typedef std::function<std::shared_ptr<cFlowServerConnection>(cFlowConnectionID ID, cFlowUserID UserID)> cServerConnectionCreatorFunction;
private:
	typedef std::map<cFlowConnectionID, std::weak_ptr<cFlowServerConnection>> cConnections; // WARNING: THESE ARE NEVER REMOVED!
	cConnections mConnections;                                                                // refactor is needed!
	typedef std::map<uint32_t, cServerConnectionCreatorFunction> cServerCreators;
	cServerCreators mServerCreators;
public:
	std::shared_ptr<cFlowServerConnection> GetConnection(const cFlowLoginMessage &LoginMessage);
	std::shared_ptr<cFlowServerConnection> CreateConnection(const cFlowLoginMessage &LoginMessage);
	void ConnectionDied(cFlowConnectionID ConnectionID);
	void RegisterServerCreatorFunction(uint32_t MessageSuiteID, const cServerConnectionCreatorFunction &Function);
};

//template<class MessageSuite> 