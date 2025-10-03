#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

class cPixieMainLoop
{
	void OnLogic();
public:
	void Init();
};

void cPixieMainLoop::Init()
{
	cRegisteredIDSink(theLogicServer.AddLogic([this]() { OnLogic(); }, cLogicServer::LogicOrders::messaging));
}

void cPixieMainLoop::OnLogic()
{
	theEventCenter->DispatchEvents();
	theMessageCenter.dispatch();
}

void InitPixieSystemMainLoop()
{
	(new cPixieMainLoop)->Init();
}