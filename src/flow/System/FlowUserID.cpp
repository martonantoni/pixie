#include "StdAfx.h"

#include "pixie/flow/i_flow.h"

cFlowUniqueID::cFlowUniqueID(const std::string &id)
	: mID(strtoull(id.c_str(), nullptr, 16))
{
}

void cFlowUniqueID::ToStream(cMemoryStream &Stream) const
{
	Stream.WriteQWORD(mID);
}

void cFlowUniqueID::FromStream(const cMemoryStream &Stream)
{
	mID=Stream.ReadQWORD();
}

std::string cFlowUniqueID::ToString() const
{
	return fmt::sprintf("%llx", mID);
}

cFlowUniqueID cFlowUniqueID::UnverifiedGenerate()
{
	std::uniform_int_distribution<uint64_t> dist(1);   // generating in [1,~0u]: cannot include 0, because that is the invalid id.
	std::random_device rd;
	return cFlowUniqueID(dist(rd));
}
