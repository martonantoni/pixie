#include "StdAfx.h"
#include "pixie/flow/i_flow.h"


cFlowConnectionID cFlowConnectionID::GenerateRandom()
{
	std::uniform_int_distribution<uint64_t> dist(1);   // generating in [1,~0u]: cannot include 0, because that is the invalid id.
	std::random_device rd;
	return cFlowConnectionID(dist(rd));
}

void cFlowConnectionID::ToStream(cMemoryStream &Stream) const
{
	Stream.WriteQWORD(mID);
}

void cFlowConnectionID::FromStream(const cMemoryStream &Stream)
{
	mID=Stream.ReadQWORD();
}

std::string cFlowConnectionID::ToString() const
{
	return fmt::sprintf("%llu", mID);
}
