#include "StdAfx.h"

#include "pixie/system/lua/lua_global_config.h"

// tIntrusivePtr<cConfig> theGlobalConfig;
// 
// void registerGlobalPixieLuaFunctions(cLuaValue globalTable);

// cLuaGlobalConfigLoader::cLuaGlobalConfigLoader()
// {
//     auto script = std::make_shared<cLuaScript>();
// 
//     registerGlobalPixieLuaFunctions(script->globalTable());
// 
//     script->executeFile("MainConfig.lua");
//     theGlobalConfig = script->globalTable().toConfig();
// }

/*REGISTER_AUTO_SINGLETON_INIT(cLuaGlobalConfigLoader, eProgramPhases::StaticInit, DefaultInitLabel);*/