#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

void cPixieMainLoop::init()
{
    mRenderingTimerID = theMainThread->AddTimer([this]() { mainLoop(); }, cTimerRequest(10));
    mLogicID = theLogicServer.AddLogic([this]() { onLogic(); }, cLogicServer::LogicOrders::messaging);
}

void cPixieMainLoop::onLogic()
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

    thePixieDesktop.render();

    for (auto& window : thePixieDesktop.ownerlessWindows())
        window->CheckOwnerlessSprites();

    theDevice->present();
}
