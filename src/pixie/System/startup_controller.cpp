#include "StdAfx.h"
#include "pixie/system/ConsoleCreator.h"
#include "pixie/system/lua/lua_global_config.h"
#include <filesystem>

tIntrusivePtr<cConfig> theGlobalConfig;

void registerGlobalPixieLuaFunctions(cLuaValue globalTable);

void cStartupController::Start_MainThread()
{
    mConfig = pixieAppConfiguration();
    if(mConfig.createConsole)
        CreateConsole();
    MainLog = new cMainLog(
        cLog::USE_MUTEX | cLog::FLUSH_OVER_TIME |
        (mConfig.mainLog.writeToConsole ? cLog::ECHO : 0) |
        (mConfig.mainLog.writeToFile ? 0 : cLog::NO_FILE) |
        (mConfig.mainLog.useTimeStamp ? cLog::TIME_STAMP : 0));
    MainLog->Log("----------- START -----------");
    MainLog->Log("working dir: \"%s\"", std::filesystem::current_path().string().c_str());

    theProgramDirector->Start();
    theMainThread->callback(
        [this]() 
        {
            continueStartup(); 
        }, eCallbackType::NoImmediate);
}


void cStartupController::continueStartup()
{
    cLuaScript::staticInit();

    auto script = std::make_shared<cLuaScript>();
    registerGlobalPixieLuaFunctions(script->globalTable());
    script->executeFile(mConfig.mainLuaConfigPath.empty() ? "MainConfig.lua" : mConfig.mainLuaConfigPath.c_str());
    theGlobalConfig = script->globalTable().toConfig();


    auto InstanceName = theGlobalConfig->get<std::string>("instance_name", std::string());
    if (!InstanceName.empty())
        cProgramTitle::Get()->SetInstanceName(InstanceName);
    cEventCenter::Get();

    if(mConfig.initPixie)
        InitPixieSystem();

    if(mConfig.startApplication)
        mConfig.startApplication();

// 	cMouseCursor::Get();
//     cMouseCursorServer::Get();
}

void cStartupController::start()
{
    new cThreadServer;
    cEventCenter::Get();
    theThreadServer->createMainThread();
    theMainThread->callback([this]() {Start_MainThread(); });
    Sleep(INFINITE);
}


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    //    gInstance = hInstance;
    cStartupController StartupController;
    StartupController.start();
    return 0;
}