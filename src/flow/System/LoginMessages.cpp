#include "StdAfx.h"

#include "pixie/flow/i_flow.h"
#include "pixie/flow/LoginMessages.h"

std::shared_ptr<cMemoryStream> cFlowLoginMessage::ToStream() const
{
	auto Stream=cFlowStream::CreateMemoryStreamForWriting();
	mConnectionID.ToStream(*Stream);
	mUserID.ToStream(*Stream);
	Stream->WriteDWORD(mMessageSuiteID);
	return Stream;
}

bool cFlowLoginMessage::FromStream(const cMemoryStream &Stream)
{
	mConnectionID.FromStream(Stream);
	mUserID.FromStream(Stream);
	mMessageSuiteID=Stream.ReadDWORD();
	return true;
}

std::string cFlowLoginMessage::ToString() const
{
	return fmt::sprintf("ConnectionID: %s, UserID: %s, MessageSuiteID: %.4s", mConnectionID.ToString().c_str(), mUserID.ToString().c_str(), (const char *)&mMessageSuiteID);
}

std::shared_ptr<cMemoryStream> cFlowLoginReplyMessage::ToStream() const
{
	auto Stream=cFlowStream::CreateMemoryStreamForWriting();
	mConnectionID.ToStream(*Stream);
	if(!mConnectionID.IsValid())
		Stream->WriteString(mRejectReason);
	return Stream;
}

bool cFlowLoginReplyMessage::FromStream(const cMemoryStream &Stream)
{
	mConnectionID.FromStream(Stream);
	if(!mConnectionID.IsValid())
		mRejectReason=Stream.ReadString();
	return true;
}
