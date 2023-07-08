#include "pixie/system/commoninclude_system.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie\system\ConsoleCreator.h"

const char* ProgramName = "Pixie Sandbox";
const char* VersionString = "0.2";


cStartupController::cConfig pixieAppConfiguration()
{
    cStartupController::cConfig config;
    config.mainLuaConfigPath = "MainConfig.lua";
    return config;
}

// class cStartupController
// {
//     void ContinueStartup();
//     void Start_MainThread();
// public:
//     void Start();
// };
// 
// void cStartupController::Start_MainThread()
// {
//     // #ifdef _DEBUG
//     CreateConsole();
//     // #endif
//     MainLog = new cMainLog;
//     MainLog->Log("----------- START -----------");
// 
//     theProgramDirector->Start();
//     ::CallBack(theMainThread, eCallbackType::NoImmediate, this, &cStartupController::ContinueStartup);
// }
// 
// namespace DaniBasic { void testDaniBasic(); }
// 
// void cStartupController::ContinueStartup()
// {
// 
//     //     DaniBasic::testDaniBasic();
//     //     exit(0);
// 
// //     InitLua();
// //     LoadLUAFile("MainConfig.lua");
// //     theMainConfig = cLuaBasedConfig::CreateConfig(theLuaState);
// //     auto InstanceName = theMainConfig->GetString("instance_name", std::string());
// //     if (!InstanceName.empty())
// //         cProgramTitle::Get()->SetInstanceName(InstanceName);
// //     cPrimaryWindow::Get();
// // 
// //     InitPixieSystem();
// // 
// //     //////////////////////////////////////////////////////////////////////////
// //     //	cMouseCursor::Get();
// //     cMouseCursorServer::Get();
// //     DaniBasic::cLocalization::Get().init("eng");
// // 
// //     auto scenario = Ready::cScenario::fromFile("scenarios/hello_world.lua");
// //     auto luaBridge = scenario->createScenarioLUABridge();
// //     luaBridge->initScenario();
// // 
// // 
// //     auto uiStateManager = std::make_unique<cUIStateManager>();
// //     uiStateManager->Init();
// //     uiStateManager->PushState(std::make_unique<Ready::cMainMenu>(*uiStateManager));
// //     //    uiStateManager->PushState(std::make_unique<DaniBasic::cDevScreen>(*uiStateManager));
// // 
// //     uiStateManager.release();
// }
// 
// //void cStartupController::start(const cConfig& config);
// 
// void cStartupController::Start()
// {
//     new cThreadServer;
//     theMainThread = theThreadServer->GetThread("main", true);
//     ::CallBack(theMainThread, eCallbackType::Normal, this, &cStartupController::Start_MainThread);
//     Sleep(INFINITE);
// }


// int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
// {
//     //    gInstance = hInstance;
//     cStartupController StartupController;
//     StartupController.start();
//     return 0;
// }
