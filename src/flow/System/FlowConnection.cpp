#include "StdAfx.h"

#include "pixie/flow/i_flow.h"
#include "pixie/flow/LoginMessages.h"

cFlowConnection::~cFlowConnection()
{
// 	if(mLog)
// 		mLog->Log("~cFlowConnection");
}

void cFlowConnection::StreamClosed(cFlowStream *Stream)
{
	ASSERT(gNetworkThread->IsInThread());
	if(Stream!=mStream.get())
		return;
	mStream.reset();
}

std::function<void()> cFlowConnection::MessageReceived(cFlowStream *Stream, const cFlowMessageHeader &Header, const cMemoryStream &Message)
{
	ASSERT(gNetworkThread->IsInThread());
	if(Stream!=mStream.get())
		return {};
	ConfirmDelivery(Header.mLastKnownSeq);
	auto DispatcherFunction=HandleMessage(Header, Message);
	if(DispatcherFunction)
		++mLastKnownSeq;
	return DispatcherFunction;
}

void cFlowConnection::ConfirmDelivery(unsigned int LastKnownSeq)
{
// 	MainLog->Log("ConfirmDelivery: %u  (mSeqOfFirstUnconfirmedMessage: %u, unconfirmed #: %u)", LastKnownSeq, mSeqOfFirstUnconfirmedMessage,
// 		(unsigned int)mUnconfirmedMessages.size());
	if(mSeqOfFirstUnconfirmedMessage<=LastKnownSeq)
	{
		if(ASSERTFALSE(LastKnownSeq>mSeqOfFirstUnconfirmedMessage+mUnconfirmedMessages.size()))
			LastKnownSeq=mSeqOfFirstUnconfirmedMessage+(unsigned int)mUnconfirmedMessages.size();
		if(!mUnconfirmedMessages.empty())
			mUnconfirmedMessages.erase(mUnconfirmedMessages.begin(), mUnconfirmedMessages.begin()+(LastKnownSeq-mSeqOfFirstUnconfirmedMessage)+1);
		mSeqOfFirstUnconfirmedMessage=LastKnownSeq+1;
	}
//	MainLog->Log("after confirm ... mSeqOfFirstUnconfirmedMessage: %u, unconfirmed #: %u", mSeqOfFirstUnconfirmedMessage, (unsigned int)mUnconfirmedMessages.size());
}

void cFlowConnection::ResendMissingMessages(unsigned int LastKnownSeq)
{
	if(ASSERTFALSE(LastKnownSeq+1<mSeqOfFirstUnconfirmedMessage)) // would need to resend messages that we no longer have (were already confirmed)
		return; 
	auto Offset=LastKnownSeq+1-mSeqOfFirstUnconfirmedMessage;
	auto MessagesToResend=mUnconfirmedMessages.size()-Offset;
	if(!MessagesToResend)
		return;
	MainLog->Log("Resending messages: from %u, total messages: %u", LastKnownSeq+1, (unsigned int)MessagesToResend);
	ASSERT(Offset==0); // we just called ConfirmDelivery with LastKnownSeq
	for(auto i=mUnconfirmedMessages.begin()+Offset;i!=mUnconfirmedMessages.end();++i)
	{
		mStream->Send(*i);
	}
}

void cFlowConnection::SendMessage_NT(const cFlowMessageBase &Message, eMessageType MessageType)
{
	cFlowRawMessage RawMessage(Message);
	if(MessageType==eMessageType::Normal)
	{
		mUnconfirmedMessages.emplace_back(RawMessage);
	}
	if(mStream&&mStream->IsConnected())
		mStream->Send(RawMessage);
}

bool cFlowConnection::LoginReplyArrived(const cFlowLoginReplyMessage &LoginReplyMessage, unsigned int LastKnownSequence)
{
	if(!LoginReplyMessage.mRejectReason.empty())
	{
		MainLog->Log("Login rejected: \"%s\"", LoginReplyMessage.mRejectReason.c_str());
		mStream.reset();
		return false;
	}
	mID=LoginReplyMessage.mConnectionID;
	MainLog->Log("Login accepted. LastKnownSeq: %u", LastKnownSequence);
	if(ASSERTFALSE(LastKnownSequence+1<mSeqOfFirstUnconfirmedMessage||LastKnownSequence>mSeqOfFirstUnconfirmedMessage+mUnconfirmedMessages.size()))
	{
		mStream.reset();
		return false;
	}
	ConfirmDelivery(LastKnownSequence);
	ResendMissingMessages(LastKnownSequence);
	return true;
}

void cFlowConnection::SetLog_NT(std::shared_ptr<cTextLog> Log)
{
	mLog=std::move(Log);
	if(mStream)
		mStream->SetLog(Log);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cFlowClientConnection::cFlowClientConnection(cFlowUserID UserID, const cSocketAddress &Address)
	: cFlowConnection(UserID)
	, mAddress(Address)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cFlowServerConnection::cFlowServerConnection(cFlowConnectionID ConnectionID, cFlowUserID UserID)
	: cFlowConnection(ConnectionID, UserID)
{
}

cFlowServerConnection::~cFlowServerConnection()
{
	cFlowServerConnectionManager::Get().ConnectionDied(GetConnectionID());
}

bool cFlowServerConnection::StreamConnected(std::unique_ptr<cFlowStream> Stream, const cFlowLoginMessage &LoginMessage, unsigned int LastKnownSequence)
{
	MainLog->Log("StreamConnected ... LastKnownSeq: %u", LastKnownSequence);
	ASSERT(gNetworkThread->IsInThread());
	mStream=std::move(Stream);
	if(ASSERTTRUE(mStream))
	{
		if(ASSERTFALSE(LastKnownSequence+1<mSeqOfFirstUnconfirmedMessage||LastKnownSequence>mSeqOfFirstUnconfirmedMessage+mUnconfirmedMessages.size()))
		{
			RejectLogin("invalid LastKnownSeq");
			mStream.reset();
			return false;
		}
		ConfirmDelivery(LastKnownSequence);
		mUserID=LoginMessage.mUserID;
		AcceptLogin();
		ResendMissingMessages(LastKnownSequence);
	}
	return true;
}

void cFlowServerConnection::RejectLogin(const std::string &RejectReason)
{
	cFlowLoginReplyMessage Reply;
	Reply.mRejectReason=RejectReason;
	SendMessage_NT(Reply, eMessageType::Control);
}

void cFlowServerConnection::AcceptLogin()
{
	cFlowLoginReplyMessage Reply;
	Reply.mConnectionID=mID;
	SendMessage_NT(Reply, eMessageType::Control);
}
