#pragma once

struct cFlowMessageHeader final
{
	uint32_t mMessageType;
	size_t mMessageLength;
	unsigned int mLastKnownSeq;

	void ToStream(cMemoryStream &Stream) const;
	bool FromStream(const cMemoryStream &Stream);
	static constexpr size_t GetStreamSize()
	{
		return sizeof(uint32_t)*3;
	}
};