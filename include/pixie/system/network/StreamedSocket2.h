#pragma once

class cSocketCommunicator;

class cStreamedSocket2: public cSocketHandler
{
	std::shared_ptr<cTextLog> mLog;
	std::unique_ptr<cSocket> mSocket;
	cStreamedSocketCommunicator *mCommunicator=nullptr;
	typedef std::vector<std::shared_ptr<cMemoryStream>> cOutgoingDataList;
	cOutgoingDataList mOutgoingDataList;
	cMemoryStream mIncomingBuffer;
	bool mWasParseCalled=false;
// cSocketHandler interface:
	virtual void OnConnected() override;
	virtual void OnCanRead() override;
	virtual void OnCanWrite() override;
	virtual void OnClosed() override;
public:
	cStreamedSocket2(cStreamedSocketCommunicator *Communicator);
	cStreamedSocket2(cStreamedSocketCommunicator *Communicator, std::unique_ptr<cSocket> Socket);
	cStreamedSocket2(const cStreamedSocket2 &) = delete;
	virtual ~cStreamedSocket2();

	void SetLog(std::shared_ptr<cTextLog> Log);
	void Send(std::shared_ptr<cMemoryStream> Data);
	void Close();
	void Connect(const cSocketAddress &Address);
};


