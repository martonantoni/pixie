#pragma once

extern const char* programName;
extern const char* versionString;

class cStartupController
{
    void ContinueStartup();
    void Start_MainThread();
public:
    struct cConfig
    {
        cPath mainLuaConfigPath;
        std::function<void()> startApplication;
    };
    void start();
};

cStartupController::cConfig pixieAppConfiguration();

