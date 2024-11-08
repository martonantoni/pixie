#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

const char* ProgramName = "Pixie System Common Tests";
const char* VersionString = "1.0";

// Main function to run the tests
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}