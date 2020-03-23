#include "StdAfx.h"

class cPixieMainLoop
{
	void OnLogic();
public:
	void Init();
};

void cPixieMainLoop::Init()
{
	cRegisteredIDSink(theLogicServer.AddLogic([this]() { OnLogic(); }, 300));
}

void cPixieMainLoop::OnLogic()
{
	theEventCenter->DispatchEvents();
}

void InitPixieSystemMainLoop()
{
	(new cPixieMainLoop)->Init();
}