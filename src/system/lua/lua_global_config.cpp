#include "StdAfx.h"

#include "pixie/system/lua/lua_global_config.h"

// std::shared_ptr<cConfig> theGlobalConfig;
// 
// void registerGlobalPixieLuaFunctions(cLuaObject globalTable);

// cLuaGlobalConfigLoader::cLuaGlobalConfigLoader()
// {
//     auto script = std::make_shared<cLuaState>();
// 
//     registerGlobalPixieLuaFunctions(script->globalTable());
// 
//     script->executeFile("MainConfig.lua");
//     theGlobalConfig = script->globalTable().toConfig();
// }

/*REGISTER_AUTO_SINGLETON_INIT(cLuaGlobalConfigLoader, eProgramPhases::StaticInit, DefaultInitLabel);*/