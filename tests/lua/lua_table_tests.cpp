#include "pixie/system/commoninclude_system.h"
#include "pixie\system\ConsoleCreator.h"

const char* ProgramName = "Ready";
const char* VersionString = "0.2";


// class cLuaBasedConfig : public cConfig
// {
// public:
//     cLuaBasedConfig(std::shared_ptr<cLuaScript> script, int reference = LUA_NOREF);
//     virtual ~cLuaBasedConfig();
//     virtual void Set(const std::string& Key, int Value) override;
//     virtual void Set(const std::string& Key, bool Value) override;
//     virtual void Set(const std::string& Key, double Value) override;
//     virtual void Set(const std::string& Key, const std::string& Value) override;
//     virtual void Set(const std::string& Key, const char* Value) override;
//     virtual int GetInt(const std::string& Key, const tDefaultValue<int>& Default = tDefaultValue<int>()) const override;
//     virtual int GetInt(int Index, const tDefaultValue<int>& Default = tDefaultValue<int>()) const override;
//     virtual double GetDouble(const std::string& Key, const tDefaultValue<double>& Default = tDefaultValue<double>()) const override;
//     virtual double GetDouble(int Index, const tDefaultValue<double>& Default = tDefaultValue<double>()) const override;
//     virtual std::string GetString(const std::string& Key, const tDefaultValue<std::string>& Default = tDefaultValue<std::string>()) const override;
//     virtual std::string GetString(int Index, const tDefaultValue<std::string>& Default = tDefaultValue<std::string>()) const override;
//     virtual bool GetBool(const std::string& Key, const tDefaultValue<bool>& Default = tDefaultValue<bool>()) const override;
//     virtual bool GetBool(int Index, const tDefaultValue<bool>& Default = tDefaultValue<bool>()) const override;
//     virtual std::vector<std::string> GetKeys() const override;
//     virtual size_t GetValueCount() const override;
// 
//     //	virtual std::unique_ptr<cSubConfigMap> GetSubConfigMap() override;
// 
//     virtual size_t GetSubConfigCount() const override;
//     virtual tIntrusivePtr<cConfig> GetSubConfig(int Index) const override;
// 
//     static tIntrusivePtr<cConfig> CreateConfig(const char* MainTableKey, LuaState* State);
//     static tIntrusivePtr<cConfig> CreateConfig(LuaState* State);
//     static tIntrusivePtr<cConfig> CreateConfig(const cPath& Path);
//     static tIntrusivePtr<cConfig> CreateConfig(LuaObject Object);
// };
// 


class cStartupController
{
    void ContinueStartup();
    void Start_MainThread();
public:
    void Start();
};

void cStartupController::Start_MainThread()
{
    // #ifdef _DEBUG
    CreateConsole();
    // #endif
    MainLog = new cMainLog;
    MainLog->Log("----------- START -----------");

    theProgramDirector->Start();
    ::CallBack(theMainThread, eCallbackType::NoImmediate, this, &cStartupController::ContinueStartup);
}

namespace DaniBasic { void testDaniBasic(); }

void cStartupController::ContinueStartup()
{
    //     DaniBasic::testDaniBasic();
    //     exit(0);

//     InitLua();
//     LoadLUAFile("MainConfig.lua");
//     theMainConfig = cLuaBasedConfig::CreateConfig(theLuaState);
//     auto InstanceName = theMainConfig->GetString("instance_name", std::string());
//     if (!InstanceName.empty())
//         cProgramTitle::Get()->SetInstanceName(InstanceName);
//     cPrimaryWindow::Get();
// 
//     InitPixieSystem();
// 
//     //////////////////////////////////////////////////////////////////////////
//     //	cMouseCursor::Get();
//     cMouseCursorServer::Get();
//     DaniBasic::cLocalization::Get().init("eng");
// 
//     auto scenario = Ready::cScenario::fromFile("scenarios/hello_world.lua");
//     auto luaBridge = scenario->createScenarioLUABridge();
//     luaBridge->initScenario();
// 
// 
//     auto uiStateManager = std::make_unique<cUIStateManager>();
//     uiStateManager->Init();
//     uiStateManager->PushState(std::make_unique<Ready::cMainMenu>(*uiStateManager));
//     //    uiStateManager->PushState(std::make_unique<DaniBasic::cDevScreen>(*uiStateManager));
// 
//     uiStateManager.release();
}

void cStartupController::Start()
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
    StartupController.Start();
    return 0;
}