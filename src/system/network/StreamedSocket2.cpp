#include "StdAfx.h"

#include "SocketErrorCodes.h"


cStreamedSocket2::cStreamedSocket2(cStreamedSocketCommunicator *Communicator)
	: mCommunicator(Communicator)
{
	ASSERT(mCommunicator);
}

cStreamedSocket2::cStreamedSocket2(cStreamedSocketCommunicator *Communicator, std::unique_ptr<cSocket> Socket)
	: mSocket(std::move(Socket))
	, mCommunicator(Communicator)
{
	ASSERT(mCommunicator);
	mSocket->SetHandler(this);
	mSocket->Resume();
}

cStreamedSocket2::~cStreamedSocket2()
{
	ASSERT(gNetworkThread->IsInThread());
}

void cStreamedSocket2::Connect(const cSocketAddress &Address)
{
	ASSERT(gNetworkThread->IsInThread());
	if(ASSERTFALSE(mSocket))
		return;
	mSocket.reset(new cSocket);
	mSocket->SetLog(mLog);
	mSocket->CreateTCP();
	mSocket->Connect(Address);
	mSocket->SetHandler(this);
	mSocket->Resume();
}

void cStreamedSocket2::Send(std::shared_ptr<cMemoryStream> Data)
{
	ASSERT(gNetworkThread->IsInThread());
	Data->SetPosition(0);
	bool CanTryWrite=mOutgoingDataList.empty();
	mOutgoingDataList.emplace_back(std::move(Data));
	if(CanTryWrite&&mSocket)
	{
		OnCanWrite();
	}
}

void cStreamedSocket2::Close()
{
	ASSERT(gNetworkThread->IsInThread());
	if(mSocket)
		mSocket->Close();
}

void cStreamedSocket2::OnConnected()
{
	if(mLog)
		mLog->Log("Connected");
	if(mCommunicator)
		mCommunicator->StreamConnected(); // warning: may cause this to be destroyed
}

void cStreamedSocket2::OnCanRead()
{
	if(mWasParseCalled)
	{
		mIncomingBuffer.SetAsStart();
		mIncomingBuffer.ShrinkToFit();
		mWasParseCalled=false;
	}
	mIncomingBuffer.SetPosition(mIncomingBuffer.GetLength());
	size_t BytesRead=mSocket->Read(mIncomingBuffer.AccessData(0x8000), 0x8000);
	if(BytesRead)
	{
		mIncomingBuffer.ReportWritten(BytesRead);
		{
			if(mCommunicator)
			{
				mIncomingBuffer.SetPosition(0);
				mWasParseCalled=true;
				mCommunicator->ParseStream(mIncomingBuffer); // warning: may cause this to be destroyed
			}
			else
				return;
		}
	}
}

void cStreamedSocket2::OnCanWrite()
{
	while(!mOutgoingDataList.empty())
	{
		auto &OutgoingData=mOutgoingDataList.front();
		OutgoingData->AdvancePosition(mSocket->Write(OutgoingData->GetData(), OutgoingData->GetLengthLeft()));
		if(OutgoingData->GetLengthLeft())
		{
			return;
		}
		mOutgoingDataList.erase(mOutgoingDataList.begin());
	}
	if(mCommunicator)
		mCommunicator->CanSend(); // warning: may cause this to be destroyed
}

void cStreamedSocket2::OnClosed()
{
	mSocket.reset();
	if(mCommunicator)
		mCommunicator->StreamClosed(); // warning: may cause this to be destroyed
}

void cStreamedSocket2::SetLog(std::shared_ptr<cTextLog> Log)
{
	ASSERT(gNetworkThread->IsInThread());
	if(mSocket)
		mSocket->SetLog(Log);
	mLog=std::move(Log);
}