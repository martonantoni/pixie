#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

const char* ProgramName = "Pixie Test";
const char* VersionString = "0.3";

TEST(threads, wait_callback)
{
    cThread testedThread("test");
    testedThread.Start();
    for (int i = 0; i < 100; ++i)
    {
        bool done = false;
        testedThread.callback([&done]() { done = true; }, eCallbackType::Wait);
        ASSERT_TRUE(done);
    }
}

// Main function to run the tests
int main(int argc, char** argv)
{
    cLuaState::staticInit();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}