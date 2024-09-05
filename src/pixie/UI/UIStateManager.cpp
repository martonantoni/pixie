#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cPixieWindow* cUIState::GetWindow() const
{
    return mUIStateManager.GetWindow();
}

void cUIStateManager::Init()
{
    mWindow = std::make_unique<cPixieWindow>();
    cPixieWindow::cInitData InitData;
    InitData.mParentWindow = &thePixieDesktop;
    InitData.mPlacement = thePixieDesktop.GetPlacement();
    mWindow->Init(InitData);

    mBackgroundSprite = std::make_unique<cSprite>();
    mBackgroundSprite->SetWindow(GetWindow());
    mBackgroundSprite->SetTexture(theTextureManager.GetTexture("ui_background"));
    mBackgroundSprite->SetSize(GetWindow()->GetClientRect().size());
    mBackgroundSprite->SetPosition(0, 0);
    mBackgroundSprite->SetZOrder(1);
    mBackgroundSprite->Show();
}

cPixieWindow* cUIStateManager::GetWindow() const
{
    return mWindow.get();
}

void cUIStateManager::Hide()
{
    mWindow->SetParentWindow(nullptr);
}

void cUIStateManager::Show()
{
    mWindow->SetParentWindow(&thePixieDesktop);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cUIStateManager::EnterState(cUIState& State)
{
    State.Enter();
    mBackgroundSprite->SetVisible(!State.HasOwnBackground());
}

void cUIStateManager::PopState()
{
    mStateStack.back()->Leave();
    mStateStack.pop_back();
    if (ASSERTTRUE(!mStateStack.empty()))
    {
        EnterState(*mStateStack.back());
    }
}

void cUIStateManager::PopStateN(int StatesToPop)
{
    if (StatesToPop == 0)
        return;
    mStateStack.back()->Leave();
    for (int i = 0; i < StatesToPop; ++i)
    {
        mStateStack.pop_back();
    }
    if (ASSERTTRUE(!mStateStack.empty()))
    {
        EnterState(*mStateStack.back());
    }
}

bool cUIStateManager::PopState_Safe()
{
    if (mStateStack.size() > 1)
    {
        PopState();
        return true;
    }
    return false;
}

void cUIStateManager::PushState(std::unique_ptr<cUIState> State)
{
    if (!mStateStack.empty())
        mStateStack.back()->Leave();
    mStateStack.emplace_back(std::move(State));
    EnterState(*mStateStack.back());
}

void cUIStateManager::ReplaceTopState(std::unique_ptr<cUIState> State)
{
    if (ASSERTTRUE(!mStateStack.empty()))
    {
        mStateStack.back()->Leave();
        mStateStack.pop_back();  // popping without calling Enter on the state bellow
    }
    PushState(std::move(State));
}
