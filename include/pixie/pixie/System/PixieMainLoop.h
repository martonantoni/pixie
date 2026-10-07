#pragma once

class cPixieMainLoop
{
    cRegisteredID mRenderingTimerID;
    cRegisteredID mLogicID;
    void onLogic();
    bool mIsClosing = false;
    void mainLoop();
public:
    void init();
    void Close();
};