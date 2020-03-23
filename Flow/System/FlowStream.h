#pragma once

class cFlowConnection;

class cFlowStream: public cStreamedSocketCommunicator, public cDropGuarded
{
protected:
	std::weak_ptr<cFlowConnection> mConnection;
	bool mIsConnected=false;
	std::string mDebugPrompt;
	std::shared_ptr<cTextLog> mLog;
	bool mFatalErrorEncountered=false;
	bool mFirstMessageReceived=false;
	void FatalError(int id);
	virtual bool ProcessFirstMessage(const cMemoryStream &Stream, const cFlowMessageHeader &Header)=0;
	virtual void HandleClosedStream()=0;
	void SetConnection(std::shared_ptr<cFlowConnection> Connection);
private:
// cSocketCommunicator interface:
	virtual void ParseStream(const cMemoryStream &Stream) override;
	virtual void StreamClosed() override;
protected:
	cFlowStream(std::shared_ptr<cFlowConnection> Connection);
	cFlowStream(): cFlowStream(nullptr) {}
	virtual ~cFlowStream()=default;
public:
	void Close();
	void SetDebugName(const std::string &DebugName) { mDebugPrompt=fmt::sprintf("[%s] ", DebugName.c_str()); }
	void SetLog(std::shared_ptr<cTextLog> Log);
	static std::shared_ptr<cMemoryStream> CreateMemoryStreamForWriting();
	void Send(const cFlowRawMessage &RawMessage);
	bool IsConnected() const { return mIsConnected; }
};

USE_DROP_INSTEAD_DELETE_PARENT(cFlowStream, cDropGuarded);

class cFlowClientStream: public cFlowStream
{
	cSocketAddress mAddress;
	cRegisteredID mReconnectTimerID;
	void Connect();
	virtual bool ProcessFirstMessage(const cMemoryStream &Stream, const cFlowMessageHeader &Header) override;
	virtual void HandleClosedStream() override;
	virtual void StreamConnected() override;
protected:
	virtual ~cFlowClientStream()=default;
public:
	cFlowClientStream(std::shared_ptr<cFlowConnection> Connection);
	void Connect(const cSocketAddress &Address);            // for Client
};

USE_DROP_INSTEAD_DELETE_PARENT(cFlowClientStream, cFlowStream);

class cFlowServerStream: public cFlowStream
{
	virtual bool ProcessFirstMessage(const cMemoryStream &Stream, const cFlowMessageHeader &Header) override;
	virtual void HandleClosedStream() override;
protected:
	virtual ~cFlowServerStream()=default;
public:
	void AttachToSocket(std::unique_ptr<cSocket> Socket);  // for Server
};

USE_DROP_INSTEAD_DELETE_PARENT(cFlowServerStream, cFlowStream);
