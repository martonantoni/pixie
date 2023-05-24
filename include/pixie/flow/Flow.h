#pragma once

class cFlowConnection;
class cFlowClientConnection;
class cFlowServerConnection;
class cFlowStream;
struct cFlowLoginReplyMessage;

class cFlow
{
	void DispatchMessage_MT(const std::shared_ptr<cFlowConnection> &Connection, const std::function<void()> DispatcherFunction);
	void Connect_NT(const std::shared_ptr<cFlowClientConnection> &Connection);
	void SendMessage_NT(const std::shared_ptr<cFlowConnection> &Connection, const std::shared_ptr<const cFlowMessageBase> &Message);
	void SetLog_NT(const std::shared_ptr<cFlowConnection> &Connection, std::shared_ptr<cTextLog> Log);
	void FatalError_MT(const std::shared_ptr<cFlowConnection> &Connection);
public:
// interface for Stream:
	bool MessageReceived(std::shared_ptr<cFlowConnection> Connection, cFlowStream *Stream, const cFlowMessageHeader &Header, const cMemoryStream &Message);
	void StreamClosed(std::shared_ptr<cFlowConnection> Connection, cFlowStream *Stream);
	bool LoginReplyArrived(std::shared_ptr<cFlowConnection> Connection, const cFlowLoginReplyMessage &LoginReplyMessage, unsigned int LastKnownSequence);
	void FatalError(std::shared_ptr<cFlowConnection> Connection);
// interface for the users of the Connections:
	void Connect(std::shared_ptr<cFlowClientConnection> Connection);
	void SendMessage(std::shared_ptr<cFlowConnection> Connection, std::shared_ptr<const cFlowMessageBase> Message);
	void SetLog(std::shared_ptr<cFlowConnection> Connection, std::shared_ptr<cTextLog> Log);
};

extern cFlow theFlow;