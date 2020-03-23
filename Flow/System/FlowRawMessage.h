#pragma once

struct cFlowRawMessage final
{
	std::shared_ptr<cMemoryStream> mMemoryStream;
	uint32_t mMessageType;

	cFlowRawMessage(const cFlowMessageBase &Message);
	cFlowRawMessage(const cFlowRawMessage &)=default;
	cFlowRawMessage &operator=(const cFlowRawMessage &)=default;
};
