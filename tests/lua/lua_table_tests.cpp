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

void verifyTestVariableValues(const cLuaTable& table)
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

void verifyTestVariableTypes(const cLuaTable& table)
{
    ASSERT_TRUE(table.isType<int>("twelve"));
    ASSERT_FALSE(table.isType<std::string>("twelve"));
}


TEST(lua_table, get_set_in_global_table)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    storeTestVariables(globalTable);
    verifyTestVariableValues(globalTable);
// now if we retrieve it again, it should be still readable:
    verifyTestVariableValues(script->globalTable());
}

TEST(lua_table, get_set_in_local_table)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable localTable = script->createTable();
    storeTestVariables(localTable);
    verifyTestVariableValues(localTable);
}

TEST(lua_table, copy_table_obj)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable localTable = script->createTable();
    storeTestVariables(localTable);
    verifyTestVariableValues(localTable);
    cLuaTable copyOfTableObj = localTable;
    verifyTestVariableValues(copyOfTableObj);
}

TEST(lua_table, move_table_obj)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable localTable = script->createTable();
    storeTestVariables(localTable);
    verifyTestVariableValues(localTable);
    cLuaTable movedTableObj = std::move(localTable);
    verifyTestVariableValues(movedTableObj);
}

TEST(lua_table, is_type)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    storeTestVariables(globalTable);
    verifyTestVariableTypes(script->globalTable());
}

TEST(lua_table, create_sub_tables)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    cLuaTable subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);
    verifyTestVariableValues(subTable);
    verifyTestVariableValues(script->globalTable().subTable("mySubTable"));
}

TEST(lua_table, sub_table_access_with_get)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    cLuaTable subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);

    ASSERT_TRUE(globalTable.isType<cLuaTable>("mySubTable"s));

    cLuaTable subTable2 = globalTable.get<cLuaTable>("mySubTable"s);
    verifyTestVariableValues(subTable2);

}

TEST(lua_table, funcion_v_v)  // void(void)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    bool success = false;
    globalTable.registerFunction<void>("test"s, [&success]() ->void { success = true; });
    globalTable.callFunction("test"s);
    ASSERT_TRUE(success);
}

TEST(lua_table, function_v_iii)  // void(int,int,int)
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

TEST(lua_table, function_v_t)  // void(cLuaTable)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaTable globalTable = script->globalTable();
    int result = 0;
    globalTable.registerFunction<void, cLuaTable>("test"s,
        [&result](cLuaTable t)
        {
            result = t.get<int>("a") + t.get<int>("b");
        });

    cLuaTable parameterTable = script->createTable();
    parameterTable.set("a", 33);
    parameterTable.set("b", 44);
    globalTable.callFunction("test"s, std::move(parameterTable));

    ASSERT_EQ(result, 77);

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


TEST(lua_execute, simple_execute)
{
    auto script = std::make_shared<cLuaScript>();

    script->executeString(
        "apple = 42\n"
        "my_table = { apple = 22, pear = 33 }");

    auto globalTable = script->globalTable();
    ASSERT_EQ(globalTable.get<int>("apple"), 42);
    auto subTable = globalTable.get<cLuaTable>("my_table");
    ASSERT_EQ(subTable.get<int>("apple"), 22);
    ASSERT_EQ(subTable.get<int>("pear"), 33);
}

TEST(lua_execute, c_function_i_ii)
{
    auto script = std::make_shared<cLuaScript>();
    auto globalTable = script->globalTable();

    globalTable.registerFunction<int, int, int>("test"s,
        [](int r, int l)
        {
            return r + l;
        });
    script->executeString("result = test(1234, 4321)");

    ASSERT_EQ(globalTable.get<int>("result"), 5555);
}

TEST(lua_execute, c_function_t_t)
{
    auto script = std::make_shared<cLuaScript>();
    auto globalTable = script->globalTable();

    globalTable.registerFunction<cLuaTable, cLuaTable>("test"s,
        [](cLuaTable sourceTable)
        {
            auto resultTable = sourceTable.script().createTable();
            resultTable.set<int>("a", sourceTable.get<int>("a") + 11);
            resultTable.set<int>("b", sourceTable.get<int>("b") + 22);
            return resultTable;
        });

    script->executeString("result = test({ a = 7, b = 8})");

    auto result = globalTable.get<cLuaTable>("result");

    ASSERT_EQ(result.get<int>("a"), 18);
    ASSERT_EQ(result.get<int>("b"), 30);
}

// Main function to run the tests
int main(int argc, char** argv)
{
    cLuaScript::staticInit();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}