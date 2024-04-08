#include "pixie/system/commoninclude_system.h"
#include "pixie/pixie/i_pixie.h"


const char* ProgramName = "Pixie Sandbox";
const char* VersionString = "0.2";


cStartupController::cConfig2 pixieAppConfiguration()
{
    cStartupController::cConfig2 config;
    config.mainLuaConfigPath = "MainConfig.lua";
    return config;
}
