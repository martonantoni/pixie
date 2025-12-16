#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"
#include "pixie/system/configuration/config.h"

namespace lua_serialize_tests
{

TEST(serialize_lua_with_execute, global_table)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString(
        "a = 1\n"
        "b = 2\n"
        "c = \"hello\"\n"
        "d = true\n"
        "e = 3.14\n");

    cLuaObject globalTable = script->globalTable();
    auto serialized = cLuaState::objectToScript(globalTable);

    auto script2 = std::make_shared<cLuaState>();
    script2->executeString(serialized);
    cLuaObject globalTable2 = script2->globalTable();
    ASSERT_EQ(globalTable2.get<int>("a"), 1);
    ASSERT_EQ(globalTable2.get<int>("b"), 2);
    ASSERT_EQ(globalTable2.get<std::string>("c"), "hello");
    ASSERT_EQ(globalTable2.get<bool>("d"), true);
    ASSERT_EQ(globalTable2.get<double>("e"), 3.14);
}

TEST(serialize_deserialize, single_values)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString(
        "a = 1\n"
        "b = 2\n"
        "c = \"hello\"\n"
        "d = true\n"
        "e = 3.14\n");

    cLuaObject globalTable = script->globalTable();
    auto serialized_a = cLuaState::objectToScript(globalTable.get<cLuaObject>("a"));
    auto serialized_b = cLuaState::objectToScript(globalTable.get<cLuaObject>("b"));
    auto serialized_c = cLuaState::objectToScript(globalTable.get<cLuaObject>("c"));
    auto serialized_d = cLuaState::objectToScript(globalTable.get<cLuaObject>("d"));
    auto serialized_e = cLuaState::objectToScript(globalTable.get<cLuaObject>("e"));

    auto a = script->createObject();
    a.deserialize(serialized_a);
    ASSERT_TRUE(a.isType<int>());
    ASSERT_EQ(a.toInt(), 1);

    auto b = script->createObject();
    b.deserialize(serialized_b);
    ASSERT_TRUE(b.isType<int>());
    ASSERT_EQ(b.toInt(), 2);

    auto c = script->createObject();
    c.deserialize(serialized_c);
    ASSERT_TRUE(c.isType<std::string>());
    ASSERT_EQ(c.toString(), "hello");

    auto d = script->createObject();
    d.deserialize(serialized_d);
    ASSERT_TRUE(d.isType<bool>());
    ASSERT_EQ(d.toInt(), true);
    ASSERT_EQ(d.toBool(), true);

    auto e = script->createObject();
    e.deserialize(serialized_e);
    ASSERT_TRUE(e.isType<double>());
    ASSERT_EQ(e.toDouble(), 3.14);
}

TEST(serialize_deserialize, table)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString(
        "my_table = {\n"
        "  a = 1,\n"
        "  b = 2,\n"
        "  c = \"hello\",\n"
        "  d = true,\n"
        "  e = 3.14\n"
        "}\n");
    cLuaObject globalTable = script->globalTable();
    cLuaObject myTable = globalTable.get<cLuaObject>("my_table");
    auto serialized_table = cLuaState::objectToScript(myTable);
    auto table2 = script->createObject();
    table2.deserialize(serialized_table);
    ASSERT_TRUE(table2.isType<cLuaObject>());
    ASSERT_EQ(table2.get<int>("a"), 1);
    ASSERT_EQ(table2.get<int>("b"), 2);
    ASSERT_EQ(table2.get<std::string>("c"), "hello");
    ASSERT_EQ(table2.get<bool>("d"), true);
    ASSERT_EQ(table2.get<double>("e"), 3.14);
}

TEST(serialize_deserialize, multi_level_table)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString(
        R"script(
        my_table = {
          a = 1,
          b = 2,
          c = {
            d = "hello",
            e = {
              f = true,
              g = 3.14
            }
          }
        }
        )script");

    cLuaObject globalTable = script->globalTable();
    cLuaObject myTable = globalTable.get<cLuaObject>("my_table");
    auto serialized_table = cLuaState::objectToScript(myTable);

    auto table2 = script->createObject();
    table2.deserialize(serialized_table);

    ASSERT_TRUE(table2.isTable());
    ASSERT_EQ(table2.get<int>("a"), 1);
    ASSERT_EQ(table2.get<int>("b"), 2);
    auto c = table2.get("c");
    ASSERT_TRUE(c.isTable());
    ASSERT_EQ(c.get<std::string>("d"), "hello");
    auto e = c.get("e");
    ASSERT_TRUE(e.isTable());
    ASSERT_EQ(e.get<bool>("f"), true);
    ASSERT_EQ(e.get<double>("g"), 3.14);
}


} // namespace lua_serialize_tests