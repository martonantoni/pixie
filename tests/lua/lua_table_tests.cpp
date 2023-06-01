#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

void storeTestVariables(cLuaTable& table)
{
    table.set("twelve", 12);
    table.set("ten", 10);
    table.set("twelve_string", "12"s);
    table.set("ten_string", "10"s);
    table.set("twelve_and_half", 12.5);
    table.set("ten_and_half", 10.5);
}

void verifyTestVariables(const cLuaTable& table)
{
    {
        auto [value, success] = table.get<int>("twelve");
        ASSERT_TRUE(success) << "twelve";;
        ASSERT_EQ(value, 12) << "twelve";
    }
    {
        auto [value, success] = table.get<int>("ten");
        ASSERT_TRUE(success) << "ten";
        ASSERT_EQ(value, 10) << "ten";
    }
    {
        auto [value, success] = table.get<double>("twelve_and_half");
        ASSERT_TRUE(success) << "twelve_and_half";
        ASSERT_EQ(value, 12.5) << "twelve_and_half";
    }
    {
        auto [value, success] = table.get<double>("ten_and_half");
        ASSERT_TRUE(success);
        ASSERT_EQ(value, 10.5) << "ten_and_half";
    }
    {
        auto [value, success] = table.get<std::string>("twelve");
        ASSERT_TRUE(success);
        ASSERT_STREQ(value.c_str(), "12") << "twelve";
    }
    {
        auto [value, success] = table.get<std::string>("ten");
        ASSERT_TRUE(success);
        ASSERT_STREQ(value.c_str(), "10") << "ten";
    }
}

TEST(lua_table, get_set_in_global_table)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    storeTestVariables(globalTable);
    verifyTestVariables(globalTable);
// now if we retrieve it again, it should be still readable:
    verifyTestVariables(script->globalTable());
}

TEST(lua_table, create_sub_tables)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    cLuaTable subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);
    verifyTestVariables(subTable);
    verifyTestVariables(script->globalTable().subTable("mySubTable"));
}

// Main function to run the tests
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}