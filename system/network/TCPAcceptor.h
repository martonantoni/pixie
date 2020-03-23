#pragma once

class cSocketTaker
{
public:
	virtual ~cSocketTaker()=default;
	virtual void OnNewSocket(std::unique_ptr<cSocket> NewSocket)=0; // called in the network thread
};

class cTCPAcceptor: public cSocketHandler, public cIntrusiveThreadsafeRefCount
{
	cMutex mLock;
	cSocketTaker *mSocketTaker;                 // guarded by mMutex
	std::unique_ptr<cSocket> mListeningSocket;  // accessed only in network thread
	virtual void OnConnected() override;
	virtual void OnCanAccept() override;
	virtual void OnCanRead() override;
	virtual void OnCanWrite() override;
	virtual void OnClosed() override;
	void Listen_NetThread(unsigned short Port);
	void Close_NetThread();
protected:
	virtual ~cTCPAcceptor();
public:
	cTCPAcceptor(cSocketTaker *SocketTaker);
	void Listen(unsigned short Port);   // can be called any thread
	void Close();
};