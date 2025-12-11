#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"
#include "pixie/system/configuration/config.h"

namespace lua_config_tests
{

TEST(config_from_lua, simple_table)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString(
        "a = 1\n"
        "b = 2\n"
        "c = \"hello\"\n"
        "d = true\n"
        "e = 3.14\n");

    cLuaObject globalTable = script->globalTable();
    auto config = Pixie::toConfig(globalTable); 
    ASSERT_EQ(config->get<int>("a", 0), 1);
    ASSERT_EQ(config->get<int>("b", 0), 2);
    ASSERT_EQ(config->get<std::string>("c", ""), "hello");
    ASSERT_EQ(config->get<bool>("d", false), true);
    ASSERT_EQ(config->get<double>("e", 0.0), 3.14);
}

TEST(config_from_lua, nested_table)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString(
        "sub_table_1 = { sub_table_11 = { a = 1, c = 2, d = \"hello\", e = true, f = 3.14 } }\n"
        "sub_table_2 = { sub_table_21 = { a = 3, c = 4, d = \"hi\", e = false, f = 6.28 } }\n");

    cLuaObject globalTable = script->globalTable();
    auto config = Pixie::toConfig(globalTable);
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
    auto script = std::make_shared<cLuaState>();
    script->executeString(
        "array = { 1, 2, 3, 4, 5 }\n");

    cLuaObject globalTable = script->globalTable();
    auto config = Pixie::toConfig(globalTable);
    auto testedConfig = config->getSubConfig("array");
    ASSERT_TRUE(testedConfig->isArray());
    ASSERT_EQ(testedConfig->get<int>(0), 1);
    ASSERT_EQ(testedConfig->get<int>(1), 2);
    ASSERT_EQ(testedConfig->get<int>(2), 3);
    ASSERT_EQ(testedConfig->get<int>(3), 4);
    ASSERT_EQ(testedConfig->get<int>(4), 5);
}

void configToLuaScript_simple_table(const cLuaState::cConfigToScriptStyle& style)
{
    cConfig config;
    config.set("a", 1);
    config.set("b", 2);
    config.set("c", "hello");
    config.set("d", true);
    config.set("e", 3.14);

    auto scriptifiedConfig = cLuaState::configToScript(config, style);
    if (style.singleLine)
    {
        EXPECT_TRUE(scriptifiedConfig.find('\n') == std::string::npos);
    }
    //printf("%s\n", scriptifiedConfig.c_str());
    auto script = std::make_shared<cLuaState>();
    script->executeString(scriptifiedConfig);

    cLuaObject globalTable = script->globalTable();
    ASSERT_EQ(globalTable.get<int>("a"), 1);
    ASSERT_EQ(globalTable.get<int>("b"), 2);
    ASSERT_EQ(globalTable.get<std::string>("c"), "hello");
    ASSERT_EQ(globalTable.get<bool>("d"), true);
    ASSERT_EQ(globalTable.get<double>("e"), 3.14);
}

TEST(config_to_lua_script, simple_table)
{
    configToLuaScript_simple_table({ .singleLine = false });
}

TEST(config_to_lua_script, simple_table__single_line)
{
    configToLuaScript_simple_table({ .singleLine = true });
}

void configToLuaScript_nestedTable(const cLuaState::cConfigToScriptStyle& style)
{
    cConfig config;
    {
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
    }

    auto scriptifiedConfig = cLuaState::configToScript(config, style);
    if (style.singleLine)
    {
        EXPECT_TRUE(scriptifiedConfig.find('\n') == std::string::npos);
    }
    //printf("%s\n", scriptifiedConfig.c_str());
    auto script = std::make_shared<cLuaState>();
    script->executeString(scriptifiedConfig);

    {
        cLuaObject globalTable = script->globalTable();
        cLuaObject subTable1 = globalTable.get<cLuaObject>("sub_table_1");
        ASSERT_EQ(subTable1.get<int>("a"), 1);
        ASSERT_EQ(subTable1.get<int>("b"), 2);
        ASSERT_EQ(subTable1.get<std::string>("c"), "hello");
        ASSERT_EQ(subTable1.get<bool>("d"), true);
        ASSERT_EQ(subTable1.get<double>("e"), 3.14);
        cLuaObject subTable11 = subTable1.get<cLuaObject>("sub_table_11");
        ASSERT_EQ(subTable11.get<int>("a"), 3);
        ASSERT_EQ(subTable11.get<int>("b"), 4);
        ASSERT_EQ(subTable11.get<std::string>("c"), "hi");
        ASSERT_EQ(subTable11.get<bool>("d"), false);
        ASSERT_EQ(subTable11.get<double>("e"), 6.28);
    }
}

TEST(config_to_lua_script, nested_tables)
{
    configToLuaScript_nestedTable({ .singleLine = false });
}

TEST(config_to_lua_script, nested_tables__single_line)
{
    configToLuaScript_nestedTable({ .singleLine = false });
}

void configToLuaScript_array(const cLuaState::cConfigToScriptStyle& style)
{
    cConfig config;
    auto subConfig = config.createSubConfig("my_array");
    subConfig->makeArray();
    subConfig->push(1);
    subConfig->push(2);
    subConfig->push("hello");
    subConfig->push(true);
    subConfig->push(3.14);

    auto scriptifiedConfig = cLuaState::configToScript(config, style);
    if (style.singleLine)
    {
        EXPECT_TRUE(scriptifiedConfig.find('\n') == std::string::npos);
    }
    //printf("%s\n", scriptifiedConfig.c_str());
    auto script = std::make_shared<cLuaState>();
    script->executeString(scriptifiedConfig);

    cLuaObject globalTable = script->globalTable();
    cLuaObject subTable = globalTable.get<cLuaObject>("my_array");
    ASSERT_EQ(subTable.get<int>(1), 1);
    ASSERT_EQ(subTable.get<int>(2), 2);
    ASSERT_EQ(subTable.get<std::string>(3), "hello");
    ASSERT_EQ(subTable.get<bool>(4), true);
    ASSERT_EQ(subTable.get<double>(5), 3.14);
}

TEST(config_to_lua_script, array)
{
    configToLuaScript_array({ .singleLine = false });
}

TEST(config_to_lua_script, array__single_line)
{
    configToLuaScript_array({ .singleLine = true });
}

TEST(config_to_lua_script, array_on_global_is_error)
{
    cConfig config;
    config.makeArray();
    config.push(1);
    config.push(2);
    config.push("hello");
    config.push(true);
    config.push(3.14);

    ASSERT_THROW(cLuaState::configToScript(config), std::runtime_error);
}

} // namespace lua_config_tests