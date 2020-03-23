#include "StdAfx.h"

cFlowRawMessage::cFlowRawMessage(const cFlowMessageBase &Message)
	: mMemoryStream(Message.ToStream())
	, mMessageType(Message.GetMessageType())
{
}
