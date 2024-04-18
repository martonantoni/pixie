#include "event_tests_common.h"

namespace EventSystemTests
{




} // namespace EventSystemTests
//////////////////////////////////////////////////////////////////////////

const char* ProgramName = "Pixie Event System Test";
const char* VersionString = "0.1";

// Main function to run the tests
int main(int argc, char** argv)
{
    cLuaScript::staticInit();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}