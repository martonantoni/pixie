#pragma once

extern const char* programName;
extern const char* versionString;

class cStartupController final
{
public:
    struct cConfig
    {
        cPath mainLuaConfigPath;
        std::function<void()> startApplication;
        bool initPixie = true;
#ifdef _CONSOLE
        bool createConsole = true;
#else
        bool createConsole = false;
#endif
    };
private:
    void continueStartup();
    void Start_MainThread();
    cStartupController::cConfig mConfig;
public:
    [[noreturn]] void start();
};

cStartupController::cConfig pixieAppConfiguration();

