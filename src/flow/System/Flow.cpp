#include "StdAfx.h"
#include "pixie/flow/i_flow.h"

cFlow theFlow;

void cFlow::DispatchMessage_MT(const std::shared_ptr<cFlowConnection> &Connection, const std::function<void()> DispatcherFunction)
{
	DispatcherFunction();
}

bool cFlow::MessageReceived(std::shared_ptr<cFlowConnection> Connection, cFlowStream *Stream, const cFlowMessageHeader &Header, const cMemoryStream &Message)
{
	auto FunctionToCall=Connection->MessageReceived(Stream, Header, Message);
	if(!FunctionToCall)
		return false;
	::CallBack(theMainThread, eCallbackType::Normal, this, &cFlow::DispatchMessage_MT, std::move(Connection), std::move(FunctionToCall));
	return true;
}

void cFlow::StreamClosed(std::shared_ptr<cFlowConnection> Connection, cFlowStream *Stream)
{
	Connection->StreamClosed(Stream);
}

bool cFlow::LoginReplyArrived(std::shared_ptr<cFlowConnection> Connection, const cFlowLoginReplyMessage &LoginReplyMessage, unsigned int LastKnownSequence)
{
	return Connection->LoginReplyArrived(LoginReplyMessage, LastKnownSequence);
}

void cFlow::Connect_NT(const std::shared_ptr<cFlowClientConnection> &Connection)
{
	MainLog->Log("cFlow::Connect_NT");
	auto Stream=std::make_unique<cFlowClientStream>(Connection);
	Stream->SetLog(Connection->mLog);
	Stream->Connect(Connection->mAddress);
	Connection->mStream=std::move(Stream);
}

void cFlow::Connect(std::shared_ptr<cFlowClientConnection> Connection)
{
	::CallBack(gNetworkThread, eCallbackType::Normal, this, &cFlow::Connect_NT, std::move(Connection));
}

void cFlow::SendMessage_NT(const std::shared_ptr<cFlowConnection> &Connection, const std::shared_ptr<const cFlowMessageBase> &Message)
{
	Connection->SendMessage_NT(*Message, cFlowConnection::eMessageType::Normal);
}

void cFlow::SendMessage(std::shared_ptr<cFlowConnection> Connection, std::shared_ptr<const cFlowMessageBase> Message)
{
	::CallBack(gNetworkThread, eCallbackType::Normal, this, &cFlow::SendMessage_NT, std::move(Connection), std::move(Message));
}

void cFlow::SetLog_NT(const std::shared_ptr<cFlowConnection> &Connection, std::shared_ptr<cTextLog> Log)
{
	Connection->SetLog_NT(std::move(Log));
}

void cFlow::SetLog(std::shared_ptr<cFlowConnection> Connection, std::shared_ptr<cTextLog> Log)
{
	::CallBack(gNetworkThread, eCallbackType::Normal, this, &cFlow::SetLog_NT, std::move(Connection), std::move(Log));
}

void cFlow::FatalError_MT(const std::shared_ptr<cFlowConnection> &Connection)
{
	Connection->ProcessFatalError();
}

void cFlow::FatalError(std::shared_ptr<cFlowConnection> Connection)
{
	::CallBack(theMainThread, eCallbackType::Normal, this, &cFlow::FatalError_MT, std::move(Connection));
}
