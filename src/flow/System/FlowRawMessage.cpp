#include "StdAfx.h"
#include "pixie/flow/i_flow.h"

cFlowRawMessage::cFlowRawMessage(const cFlowMessageBase &Message)
	: mMemoryStream(Message.ToStream())
	, mMessageType(Message.GetMessageType())
{
}
