#include "pixie/system/commoninclude_system.h"
#include "pixie/pixie/i_pixie.h"


const char* ProgramName = "Pixie Sandbox";
const char* VersionString = "0.2";


cStartupController::cConfig pixieAppConfiguration()
{
    cStartupController::cConfig config;
    config.mainLuaConfigPath = "MainConfig.lua";
    return config;
}
