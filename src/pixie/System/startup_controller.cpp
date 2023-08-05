#include "StdAfx.h"
#include "pixie/system/ConsoleCreator.h"
#include "pixie/system/lua/lua_global_config.h"
#include <filesystem>

// const char* ProgramName = "Pixie";
// const char* VersionString = "0.2";


tIntrusivePtr<cConfig> theGlobalConfig;

void registerGlobalPixieLuaFunctions(cLuaValue globalTable);

void cStartupController::Start_MainThread()
{
    // #ifdef _DEBUG
    CreateConsole();
    // #endif
    MainLog = new cMainLog;
    MainLog->Log("----------- START -----------");
    MainLog->Log("working dir: \"%s\"", std::filesystem::current_path().string().c_str());

    theProgramDirector->Start();
    ::CallBack(theMainThread, eCallbackType::NoImmediate, this, &cStartupController::ContinueStartup);
}


void cStartupController::ContinueStartup()
{
    cStartupController::cConfig config = pixieAppConfiguration();
    cLuaScript::staticInit();

    auto script = std::make_shared<cLuaScript>();
    registerGlobalPixieLuaFunctions(script->globalTable());
    script->executeFile(config.mainLuaConfigPath.empty() ? "MainConfig.lua" : config.mainLuaConfigPath.c_str());
    theGlobalConfig = script->globalTable().toConfig();


    auto InstanceName = theGlobalConfig->GetString("instance_name", std::string());
    if (!InstanceName.empty())
        cProgramTitle::Get()->SetInstanceName(InstanceName);
    cPrimaryWindow::Get();

    InitPixieSystem();

    if(config.startApplication)
        config.startApplication();

// 	cMouseCursor::Get();
//     cMouseCursorServer::Get();
}

void cStartupController::start()
{
    new cThreadServer;
    theMainThread = theThreadServer->GetThread("main", true);
    ::CallBack(theMainThread, eCallbackType::Normal, this, &cStartupController::Start_MainThread);
    Sleep(INFINITE);
}


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    //    gInstance = hInstance;
    cStartupController StartupController;
    StartupController.start();
    return 0;
}