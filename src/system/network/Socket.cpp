#include "StdAfx.h"

#include "i_Network.h"
#include "SocketErrorCodes.h"

cThread *gNetworkThread=NULL;


#define CHECK_SOCKET_ERROR(Condition,FunctionName) \
	if(Condition) { Error(#FunctionName, ::WSAGetLastError()); return; }

cSocket::cSocket(SOCKET Socket)
	: mSocket(Socket)
{
	CreateEvent();
}

cSocket::~cSocket()
{
	Cleanup();
}

void cSocket::CreateTCP()
{
	ASSERT(gNetworkThread->IsInThread());
	ASSERT(mSocket==INVALID_SOCKET);
	mSocket=::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	CHECK_SOCKET_ERROR(mSocket==INVALID_SOCKET, WSASocket);
	CreateEvent();
}

void cSocket::CreateEvent()
{
	ASSERT(!mSocketEvent&&!mCloseEvent);
	HANDLE EventHandle=::WSACreateEvent();
	CHECK_SOCKET_ERROR(EventHandle==WSA_INVALID_EVENT, WSACreateEvent);
	CHECK_SOCKET_ERROR(::WSAEventSelect(mSocket, EventHandle, FD_READ|FD_WRITE|FD_CLOSE|FD_CONNECT|FD_ACCEPT)==SOCKET_ERROR, WSAEventSelect);
	mSocketEvent.reset(new cNativeEvent(EventHandle));
	mCloseEvent.reset(new cNativeEvent(cNativeEvent::ManualReset));
}

#define SOCKET_CHECK_ERROR_BIT(Type, TypeText) \
	if(NetworkEvents.lNetworkEvents&FD_##Type&&NetworkEvents.iErrorCode[FD_##Type##_BIT]) \
	{ \
		if(NetworkEvents.iErrorCode[FD_##Type##_BIT]==WSAEWOULDBLOCK) \
		{ \
			MainLog->Log("Warning: WSAEWOULDBLOCK received as an event for FD_"#Type); \
			NetworkEvents.lNetworkEvents&=~FD_##Type; \
		} \
		else \
		{ \
			Error(TypeText, NetworkEvents.iErrorCode[FD_##Type##_BIT]); \
			ErrorHappened=true; \
		} \
	}

void cSocket::OnSocketEvent()
{
	ASSERT(mSocketHandler);
	if(!mSocketHandler)
		return;
	WSANETWORKEVENTS NetworkEvents;
	CHECK_SOCKET_ERROR(::WSAEnumNetworkEvents(mSocket, mSocketEvent->GetHandle() , &NetworkEvents)==SOCKET_ERROR, WSAEnumNetworkEvents);
	int ErrorHappened=false;
	SOCKET_CHECK_ERROR_BIT(READ, "Read");
	SOCKET_CHECK_ERROR_BIT(CONNECT, "Connect");
	SOCKET_CHECK_ERROR_BIT(WRITE, "Write");
	SOCKET_CHECK_ERROR_BIT(ACCEPT, "Accept");
	if(!ErrorHappened)
	{
		cDestroyTrapGuard DestroyGuard(mDestroyTrap);
		if(NetworkEvents.lNetworkEvents&FD_CONNECT)
			mSocketHandler->OnConnected();
		if(DestroyGuard.Triggered()||!mSocketEventID.IsValid()||!mSocketHandler) return;
		if(NetworkEvents.lNetworkEvents&FD_ACCEPT)
			mSocketHandler->OnCanAccept();
		if(DestroyGuard.Triggered()||!mSocketEventID.IsValid()||!mSocketHandler) return;
		if(NetworkEvents.lNetworkEvents&FD_READ)
			mSocketHandler->OnCanRead();
		if(DestroyGuard.Triggered()||!mSocketEventID.IsValid()||!mSocketHandler) return;
		if(NetworkEvents.lNetworkEvents&FD_WRITE)
			mSocketHandler->OnCanWrite();
		if(DestroyGuard.Triggered())
			return;
		if(NetworkEvents.lNetworkEvents&FD_CLOSE)
		{
			mSocketEventID.Unregister();
			mCloseEvent->Set();
		}
	}
}

#undef SOCKET_CHECK_ERROR_BIT

void cSocket::OnClose()
{
	ASSERT(mSocketHandler);
	if(!mSocketHandler)
		return;
	Cleanup();
	mSocketHandler->OnClosed();  // warning, this might delete this object!
}

void cSocket::Cleanup()
{
	mCloseID.Unregister();
	mSocketEventID.Unregister();
	mCloseEvent.reset();
	mSocketEvent.reset();
	::closesocket(mSocket);
	mSocket=INVALID_SOCKET;
}

void cSocket::Connect(const cSocketAddress &Address)
{
	if(mLog)
		mLog->Log("Connecting to %s", Address.ToString().c_str());
	ASSERT(gNetworkThread->IsInThread());
	const sockaddr_in *ResolvedAddress=Address.GetSockaddr_in();
	if(!ResolvedAddress)
	{
		auto LastError=::WSAGetLastError();
		Error("resolve", LastError);
	}
	if(::connect(mSocket, (const sockaddr *)ResolvedAddress, sizeof(sockaddr_in)))
	{
		auto LastError=::WSAGetLastError();
		if(LastError!=WSAEWOULDBLOCK)
			Error("connect", LastError);
	}
}

void cSocket::Listen(unsigned short Port)
{
	if(mLog)
		mLog->Log("Listening on port %u", Port);
	ASSERT(gNetworkThread->IsInThread());
	sockaddr_in Address;
	Address.sin_family=AF_INET;
	Address.sin_addr.S_un.S_addr=0;
	Address.sin_port=htons(Port);
	CHECK_SOCKET_ERROR(::bind(mSocket, (sockaddr *)&Address, sizeof(sockaddr_in))==SOCKET_ERROR, bind);
	CHECK_SOCKET_ERROR(::listen(mSocket, 2)==SOCKET_ERROR, listen);
}

std::unique_ptr<cSocket> cSocket::Accept()
{
	ASSERT(gNetworkThread->IsInThread());
	sockaddr_in SocketAddress;
	int SizeOfSockaddr_in=sizeof(sockaddr_in);
	SOCKET NewSocket=::accept(mSocket, (sockaddr *)&SocketAddress, &SizeOfSockaddr_in);
	if(NewSocket==SOCKET_ERROR)
		return std::unique_ptr<cSocket>();
	cSocket *NewSocketObject=new cSocket(NewSocket);
//	NewSocketObject->Name=fmt::sprintf("%s#client", Name); // a temporary name until BindToObject is called
	return std::unique_ptr<cSocket>(NewSocketObject);
}

void cSocket::Close()
{
	ASSERT(gNetworkThread->IsInThread());
	if(mSocket!=INVALID_SOCKET)
		::closesocket(mSocket);
	mCloseEvent->Set();
}

void cSocket::Error(const char *FailedFunction, int ErrorCode)
{
	if(mLog)
		mLog->Log("%s failed: %s", FailedFunction, GetWSAErrorCodeString(ErrorCode));
	mSocketEventID.Unregister();
	Close();
}

size_t cSocket::Read(char *Buffer, size_t Length)
{
	ASSERT(gNetworkThread->IsInThread());
	int Received=::recv(mSocket, Buffer, (int)Length, 0);
	if(Received==SOCKET_ERROR)
	{
		auto LastError=::WSAGetLastError();
		if(LastError!=WSAEWOULDBLOCK)
		{
			Error("recv", LastError);
			return 0;
		}
		return 0;
	}
	if(Received<0)
	{
		Error("recv", ::WSAGetLastError());
		return 0;
	}
	if(Received==0)
	{
		// gracefully closed from other side
		Close(); 
		return 0;
	}
	return Received;
}

size_t cSocket::Write(const char *Buffer, size_t Length)
{
	ASSERT(gNetworkThread->IsInThread());
	int Sent=::send(mSocket, Buffer, (int)Length, 0);
	if(Sent==SOCKET_ERROR)
	{
		auto LastError=::WSAGetLastError();
		if(LastError!=WSAEWOULDBLOCK)
		{
			Error("send", LastError);
			return 0;
		}
		return 0;
	}
	if(Sent<0)
	{
		Error("send", ::WSAGetLastError());
		return 0;
	}
	return Sent;
}

void cSocket::SetHandler(cSocketHandler *Handler)
{
	ASSERT(gNetworkThread->IsInThread());
	mSocketHandler=Handler;
}

void cSocket::Suspend()
{
	ASSERT(gNetworkThread->IsInThread());
	mSocketEventID.Unregister();
	mCloseID.Unregister();
}

void cSocket::Resume()
{
	ASSERT(gNetworkThread->IsInThread());
	ASSERT(!!mSocketEvent&&!!mCloseEvent);
	mSocketEventID=gNetworkThread->AddEventHandler([this]() { OnSocketEvent(); }, mSocketEvent.get());
	mCloseID=gNetworkThread->AddEventHandler([this]() { OnClose(); }, mCloseEvent.get());
}

void cSocket::SetLog(std::shared_ptr<cTextLog> Log)
{
	ASSERT(gNetworkThread->IsInThread());
	mLog=std::move(Log);
}

#undef CHECK_SOCKET_ERROR

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InitializeNetwork()
{
	gNetworkThread=theThreadServer->GetThread("network");

	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested=MAKEWORD(2, 0);
	if(WSAStartup(wVersionRequested, &wsaData))
	{
		THROW_DETAILED_EXCEPTION("WSAStartup failed");
	}
	if(LOBYTE(wsaData.wVersion)!=2||HIBYTE(wsaData.wVersion)!=0)
	{
		WSACleanup();
		THROW_DETAILED_EXCEPTION("WSA version conflict");
		return;
	}
}

REGISTER_AUTO_INIT_FUNCTION_CALL(InitializeNetwork, eProgramPhases::StaticInit, DefaultInitLabel);