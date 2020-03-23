#pragma once

class cSocketHandler
{
public:
	virtual ~cSocketHandler()=default;
	virtual void OnConnected() {}
	virtual void OnCanAccept() {}
	virtual void OnCanRead()=0;
	virtual void OnCanWrite()=0;
	virtual void OnClosed()=0;
};

class cSocket final
{
	std::shared_ptr<cTextLog> mLog;
	cSocketHandler *mSocketHandler=nullptr;
	SOCKET mSocket=INVALID_SOCKET;
	std::unique_ptr<cNativeEvent> mSocketEvent, mCloseEvent;
	cRegisteredID mSocketEventID, mCloseID;
	cDestroyTrap mDestroyTrap;
	void OnSocketEvent();
	void OnClose();
	void CreateEvent();
	void Error(const char *FailedFunction, int ErrorCode);
	void Cleanup();
public:
	cSocket() = default;
	cSocket(SOCKET Socket);
	cSocket(const cSocket &) = delete;
	~cSocket();
	void CreateTCP();
	void Connect(const cSocketAddress &Address);
	void Listen(unsigned short Port);
	std::unique_ptr<cSocket> Accept();
	void Close();
	size_t Read(char *Buffer, size_t Length);
	size_t Write(const char *Buffer, size_t Length);
	void SetHandler(cSocketHandler *Handler);
	void SetLog(std::shared_ptr<cTextLog> Log);

	void Suspend();
	void Resume();
};

extern cThread *gNetworkThread;
