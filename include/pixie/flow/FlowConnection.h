#pragma once

struct cFlowLoginReplyMessage;
struct cFlowLoginMessage;
class cFlowServerConnection;

class cFlowConnection
{
	friend class cFlow;
protected:
	cFlowConnectionID mID;
	cFlowUserID mUserID;
	typedef std::vector<cFlowRawMessage> cRawMessages;
	cRawMessages mUnconfirmedMessages; // messages that are not confirmed by other side as "received", only accessed in network thread
	unsigned int mSeqOfFirstUnconfirmedMessage=1; // only accessed in network thread
	unsigned int mLastKnownSeq=0; // only accessed in network thread
	std::shared_ptr<cTextLog> mLog;

	void ConfirmDelivery(unsigned int LastKnownSeq);
	void ResendMissingMessages(unsigned int LastKnownSeq);
	enum class eMessageType { Control, Normal };
	void SendMessage_NT(const cFlowMessageBase &Message, eMessageType MessageType);
	virtual std::function<void()> HandleMessage(const cFlowMessageHeader &Header, const cMemoryStream &Stream)=0;
	void SetLog_NT(std::shared_ptr<cTextLog> Log);

	std::unique_ptr<cFlowStream> mStream; // only accessed in network thread

	cFlowConnection(cFlowConnectionID ID, cFlowUserID UserID): mID(ID), mUserID(UserID) {} // Server
	cFlowConnection(cFlowUserID UserID): mUserID(UserID) {} // Client

// interface for the cFlow:
	std::function<void()> MessageReceived(cFlowStream *Stream, const cFlowMessageHeader &Header, const cMemoryStream &Message);
	void StreamClosed(cFlowStream *Stream);
	bool LoginReplyArrived(const cFlowLoginReplyMessage &LoginReplyMessage, unsigned int LastKnownSequence);
	virtual void ProcessFatalError()=0;
protected:
	virtual ~cFlowConnection();
public:
// functions for the FlowStream, all these are called in the network thread:
	unsigned int GetLastKnownSeq() const { return mLastKnownSeq; }
	cFlowConnectionID GetConnectionID() const { return mID; }
	cFlowUserID GetUserID() const { return mUserID; }
	virtual uint32_t GetMessageSuiteID() const=0;
};

class cFlowClientConnection: public cFlowConnection
{
	friend class cFlow;
	cSocketAddress mAddress;
protected:
	virtual ~cFlowClientConnection()=default;
public:
// functions for the outside world (Main Thread only):
	cFlowClientConnection(cFlowUserID UserID, const cSocketAddress &Address);
};


class cFlowServerConnection: public cFlowConnection
{
	void RejectLogin(const std::string &RejectReason);
	void AcceptLogin();
protected:
	virtual ~cFlowServerConnection();
public:
	bool StreamConnected(std::unique_ptr<cFlowStream> Stream, const cFlowLoginMessage &LoginMessage, unsigned int LastKnownSequence);
// functions for the outside world (Main Thread only):
	cFlowServerConnection(cFlowConnectionID ConnectionID, cFlowUserID UserID);
};

template<class BaseClass, class MessageSuite> class tFlowConnection: public BaseClass
{
	tRegisteredObjects<typename MessageSuite::cMessageProcessor *> mMessageProcessors;
protected:
	virtual std::function<void()> HandleMessage(const cFlowMessageHeader &Header, const cMemoryStream &Stream) override
	{
		auto Message=MessageSuite::ParseMessage(Header, Stream);
		if(!Message||Stream.WasReadError())
			return {};
		return [Message=Message.release(), this]() 
		{
			ASSERT(!mMessageProcessors.empty());
			mMessageProcessors.ForEach([&Message=*Message, this](auto *Processor) { Message.Dispatch(this->GetUserID(), *Processor); });
			delete Message; // can't capture unique_ptr and then move the lambda into std::function.
		};
	}
	virtual void ProcessFatalError() override
	{
		mMessageProcessors.ForEach([UserID=this->GetUserID()](auto *Processor) { Processor->ProcessFatalError(UserID); });
	}
public:
// functions for the outside world (Main Thread only):
	cRegisteredID RegisterMessageProcessor(typename MessageSuite::cMessageProcessor *MessageProcessor)
	{
		return mMessageProcessors.Register(MessageProcessor);
	}
	tFlowConnection(cFlowUserID UserID, const cSocketAddress &Address): BaseClass(UserID, Address) {}
	tFlowConnection(cFlowConnectionID ConnectionID, cFlowUserID UserID): BaseClass(ConnectionID, UserID) {}
	virtual uint32_t GetMessageSuiteID() const override { return MessageSuite::MessageSuiteID(); }
};

template<class MessageSuite> using tFlowClientConnection = tFlowConnection<cFlowClientConnection, MessageSuite>;
template<class MessageSuite> using tFlowServerConnection = tFlowConnection<cFlowServerConnection, MessageSuite>;

