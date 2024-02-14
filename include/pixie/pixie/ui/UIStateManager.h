#pragma once

class cUIStateManager;

class cUIState
{
protected:
    cUIStateManager& mUIStateManager;
    cPixieWindow* GetWindow() const;
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
    template<class T> void MakeAndPushState() { PushState(std::make_unique<T>(*this)); }
    void ReplaceTopState(std::unique_ptr<cUIState> State);
    void PopState();
    void PopStateN(int StatesToPop);
    bool PopState_Safe();
    int getStateStackSize() const { return static_cast<int>(mStateStack.size()); }

    void Hide();
    void Show();

    cPixieWindow* GetWindow() const;
};