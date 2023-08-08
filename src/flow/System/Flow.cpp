#include "StdAfx.h"
#include "pixie/flow/i_flow.h"

cFlow theFlow;

bool cFlow::MessageReceived(std::shared_ptr<cFlowConnection> Connection, cFlowStream *Stream, const cFlowMessageHeader &Header, const cMemoryStream &Message)
{
	auto FunctionToCall=Connection->MessageReceived(Stream, Header, Message);
	if(!FunctionToCall)
		return false;
	theMainThread->callback(
		[this, connection = std::move(Connection), FunctionToCall = std::move(FunctionToCall)]()
		{
			FunctionToCall();
		});
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
	gNetworkThread->callback(
		[this, connection = std::move(Connection)]()
		{
			Connect_NT(connection);
		});
}

void cFlow::SendMessage_NT(const std::shared_ptr<cFlowConnection> &Connection, const std::shared_ptr<const cFlowMessageBase> &Message)
{
	Connection->SendMessage_NT(*Message, cFlowConnection::eMessageType::Normal);
}

void cFlow::SendMessage(std::shared_ptr<cFlowConnection> Connection, std::shared_ptr<const cFlowMessageBase> Message)
{
	gNetworkThread->callback(
		[this, connection = std::move(Connection), message = std::move(Message)]()
		{
			SendMessage_NT(connection, message);
		});
}

void cFlow::SetLog_NT(const std::shared_ptr<cFlowConnection> &Connection, std::shared_ptr<cTextLog> Log)
{
	Connection->SetLog_NT(std::move(Log));
}

void cFlow::SetLog(std::shared_ptr<cFlowConnection> Connection, std::shared_ptr<cTextLog> Log)
{
	gNetworkThread->callback([this, connection = std::move(Connection), log = std::move(Log)]()
		{
			SetLog_NT(connection, log);
		});
}

void cFlow::FatalError_MT(const std::shared_ptr<cFlowConnection> &Connection)
{
	Connection->ProcessFatalError();
}

void cFlow::FatalError(std::shared_ptr<cFlowConnection> Connection)
{
	theMainThread->callback([this, connection = std::move(Connection)]()
	{
		FatalError_MT(connection);
	});
}
