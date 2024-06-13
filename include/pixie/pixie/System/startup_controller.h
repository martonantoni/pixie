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
        std::function<void()> stopApplication;
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
    cStartupController::cConfig mConfig;
    cRegisteredID mDestroyHandlerID;
public:
    [[noreturn]] void start();
};

cStartupController::cConfig pixieAppConfiguration();

