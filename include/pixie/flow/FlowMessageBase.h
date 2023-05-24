#pragma once

#define FLOW_MESSAGE_TYPE(x) \
 	static constexpr uint32_t MessageType() { return (#x[3]<<24)|(#x[2]<<16)|(#x[1]<<8)|#x[0]; } \
	virtual uint32_t GetMessageType() const override { return MessageType(); }

struct cFlowMessageBase
{
	virtual std::shared_ptr<cMemoryStream> ToStream() const=0;
	virtual bool FromStream(const cMemoryStream &Stream)=0; // Returns false if there was some error parsing the message from the stream.
	                                                        // If it failed to read message because of read error from Stream, it does not have to
	                                                        // return false (it might though). 
	                                                        // (false means that there was some "logical" error in the message).
	                                                        //
	                                                        // Example: the first byte could be an ID for a couple of submessage types. Even though
	                                                        //    the Stream has plenty of additional data, the FromStream function might decide after the 
	                                                        //    first byte that the message is invalid, and there's no point (nor definition of how) to read
	                                                        //    the rest of the message. In this case it has to return false. (the caller can't detect
	                                                        //    the parse error based on the state of the Stream).
	virtual uint32_t GetMessageType() const=0;
};

#define FLOW_MESSAGE_SUITE_TYPE(x) \
 	static constexpr uint32_t MessageSuiteID() { return (#x[3]<<24)|(#x[2]<<16)|(#x[1]<<8)|#x[0]; } 

#define FLOW_DECLARE_MESSAGE(MessageClass, MessageType) \
	struct MessageClass: public cMessageBase \
	{ \
		virtual std::shared_ptr<cMemoryStream> ToStream() const override; \
		virtual bool FromStream(const cMemoryStream &Stream) override; \
		virtual void Dispatch(cFlowUserID UserID, cMessageProcessor &MessageProcessor) { MessageProcessor.Process(UserID, *this); } \
		FLOW_MESSAGE_TYPE(MessageType);

#define FLOW_PARSE_MESSAGE_CASE(MessageClass) \
	case MessageClass::MessageType(): \
	{ \
		auto Message=std::make_unique<MessageClass>(); \
		if(!Message->MessageClass::FromStream(Stream)) \
			return nullptr; \
		return Message; \
	}

#define FLOW_REGISTER_MESSAGE_SUITE(MessageSuite) \
struct MessageSuite##Registrator \
{ \
	MessageSuite##Registrator() \
	{ \
		cFlowServerConnectionManager::Get().RegisterServerCreatorFunction(MessageSuite::MessageSuiteID(), \
			[](cFlowConnectionID ID, cFlowUserID UserID) \
			{ \
				auto NewConnection=std::make_shared<tFlowServerConnection<MessageSuite>>(ID, UserID); \
				MessageSuite::HandleNewConnection(NewConnection); \
				return NewConnection; \
			}); \
			\
	} \
} InstanceOf_##MessageSuite##Registrator; 


/*

                           SAMPLE MESSAGESUITE


struct cTestMessageSuite
{
	FLOW_MESSAGE_SUITE_TYPE(test);

	struct cChatMessage;

	class cMessageProcessor
	{
	public:
		virtual void Process(const cChatMessage &) {}
	};

	class cMessageBase: public cFlowMessageBase
	{
	public:
		virtual void Dispatch(cMessageProcessor &MessageProcessor)=0;
	};

	FLOW_DECLARE_MESSAGE(cChatMessage,chat)
		std::string mSender;
		std::string mText;
	};

	static std::unique_ptr<cMessageBase> ParseMessage(const cFlowMessageHeader &Header, const cMemoryStream &Stream);

	static void HandleNewConnection(std::shared_ptr<tFlowClientConnection<cTestMessageSuite>> ClientConnection) {}
	static void HandleNewConnection(std::shared_ptr<tFlowServerConnection<cTestMessageSuite>> ServerConnection);
};

REGISTER_FLOW_MESSAGE_SUITE(cTestMessageSuite);

*/