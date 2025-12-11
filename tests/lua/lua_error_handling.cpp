#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

TEST(lua_error_handling, lua_syntax_error__executeString)
{
    auto script = std::make_shared<cLuaState>();

    try
    {
        script->executeString("function test()\n"
            "  print('Hello World'\n" // missing closing parenthesis
            "end");
        FAIL() << "did not throw";
    }
    catch (const cLuaException& e)
    {
        std::string errorMessage = e.what();
        EXPECT_NE(errorMessage.find("at line 2"), std::string::npos);
    }
    catch(...)
    {
        FAIL() << "threw wrong exception type";
    }
}

