#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"
#include "pixie/system/configuration/config2.h"

namespace lua_config_tests
{

TEST(config_from_lua, simple_table)
{
    auto script = std::make_shared<cLuaScript>();
    script->executeString(
        "a = 1\n"
        "b = 2\n"
        "c = \"hello\"\n"
        "d = true\n"
        "e = 3.14\n");

    cLuaValue globalTable = script->globalTable();
    auto config = globalTable.toConfig2();
    ASSERT_EQ(config->get<int>("a", 0), 1);
    ASSERT_EQ(config->get<int>("b", 0), 2);
    ASSERT_EQ(config->get<std::string>("c", ""), "hello");
    ASSERT_EQ(config->get<bool>("d", false), true);
    ASSERT_EQ(config->get<double>("e", 0.0), 3.14);
}

TEST(config_from_lua, nested_table)
{
    auto script = std::make_shared<cLuaScript>();
    script->executeString(
        "sub_table_1 = { sub_table_11 = { a = 1, c = 2, d = \"hello\", e = true, f = 3.14 } }\n"
        "sub_table_2 = { sub_table_21 = { a = 3, c = 4, d = \"hi\", e = false, f = 6.28 } }\n");

    cLuaValue globalTable = script->globalTable();
    auto config = globalTable.toConfig2();
    ASSERT_EQ(config->get<int>("sub_table_1.sub_table_11.a"), 1);
    ASSERT_EQ(config->get<int>("sub_table_1.sub_table_11.c"), 2);
    ASSERT_EQ(config->get<std::string>("sub_table_1.sub_table_11.d"), "hello");
    ASSERT_EQ(config->get<bool>("sub_table_1.sub_table_11.e"), true);
    ASSERT_EQ(config->get<double>("sub_table_1.sub_table_11.f"), 3.14);
    ASSERT_EQ(config->get<int>("sub_table_2.sub_table_21.a"), 3);
    ASSERT_EQ(config->get<int>("sub_table_2.sub_table_21.c"), 4);
    ASSERT_EQ(config->get<std::string>("sub_table_2.sub_table_21.d"), "hi");
}

TEST(config_from_lua, array)
{
    auto script = std::make_shared<cLuaScript>();
    script->executeString(
        "array = { 1, 2, 3, 4, 5 }\n");

    cLuaValue globalTable = script->globalTable();
    auto config = globalTable.toConfig2();
    auto testedConfig = config->getSubConfig("array");
    ASSERT_TRUE(testedConfig->isArray());
    ASSERT_EQ(testedConfig->get<int>(0), 1);
    ASSERT_EQ(testedConfig->get<int>(1), 2);
    ASSERT_EQ(testedConfig->get<int>(2), 3);
    ASSERT_EQ(testedConfig->get<int>(3), 4);
    ASSERT_EQ(testedConfig->get<int>(4), 5);
}

TEST(config_to_lua_script, simple_table)
{
    cConfig2 config;
    config.set("a", 1);
    config.set("b", 2);
    config.set("c", "hello");
    config.set("d", true);
    config.set("e", 3.14);

    auto script = std::make_shared<cLuaScript>();
    script->executeString(cLuaScript::configToScript(config));

    cLuaValue globalTable = script->globalTable();
    ASSERT_EQ(globalTable.get<int>("a"), 1);
    ASSERT_EQ(globalTable.get<int>("b"), 2);
    ASSERT_EQ(globalTable.get<std::string>("c"), "hello");
    ASSERT_EQ(globalTable.get<bool>("d"), true);
    ASSERT_EQ(globalTable.get<double>("e"), 3.14);
}

TEST(config_to_lua_script, nested_tables)
{
    cConfig2 config;
    auto subTable1 = config.createSubConfig("sub_table_1");
    subTable1->set("a", 1);
    subTable1->set("b", 2);
    subTable1->set("c", "hello");
    subTable1->set("d", true);
    subTable1->set("e", 3.14);
    auto subTable11 = subTable1->createSubConfig("sub_table_11");
    subTable11->set("a", 3);
    subTable11->set("b", 4);
    subTable11->set("c", "hi");
    subTable11->set("d", false);
    subTable11->set("e", 6.28);

    auto script = std::make_shared<cLuaScript>();
    script->executeString(cLuaScript::configToScript(config));

    cLuaValue globalTable = script->globalTable();
    ASSERT_EQ(globalTable.get<int>("sub_table_1.sub_table_11.a"), 3);
    ASSERT_EQ(globalTable.get<int>("sub_table_1.sub_table_11.b"), 4);
    ASSERT_EQ(globalTable.get<std::string>("sub_table_1.sub_table_11.c"), "hi");
    ASSERT_EQ(globalTable.get<bool>("sub_table_1.sub_table_11.d"), false);
    ASSERT_EQ(globalTable.get<double>("sub_table_1.sub_table_11.e"), 6.28);
}


} // namespace lua_config_tests