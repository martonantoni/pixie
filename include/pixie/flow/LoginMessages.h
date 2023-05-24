#pragma once

struct cFlowLoginMessage: public cFlowMessageBase
{
	FLOW_MESSAGE_TYPE(lgin);
	cFlowConnectionID mConnectionID;   // if invalid: requesting a new connection
									   // if valid: want to rejoin an existing connection
	cFlowUserID mUserID;               // cannot be invalid
	uint32_t mMessageSuiteID;

	virtual std::shared_ptr<cMemoryStream> ToStream() const override;
	virtual bool FromStream(const cMemoryStream &Stream) override;
	std::string ToString() const;
};

struct cFlowLoginReplyMessage: public cFlowMessageBase
{
	FLOW_MESSAGE_TYPE(lgir)
	cFlowConnectionID mConnectionID;  // if invalid: login rejected
									  // if valid: login accepted
	std::string mRejectReason;        // only filled out if mConnectionID is invalid

	virtual std::shared_ptr<cMemoryStream> ToStream() const override;
	virtual bool FromStream(const cMemoryStream &Stream) override;
};

