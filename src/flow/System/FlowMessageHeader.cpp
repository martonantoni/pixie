#include "StdAfx.h"


void cFlowMessageHeader::ToStream(cMemoryStream &Stream) const
{
	Stream.WriteDWORD(mMessageType);
	Stream.WriteDWORD((uint32_t)mMessageLength);
	Stream.WriteDWORD((uint32_t)mLastKnownSeq);
}

bool cFlowMessageHeader::FromStream(const cMemoryStream &Stream)
{
	mMessageType=Stream.ReadDWORD();
	mMessageLength=Stream.ReadDWORD();
	mLastKnownSeq=Stream.ReadDWORD();
	return true;
}

