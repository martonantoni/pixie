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
        auto value = table.get<int>("twelve");
        ASSERT_EQ(value, 12) << "twelve";
    }
    {
        auto value = table.get<int>("ten");
        ASSERT_EQ(value, 10) << "ten";
    }
    {
        auto value = table.get<double>("twelve_and_half");
        ASSERT_EQ(value, 12.5) << "twelve_and_half";
    }
    {
        auto value = table.get<double>("ten_and_half");
        ASSERT_EQ(value, 10.5) << "ten_and_half";
    }
    {
        auto value = table.get<std::string>("twelve");
        ASSERT_STREQ(value.c_str(), "12") << "twelve";
    }
    {
        auto value = table.get<std::string>("ten");
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

TEST(lua_table, funcion_vv)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    bool success = false;
    globalTable.registerFunction<void>("test"s, [&success]() ->void { success = true; });
    globalTable.callFunction("test"s);
    ASSERT_TRUE(success);
}

TEST(lua_table, function_viii)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    int result = 0;
    globalTable.registerFunction<void, int, int, int>("test"s,
        [&result](int a, int b, int c)
        {
            result = a + b + c;
        });
    globalTable.callFunction("test"s, 10, 11, 12);
    ASSERT_EQ(result, 33);
}

void setupConfigTestVariables(cLuaScript& script)
{
    cLuaTable globalTable = script.globalTable();
    storeTestVariables(globalTable);
    cLuaTable subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);
}

void verifyConfigSingleLevel(const cConfig& config)
{
    {
        auto value = config.Get<int>("twelve", 0);
        ASSERT_EQ(value, 12) << "twelve";
    }
    {
        auto value = config.Get<int>("ten", 0);
        ASSERT_EQ(value, 10) << "ten";
    }
    {
        auto value = config.Get<double>("twelve_and_half", 0.0);
        ASSERT_EQ(value, 12.5) << "twelve_and_half";
    }
    {
        auto value = config.Get<double>("ten_and_half", 0.0);
        ASSERT_EQ(value, 10.5) << "ten_and_half";
    }
    {
        auto value = config.Get<std::string>("twelve", {});
        ASSERT_STREQ(value.c_str(), "12") << "twelve";
    }
    {
        auto value = config.Get<std::string>("ten", {});
        ASSERT_STREQ(value.c_str(), "10") << "ten";
    }

}

TEST(lua_table, toConfig_nonrecursive)
{
    auto script = std::make_shared<cLuaScript>();

    setupConfigTestVariables(*script);
    auto config = script->globalTable().toConfig(cLuaTable::IsRecursive::No);
    verifyConfigSingleLevel(*config);
}

TEST(lua_table, toConfig_recursive)
{
    auto script = std::make_shared<cLuaScript>();

    setupConfigTestVariables(*script);
    auto config = script->globalTable().toConfig(cLuaTable::IsRecursive::Yes);
    verifyConfigSingleLevel(*config);
    auto subConfig = config->GetSubConfig("mySubTable");
    ASSERT_TRUE(subConfig);
    verifyConfigSingleLevel(*subConfig);
}


// Main function to run the tests
int main(int argc, char** argv)
{
    cLuaScript::staticInit();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}