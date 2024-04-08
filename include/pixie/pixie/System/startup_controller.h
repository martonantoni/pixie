#pragma once

extern const char* programName;
extern const char* versionString;

class cStartupController final
{
public:
    struct cConfig2
    {
        cPath mainLuaConfigPath;
        std::function<void()> startApplication;
        bool initPixie = true;
#ifdef _CONSOLE
        bool createConsole = true;
#else
        bool createConsole = false;
#endif
        struct
        {
            bool useTimeStamp = true;
            bool writeToConsole = true;
            bool writeToFile = true;
        } mainLog;
    };
private:
    void continueStartup();
    void Start_MainThread();
    cStartupController::cConfig2 mConfig;
public:
    [[noreturn]] void start();
};

cStartupController::cConfig2 pixieAppConfiguration();

