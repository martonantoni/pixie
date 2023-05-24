#include "StdAfx.h"
#include "pixie/flow/i_flow.h"
#include "pixie/flow/LoginMessages.h"

cFlowStream::cFlowStream(std::shared_ptr<cFlowConnection> Connection)
	: mConnection(Connection)
{
	SetDropThread(gNetworkThread);
}

void cFlowStream::FatalError(int id)
{
	mFatalErrorEncountered=true;
	MainLog->Log("FatalError (%d)", id);
	mStream.reset();
	auto Connection=mConnection.lock();
	if(Connection)
		theFlow.FatalError(Connection);
}

void cFlowStream::ParseStream(const cMemoryStream &Stream)
{
	cDropGuard Guard(this);
	for(;!IsDropped();)
	{
		if(Stream.GetLengthLeft()<cFlowMessageHeader::GetStreamSize())
			return;
		auto MessageHeaderStartPosition=Stream.GetPosition();
		cFlowMessageHeader MessageHeader;
		if(!MessageHeader.FromStream(Stream)||Stream.GetLengthLeft()<MessageHeader.mMessageLength)
		{
			Stream.SetPosition(MessageHeaderStartPosition);
			return;
		}
		auto MessageStartPosition=Stream.GetPosition();
		if(!mFirstMessageReceived&&MessageHeader.mMessageLength>5000)
		{
			MainLog->Log("ERROR: %shello message too long (%u)", mDebugPrompt.c_str(), MessageHeader.mMessageLength);
			FatalError(1);
			return;
		}
		if(!mFirstMessageReceived)
		{
			if(!ProcessFirstMessage(Stream, MessageHeader))
			{
				FatalError(2);
				return;
			}
			mFirstMessageReceived=true;
			continue;
		}
		auto Connection=mConnection.lock();
		if(!Connection)
		{
			FatalError(3);
			return;
		}
		if(!theFlow.MessageReceived(std::move(Connection), this, MessageHeader, Stream))
		{
			FatalError(4);
			return;
		}
		if(ASSERTFALSE(Stream.WasReadError()||Stream.GetPosition()-MessageStartPosition!=MessageHeader.mMessageLength))
		{
			FatalError(5);
			return;
		}
	}
}

void cFlowStream::StreamClosed()
{
	cDropGuard Guard(this);
	HandleClosedStream();
	mFirstMessageReceived=false;
}

void cFlowStream::Close()
{
	mStream->Close();
}

void cFlowStream::Send(const cFlowRawMessage &RawMessage)
{
	MainLog->Log("Send: %.4s", (const char *)&RawMessage.mMessageType);
	cFlowMessageHeader Header;
	auto Connection=mConnection.lock();
	Header.mLastKnownSeq=Connection?Connection->GetLastKnownSeq():0;
	Header.mMessageLength=RawMessage.mMemoryStream->GetLength()-cFlowMessageHeader::GetStreamSize();
	Header.mMessageType=RawMessage.mMessageType;
	RawMessage.mMemoryStream->SetPosition(0);
	Header.ToStream(*RawMessage.mMemoryStream);
	mStream->Send(RawMessage.mMemoryStream);
}

void cFlowStream::SetConnection(std::shared_ptr<cFlowConnection> Connection)
{
	mConnection=Connection;
}

void cFlowStream::SetLog(std::shared_ptr<cTextLog> Log)
{
	mLog=std::move(Log);
	if(mStream)
		mStream->SetLog(mLog);
}

/// ----------------------------------------------- C L I E N T ----------------------------------------------

cFlowClientStream::cFlowClientStream(std::shared_ptr<cFlowConnection> Connection)
	: cFlowStream(Connection)
{
}

void cFlowClientStream::Connect(const cSocketAddress &Address)
{
	mAddress=Address;
	Connect();
}

void cFlowClientStream::Connect()
{
	mStream=std::make_unique<cStreamedSocket2>(this);
	mStream->SetLog(mLog);
	mStream->Connect(mAddress);
}

