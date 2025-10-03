#include "StdAfx.h"
#include "pixie/system/ConsoleCreator.h"
#include <filesystem>

std::shared_ptr<cConfig> theGlobalConfig;

void registerGlobalPixieLuaFunctions(cLuaObject globalTable);

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
    auto luaState = std::make_shared<cLuaState>();
    registerGlobalPixieLuaFunctions(luaState->globalTable());
    luaState->executeFile(mConfig.mainLuaConfigPath.empty() ? "MainConfig.lua" : mConfig.mainLuaConfigPath.c_str());
    theGlobalConfig = luaState->globalTable().toConfig();


    auto InstanceName = theGlobalConfig->get<std::string>("instance_name", std::string());
    if (!InstanceName.empty())
        cProgramTitle::Get()->SetInstanceName(InstanceName);
    cEventCenter::Get();

    if(mConfig.initPixie)
        InitPixieSystem();

    if (mConfig.stopApplication)
    {
        mDestroyHandlerID = cPrimaryWindow::get().AddMessageHandler(WM_DESTROY, [this](WPARAM wParam, LPARAM lParam)
            {
                if (mConfig.stopApplication)
                    mConfig.stopApplication();
                return cWindowsMessageResult();
            });
    }

    if(mConfig.startApplication)
        mConfig.startApplication();

    theRedrawableDispatcher.init();

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
    printf("Pixie System initialization....\n");
    //    gInstance = hInstance;
//    SetProcessDPIAware();
  //  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
    cStartupController StartupController;
    StartupController.start();
    return 0;
}