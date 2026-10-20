#include "StdAfx.h"
#include "pixie/system/ConsoleCreator.h"
#include <filesystem>

std::shared_ptr<cConfig> theGlobalConfig;
std::filesystem::path theConfigurationFolder;

void registerGlobalPixieLuaFunctions(cLuaObject globalTable);

void cStartupController::Start_MainThread()
{
    mConfig = pixieAppConfiguration();
    theConfigurationFolder =
        mConfig.configurationFolder.empty() ? std::filesystem::current_path() : mConfig.configurationFolder;
    if(mConfig.createConsole)
        CreateConsole();
    MainLog = new cMainLog(
        cLog::Flags::USE_MUTEX | cLog::Flags::FLUSH_OVER_TIME |
        (mConfig.mainLog.writeToConsole ? cLog::Flags::ECHO : 0) |
        (mConfig.mainLog.writeToFile ? 0 : cLog::Flags::NO_FILE) |
        (mConfig.mainLog.useTimeStamp ? cLog::Flags::TIME_STAMP : 0));
    MainLog->Log("----------- START -----------");
    MainLog->Log("working dir: \"{}\"", std::filesystem::current_path().string());

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
    luaState->executeFile(mConfig.mainLuaConfigPath.empty() ? theConfigurationFolder / "MainConfig.lua" : mConfig.mainLuaConfigPath.c_str());
    theGlobalConfig = Pixie::toConfig(luaState->globalTable());


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

#ifdef _DEBUG
    mConfigChangeNotifier = std::make_unique<Pixie::cFileChangeNotifier>("configuration", "*.lua",
        []()
        {
            theMessageCenter.post("pixie.config_file_changed");
            theMessageCenter.post("pixie.configuration_changed");
        });
#endif
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
    cStartupController StartupController;
    StartupController.start();
    return 0;
}