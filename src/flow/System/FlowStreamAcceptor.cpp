#include "StdAfx.h"
#include "pixie/flow/i_flow.h"

cFlowStreamAcceptor::cFlowStreamAcceptor()
	: mAcceptor(make_intrusive_ptr<cTCPAcceptor>(this))
{
}

void cFlowStreamAcceptor::SetLog(std::shared_ptr<cTextLog> log)
{
	mAcceptor->SetLog(std::move(log));
}

void cFlowStreamAcceptor::OnNewSocket(std::unique_ptr<cSocket> NewSocket)
{
	auto Stream=new cFlowServerStream;
	Stream->AttachToSocket(std::move(NewSocket));
}

void cFlowStreamAcceptor::Init(unsigned short Port)
{
	mAcceptor->Listen(Port);
}