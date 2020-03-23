#include "StdAfx.h"

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
	return cFlowUniqueID(dist(std::random_device()));
}
