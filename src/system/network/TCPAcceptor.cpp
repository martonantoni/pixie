#include "StdAfx.h"

#include "SocketErrorCodes.h"

cTCPAcceptor::cTCPAcceptor(cSocketTaker *SocketTaker)
	: mSocketTaker(SocketTaker)
{
	SetDestructorThread(gNetworkThread);
}

cTCPAcceptor::~cTCPAcceptor()
{
}

void cTCPAcceptor::OnConnected()
{
	ASSERT(false);
}

void cTCPAcceptor::OnCanAccept()
{
	std::unique_ptr<cSocket> NewSocket(mListeningSocket->Accept());
	cMutexGuard Guard(mLock);
	if(mSocketTaker)
		mSocketTaker->OnNewSocket(std::move(NewSocket));
}

void cTCPAcceptor::OnCanRead()
{
	ASSERT(false);
}

void cTCPAcceptor::OnCanWrite()
{
	ASSERT(false);
}

void cTCPAcceptor::OnClosed()
{
	ASSERT(false);
}

void cTCPAcceptor::Listen_NetThread(unsigned short Port)
{
	ASSERT(gNetworkThread->IsInThread());
	mListeningSocket.reset(new cSocket);
	mListeningSocket->CreateTCP();
	mListeningSocket->SetLog(mTextLog);
	mListeningSocket->SetHandler(this);
	mListeningSocket->Listen(Port);
	mListeningSocket->Resume();
	Unref();
}

void cTCPAcceptor::Listen(unsigned short Port)
{
	Ref(); // Unref is in Listen_NetThread
	::CallBack(gNetworkThread, eCallbackType::Normal, this, &cTCPAcceptor::Listen_NetThread, Port);
}

void cTCPAcceptor::SetLog(std::shared_ptr<cTextLog> log)
{
    ASSERT(gNetworkThread->IsInThread());
	mTextLog = std::move(log);
	if (mListeningSocket)
	{
		mListeningSocket->SetLog(mTextLog);
	}
}

