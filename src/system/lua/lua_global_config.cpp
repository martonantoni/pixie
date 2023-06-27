#include "StdAfx.h"

#include "lua_global_config.h"

cLuaGlobalConfigLoader::cLuaGlobalConfigLoader()
{
    auto script = std::make_shared<cLuaScript>();
    script->executeFile("MainConfig.lua");
    theGlobalConfig = script->globalTable().toConfig();
}

REGISTER_AUTO_SINGLETON_INIT(cLuaGlobalConfigLoader, eProgramPhases::StaticInit, DefaultInitLabel);