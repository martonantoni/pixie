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
    };
    void start();
};

cStartupController::cConfig pixieAppConfiguration();
