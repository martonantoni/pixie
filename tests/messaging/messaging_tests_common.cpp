#include "messaging_tests_common.h"

namespace MessageSystemTests
{




} // namespace EventSystemTests
//////////////////////////////////////////////////////////////////////////

const char* ProgramName = "Pixie Message System Test";
const char* VersionString = "1.0";

// Main function to run the tests
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}