bool cFlowClientStream::ProcessFirstMessage(const cMemoryStream &Stream, const cFlowMessageHeader &Header)
{
	auto Connection=mConnection.lock();
	if(!Connection)
		return false;
	if(ASSERTFALSE(Header.mMessageType!=cFlowLoginReplyMessage::MessageType()))
	{
		MainLog->Log("ERROR: First message (%.4s) is not LoginReply.", (const char *)&Header.mMessageType);
		FatalError(6);
		return false;
	}
	cFlowLoginReplyMessage LoginReplyMessage;
	if(!LoginReplyMessage.FromStream(Stream))
	{
		MainLog->Log("ERROR: LoginReply parse error.");
		FatalError(7);
		return false;
	}
	if(!theFlow.LoginReplyArrived(std::move(Connection), LoginReplyMessage, Header.mLastKnownSeq))
	{
		FatalError(8);
		return false;
	}
	return true;
}

void cFlowClientStream::HandleClosedStream()
{
	if(mIsConnected&&!mFirstMessageReceived)
	{
		FatalError(9);
	}
	mIsConnected=false;
	if(ASSERTFALSE(mConnection.expired()))
	{
		Drop();
		return;
	}
	if(!mFatalErrorEncountered)
	{
		mReconnectTimerID=cThread::GetCurrentThread()->AddTimer([this]() { Connect(); }, cTimerRequest(1000, cTimerRequest::OnlyOnce));
	}
}

void cFlowClientStream::StreamConnected()
{
	MainLog->Log("cFlowClientStream::StreamConnected");
	cFlowLoginMessage LoginMessage;
	auto Connection=mConnection.lock();
	LoginMessage.mConnectionID=Connection->GetConnectionID();
	LoginMessage.mUserID=Connection->GetUserID();
	LoginMessage.mMessageSuiteID=Connection->GetMessageSuiteID();
	Send({ LoginMessage });
	mIsConnected=true;
}


/// ----------------------------------------------- S E R V E R ----------------------------------------------


void cFlowServerStream::AttachToSocket(std::unique_ptr<cSocket> Socket)
{
	mStream=std::make_unique<cStreamedSocket2>(this, std::move(Socket));
}

bool cFlowServerStream::ProcessFirstMessage(const cMemoryStream &Stream, const cFlowMessageHeader &Header)
{
	if(ASSERTFALSE(Header.mMessageType!=cFlowLoginMessage::MessageType()))
		return false;
	cFlowLoginMessage LoginMessage;
	if(!LoginMessage.FromStream(Stream))
		return false;
	MainLog->Log("LoginMessage arrived -- %s", LoginMessage.ToString().c_str());
	mIsConnected=true;

	auto Connection=LoginMessage.mConnectionID.IsValid()?
		cFlowServerConnectionManager::Get().GetConnection(LoginMessage):
		cFlowServerConnectionManager::Get().CreateConnection(LoginMessage);
	if(!Connection)
	{
		if(LoginMessage.mConnectionID.IsValid())
		{
			MainLog->Log("ERROR: Player %s tried to login with expired/fake ConnectionID: %s", LoginMessage.mUserID.ToString().c_str(), LoginMessage.mConnectionID.ToString().c_str());
		}
		return false;
	}
	SetConnection(Connection);
	return Connection->StreamConnected(std::unique_ptr<cFlowStream>(this), LoginMessage, Header.mLastKnownSeq);
}

void cFlowServerStream::HandleClosedStream()
{
	mIsConnected=false;
	auto Connection=mConnection.lock();
	if(!Connection)
	{
		Drop();
		return;
	}
	theFlow.StreamClosed(std::move(Connection), this);
}

std::shared_ptr<cMemoryStream> cFlowStream::CreateMemoryStreamForWriting()
{
	auto MemoryStream=std::make_shared<cMemoryStream>();
	MemoryStream->SetPosition(cFlowMessageHeader::GetStreamSize());
	return MemoryStream;
}

