#include "shaders_tests_common.h"

#include "pixie/pixie/i_pixie.h"

//////////////////////////////////////////////////////////////////////////

const char* ProgramName = "Pixie Shaders Test";
const char* VersionString = "1.0";

cStartupController::cConfig pixieAppConfiguration()
{
    cStartupController::cConfig config;
    config.initPixie = false;
    config.mainLog.writeToConsole = false;
    config.mainLog.writeToFile = false;
    config.startApplication =
        []()
        {
        };
    return config;
}

// Main function to run the tests
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}