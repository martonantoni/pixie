#pragma once

class cUIStateManager;

class cUIState
{
protected:
    cUIStateManager& mUIStateManager;
public:
    cUIState(cUIStateManager& UIStateManager) : mUIStateManager(UIStateManager) {}
    virtual ~cUIState() = default;
    virtual void Enter() {}
    virtual void Leave() {}
    virtual bool HasOwnBackground() const { return false; }
};

class cUIStateManager
{
    std::vector<std::unique_ptr<cUIState>> mStateStack;
    std::unique_ptr<cSprite> mBackgroundSprite;
    std::unique_ptr<cPixieWindow> mWindow;
    void EnterState(cUIState& State);
public:
    void Init();
    void PushState(std::unique_ptr<cUIState> State);
    void ReplaceTopState(std::unique_ptr<cUIState> State);
    void PopState();
    bool PopState_Safe();

    void Hide();
    void Show();

    cPixieWindow* GetWindow();
};