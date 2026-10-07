#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

//cRootWindow* mMainRenderer = nullptr;

class cPixieMainLoop
{
    cRegisteredID mRenderingTimerID;
    cRegisteredID mLogicID;
	void OnLogic();
    bool mIsClosing = false;
    void mainLoop();
public:
	void Init();
    void Close();

};

void cPixieMainLoop::Init()
{
    mRenderingTimerID = theMainThread->AddTimer([this]() { mainLoop(); }, cTimerRequest(10));
    mLogicID = theLogicServer.AddLogic([this]() { OnLogic(); }, cLogicServer::LogicOrders::messaging);
}

void cPixieMainLoop::OnLogic()
{
	theEventCenter->DispatchEvents();
	theMessageCenter.dispatch();
}

void cPixieMainLoop::Close()
{
    mIsClosing = true;
}

void cPixieMainLoop::mainLoop()
{
    if (mIsClosing)
        return;

    theLogicServer.Tick();

    theRenderers.Call();

    theDevice->clearDevice();

//    if (thePixieDesktop)
    thePixieDesktop.render();

    for (auto& window : thePixieDesktop.ownerlessWindows())
        window->CheckOwnerlessSprites();

    theDevice->present();
}



void InitPixieSystemMainLoop()
{
	(new cPixieMainLoop)->Init();
}