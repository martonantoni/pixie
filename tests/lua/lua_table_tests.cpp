#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

TEST(lua_table, set_int)
{
    auto script = std::make_shared< cLuaScript> ();

    cLuaTable globalTable = script->globalTable();

    globalTable.set("alma", 12);
    auto [value, success] = globalTable.get<int>("alma");
    ASSERT_TRUE(success);
    ASSERT_EQ(value, 12);
}

// Main function to run the tests
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}