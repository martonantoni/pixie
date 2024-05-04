#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

const char* ProgramName = "Pixie Test";
const char* VersionString = "0.3";


void storeTestVariables(cLuaValue& table)
{
    table.set("twelve", 12);
    table.set("ten", 10);
    table.set("twelve_string", "12"s);
    table.set("ten_string", "10"s);
    table.set("twelve_and_half", 12.5);
    table.set("ten_and_half", 10.5);
}

void verifyTestVariableValues(const cLuaValue& table)
{
    {
        auto value = *table.get<int>("twelve");
        ASSERT_EQ(value, 12) << "twelve";
    }
    {
        auto value = *table.get<int>("ten");
        ASSERT_EQ(value, 10) << "ten";
    }
    {
        auto value = *table.get<double>("twelve_and_half");
        ASSERT_EQ(value, 12.5) << "twelve_and_half";
    }
    {
        auto value = *table.get<double>("ten_and_half");
        ASSERT_EQ(value, 10.5) << "ten_and_half";
    }
    {
        auto value = *table.get<std::string>("twelve");
        ASSERT_STREQ(value.c_str(), "12") << "twelve";
    }
    {
        auto value = *table.get<std::string>("ten");
        ASSERT_STREQ(value.c_str(), "10") << "ten";
    }
}

void verifyTestVariableTypes(const cLuaValue& table)
{
    ASSERT_TRUE(table.isType<int>("twelve"));
    ASSERT_FALSE(table.isType<std::string>("twelve"));
}
/*
volatile int perf_test_sink;

TEST(lua_table, perf)
{
    auto script = std::make_shared<cLuaScript>();
    {
        auto globalTable = script->globalTable();
        globalTable.set<int>("alma"s, 1);
    }
    {
        cPrintfPerformance_Guard perf("global_table_acc");
        for (int i = 0; i < 1'000'000; ++i)
        {
            auto globalTable = script->globalTable();
        }
    }
    {
        cPrintfPerformance_Guard perf("table_get");
        auto globalTable = script->globalTable();
        for (int i = 0; i < 1'000'000; ++i)
        {
            perf_test_sink += globalTable.get<int>("alma"s);
        }
    }

    {
        cPrintfPerformance_Guard perf("global_table_acc_and_get");
        for (int i = 0; i < 1'000'000; ++i)
        {
            auto globalTable = script->globalTable();
            perf_test_sink += globalTable.get<int>("alma"s);
        }
    }


    {
        cPrintfPerformance_Guard perf("register_get");
        auto globalTable = script->globalTable();
        for (int i = 0; i < 1'000'000; ++i)
        {
            auto subValue = globalTable.subTable("alma"s);
            perf_test_sink += subValue.toInt();
        }
    }
    {
        cPrintfPerformance_Guard perf("register_acc");
        auto globalTable = script->globalTable();
        auto subValue = globalTable.subTable("alma"s);
        for (int i = 0; i < 1'000'000; ++i)
        {
            perf_test_sink += subValue.toInt();
        }
    }
}
*/

TEST(lua_table, get_set_in_global_table)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue globalTable = script->globalTable();
    storeTestVariables(globalTable);
    verifyTestVariableValues(globalTable);
// now if we retrieve it again, it should be still readable:
    verifyTestVariableValues(script->globalTable());
}

TEST(lua_value, toInt)
{
    auto script = std::make_shared<cLuaScript>();
    cLuaValue globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaValue intValue = *globalTable.get<cLuaValue>("twelve");
    ASSERT_EQ(intValue.toInt(), 12);
    cLuaValue stringValue = *globalTable.get<cLuaValue>("ten_string");
    ASSERT_EQ(stringValue.toInt(), 10);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, toString)
{
    auto script = std::make_shared<cLuaScript>();
    cLuaValue globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaValue intValue = *globalTable.get<cLuaValue>("twelve");
    ASSERT_STREQ(intValue.toString().c_str(), "12");
    cLuaValue stringValue = *globalTable.get<cLuaValue>("ten_string");
    ASSERT_STREQ(stringValue.toString().c_str(), "10");

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, toDouble)
{
    auto script = std::make_shared<cLuaScript>();
    cLuaValue globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaValue intValue = *globalTable.get<cLuaValue>("twelve");
    ASSERT_EQ(intValue.toDouble(), 12.0);
    cLuaValue stringValue = *globalTable.get<cLuaValue>("ten_string");
    ASSERT_EQ(stringValue.toDouble(), 10.0);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, isString)
{
    auto script = std::make_shared<cLuaScript>();
    cLuaValue globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaValue intValue = *globalTable.get<cLuaValue>("twelve");
    EXPECT_FALSE(intValue.isString());
    cLuaValue floatValue = *globalTable.get<cLuaValue>("twelve_and_half");
    EXPECT_FALSE(floatValue.isString());
    cLuaValue stringValue = *globalTable.get<cLuaValue>("ten_string");
    EXPECT_TRUE(stringValue.isString());

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, isNumber)
{
    auto script = std::make_shared<cLuaScript>();
    cLuaValue globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaValue intValue = *globalTable.get<cLuaValue>("twelve");
    EXPECT_TRUE(intValue.isNumber());
    cLuaValue floatValue = *globalTable.get<cLuaValue>("twelve_and_half");
    EXPECT_TRUE(floatValue.isNumber());
    cLuaValue stringValue = *globalTable.get<cLuaValue>("ten_string");
    EXPECT_FALSE(stringValue.isNumber());

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, visit_single_variable)
{
    auto script = std::make_shared<cLuaScript>();
    cLuaValue globalTable = script->globalTable();
    storeTestVariables(globalTable);

    std::variant<std::monostate, int, double, std::string, cLuaValue> extractedValue;
    cLuaValue intValue = *globalTable.get<cLuaValue>("twelve");
    intValue.visit([&extractedValue](const auto& value)
        {
            extractedValue = value;
        });
    ASSERT(std::holds_alternative<int>(extractedValue));
    ASSERT_EQ(std::get<int>(extractedValue), 12);

    cLuaValue floatValue = *globalTable.get<cLuaValue>("twelve_and_half");
    floatValue.visit([&extractedValue](const auto& value)
        {
            extractedValue = value;
        });
    ASSERT(std::holds_alternative<double>(extractedValue));
    ASSERT_EQ(std::get<double>(extractedValue), 12.5);

    cLuaValue stringValue = *globalTable.get<cLuaValue>("ten_string");
    stringValue.visit([&extractedValue](const auto& value)
        {
            extractedValue = value;
        });
    ASSERT(std::holds_alternative<std::string>(extractedValue));
    ASSERT_STREQ(std::get<std::string>(extractedValue).c_str(), "10");

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, array_get)
{
    auto script = std::make_shared<cLuaScript>();
    script->executeString("my_array = { 2, 9, 13, \"hello\" }");
    cLuaValue globalTable = script->globalTable();
    cLuaValue testedArray = *globalTable.get<cLuaValue>("my_array");

    int element_1 = testedArray.get<int>(1);
    ASSERT_EQ(element_1, 2);
    int element_2 = testedArray.get<int>(2);
    ASSERT_EQ(element_2, 9);
    int element_3 = testedArray.get<int>(3);
    ASSERT_EQ(element_3, 13);
    std::string element_4 = testedArray.get<std::string>(4);
    ASSERT_STREQ(element_4.c_str(), "hello");

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, get_value_not_present)
{
    auto script = std::make_shared<cLuaScript>();
    script->executeString("alma = 54\nkorte = \"hello\"");
    cLuaValue globalTable = script->globalTable();
    {
        auto failedRetrievedValue = globalTable.get<int>("citrom");
        ASSERT_FALSE(failedRetrievedValue.has_value());
    }
    {
        auto failedRetrievedValue = globalTable.get<std::string>("citrom");
        ASSERT_FALSE(failedRetrievedValue.has_value());
    }
    {
        auto failedRetrievedValue = globalTable.get<cLuaValue>("citrom");
        ASSERT_FALSE(failedRetrievedValue.has_value());
    }
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, arraySize)
{
    auto script = std::make_shared<cLuaScript>();
    script->executeString(
        "my_array = { 2, 9, 13, \"hello\" }\n"
        "my_table = { a = 1, b = 2, c = 3 }\n"
        "my_empty_table = {}\n"
        "my_value = 42\n");
    cLuaValue globalTable = script->globalTable();
    cLuaValue testedArray = *globalTable.get<cLuaValue>("my_array");
    cLuaValue testedTable = *globalTable.get<cLuaValue>("my_table");
    cLuaValue testedEmptyTable = *globalTable.get<cLuaValue>("my_empty_table");
    cLuaValue testedValue = *globalTable.get<cLuaValue>("my_value");

    ASSERT_EQ(testedArray.arraySize(), 4u);
    ASSERT_EQ(testedTable.arraySize(), 0u);
    ASSERT_EQ(testedEmptyTable.arraySize(), 0u);
    ASSERT_EQ(testedValue.arraySize(), 0u);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, get_set_in_local_table)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue localTable = script->createTable();
    storeTestVariables(localTable);
    verifyTestVariableValues(localTable);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, copy_table_obj)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue localTable = script->createTable();
    storeTestVariables(localTable);
    verifyTestVariableValues(localTable);
    cLuaValue copyOfTableObj = localTable;
    verifyTestVariableValues(copyOfTableObj);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, move_table_obj)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue localTable = script->createTable();
    storeTestVariables(localTable);
    verifyTestVariableValues(localTable);
    cLuaValue movedTableObj = std::move(localTable);
    verifyTestVariableValues(movedTableObj);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, is_type)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue globalTable = script->globalTable();
    storeTestVariables(globalTable);
    verifyTestVariableTypes(script->globalTable());

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, create_sub_tables)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue globalTable = script->globalTable();
    cLuaValue subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);
    verifyTestVariableValues(subTable);
    verifyTestVariableValues(script->globalTable().subTable("mySubTable"));

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, sub_table_access_with_get)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue globalTable = script->globalTable();
    cLuaValue subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);

    ASSERT_TRUE(globalTable.isType<cLuaValue>("mySubTable"s));

    cLuaValue subTable2 = *globalTable.get<cLuaValue>("mySubTable"s);
    verifyTestVariableValues(subTable2);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_function_register, funcion_v_v)  // void(void)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue globalTable = script->globalTable();
    bool success = false;
    globalTable.registerFunction<void>("test"s, [&success]() ->void { success = true; });
    ASSERT_EQ(script->stackSize(), 0) << "after register";
    auto returned = globalTable.callFunction("test"s);
    ASSERT_TRUE(success);
    ASSERT_TRUE(returned.empty());

    ASSERT_EQ(script->stackSize(), 0) << "after call";
}

TEST(lua_function_remove, funcion_v_v)  // void(void)
{
    bool wasDestroyed = false;
    {
        auto script = std::make_shared<cLuaScript>();

        cLuaValue globalTable = script->globalTable();
        struct cDestructorChecker
        {
            cDestructorChecker(bool& wasDestroyed) : mWasDestroyed(wasDestroyed) {}
            ~cDestructorChecker() { mWasDestroyed = true; }
            bool& mWasDestroyed;
        };
        globalTable.registerFunction<void>("test"s,
            [destructorChecker = std::make_shared<cDestructorChecker>(wasDestroyed)]() {});
        ASSERT_EQ(script->stackSize(), 0) << "after register";
        globalTable.remove("test"s);
        ASSERT_EQ(script->stackSize(), 0) << "after remove";
        ASSERT_FALSE(globalTable.has("test"s));
    }
    ASSERT_TRUE(wasDestroyed);
}

TEST(lua_function_register, function_v_iii)  // void(int,int,int)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue globalTable = script->globalTable();
    int result = 0;
    globalTable.registerFunction<void, int, int, int>("test"s,
        [&result](int a, int b, int c)
        {
            result = a + b + c;
        });

    auto returned = globalTable.callFunction("test"s, 10, 11, 12);
    ASSERT_EQ(result, 33);
    ASSERT_TRUE(returned.empty());

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_function_register, function_v_t)  // void(cLuaValue)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue globalTable = script->globalTable();
    int result = 0;
    globalTable.registerFunction<void, cLuaValue>("test"s,
        [&result](cLuaValue t)
        {
            result = *t.get<int>("a") + *t.get<int>("b");
        });

    cLuaValue parameterTable = script->createTable();
    parameterTable.set("a", 33);
    parameterTable.set("b", 44);
    auto returned = globalTable.callFunction("test"s, std::move(parameterTable));
    ASSERT_TRUE(returned.empty());
    ASSERT_EQ(result, 77);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_function_call, function_i_ii)
{
    auto script = std::make_shared<cLuaScript>();

    script->executeString(
        "function otherFunction(a, b) return a+b end\n"
        "function testedFunction(a, b)\n"
        "local x = otherFunction(a,b)\n"
        "return x\n"
        "end");
    
    cLuaValue globalTable = script->globalTable();
    auto returned = globalTable.callFunction("testedFunction"s, 10, 11);
    ASSERT_EQ(returned.size(), 1u);
    ASSERT_EQ(returned.front().toInt(), 21);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, c_to_lua_back_to_c)
{
    auto script = std::make_shared<cLuaScript>();

    cLuaValue globalTable = script->globalTable();
    globalTable.registerFunction<int, int, int>("test"s,
        [](int a, int b) -> int
        {
            return a * b;
        });
    script->executeString(
        "my_table = {\n"
        "  testedFunction = function(self, a, b) return test(a, b) end\n"
        "}\n");

    auto myTable = *globalTable.get<cLuaValue>("my_table"s);
    auto returned = myTable.callMemberFunction("testedFunction"s, 10, 11);
    ASSERT_EQ(returned.size(), 1u);
    ASSERT_EQ(returned.front().toInt(), 110);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, call)
{
    auto script = std::make_shared<cLuaScript>();

    script->executeString("function testedFunction(a, b) return a+b end");

    cLuaValue globalTable = script->globalTable();
    cLuaValue testedFunction = *globalTable.get<cLuaValue>("testedFunction");
    auto returned = testedFunction.call(10, 11);
    ASSERT_EQ(returned.size(), 1u);
    ASSERT_EQ(returned.front().toInt(), 21);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, isFunction)
{
auto script = std::make_shared<cLuaScript>();

    script->executeString("function testedFunction(a, b) return a+b end");
    script->executeString("apple = 42");

    cLuaValue globalTable = script->globalTable();
    cLuaValue testedFunction = *globalTable.get<cLuaValue>("testedFunction");
    ASSERT_TRUE(testedFunction.isFunction());
    cLuaValue apple = *globalTable.get<cLuaValue>("apple");
    ASSERT_FALSE(apple.isFunction());

    ASSERT_EQ(script->stackSize(), 0);

}

TEST(lua_function_register, function_i_ii)
{
    auto script = std::make_shared<cLuaScript>();
    cLuaValue globalTable = script->globalTable();

    globalTable.registerFunction<int, int, int>("testedFunction"s,
        [](int a, int b)
        {
            return a + b;
        });

    auto returned = globalTable.callFunction("testedFunction"s, 10, 11);
    ASSERT_EQ(returned.size(), 1u);
    ASSERT_EQ(returned.front().toInt(), 21);

    ASSERT_EQ(script->stackSize(), 0);
}

void setupConfigTestVariables(cLuaScript& script)
{
    cLuaValue globalTable = script.globalTable();
    storeTestVariables(globalTable);
    cLuaValue subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);
}

void verifyConfigSingleLevel(const cConfig& config)
{
    {
        auto value = config.get<int>("twelve", 0);
        ASSERT_EQ(value, 12) << "twelve";
    }
    {
        auto value = config.get<int>("ten", 0);
        ASSERT_EQ(value, 10) << "ten";
    }
    {
        auto value = config.get<double>("twelve_and_half", 0.0);
        ASSERT_EQ(value, 12.5) << "twelve_and_half";
    }
    {
        auto value = config.get<double>("ten_and_half", 0.0);
        ASSERT_EQ(value, 10.5) << "ten_and_half";
    }
    {
        auto value = config.get<std::string>("twelve", {});
        ASSERT_STREQ(value.c_str(), "12") << "twelve";
    }
    {
        auto value = config.get<std::string>("ten", {});
        ASSERT_STREQ(value.c_str(), "10") << "ten";
    }

}

TEST(lua_table, toConfig_nonrecursive)
{
    auto script = std::make_shared<cLuaScript>();

    setupConfigTestVariables(*script);
    auto config = script->globalTable().toConfig(cLuaValue::IsRecursive::No);
    verifyConfigSingleLevel(*config);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, toConfig_recursive)
{
    auto script = std::make_shared<cLuaScript>();

    setupConfigTestVariables(*script);
    auto config = script->globalTable().toConfig(cLuaValue::IsRecursive::Yes);
    verifyConfigSingleLevel(*config);
    auto subConfig = config->getSubConfig("mySubTable");
    ASSERT_TRUE(subConfig);
    verifyConfigSingleLevel(*subConfig);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, forEach)
{
    auto script = std::make_shared<cLuaScript>();

    setupConfigTestVariables(*script);
    auto globalTable = script->globalTable();
    std::unordered_map<std::string, cLuaValue> extractedValues;
    globalTable.forEach([&extractedValues](const std::string& key, const cLuaValue& value)
        {
            extractedValues[key] = value;
        });
    ASSERT_EQ(script->stackSize(), 0);

    ASSERT_EQ(extractedValues.size(), 7u);
    ASSERT_EQ(extractedValues["twelve"].toInt(), 12);
    ASSERT_EQ(extractedValues["ten"].toInt(), 10);
    ASSERT_EQ(extractedValues["twelve_string"].toString(), "12");
    ASSERT_EQ(extractedValues["ten_string"].toString(), "10");
    ASSERT_EQ(extractedValues["twelve_and_half"].toDouble(), 12.5);
    ASSERT_EQ(extractedValues["ten_and_half"].toDouble(), 10.5);
    ASSERT_TRUE(extractedValues["mySubTable"].isTable());
}


TEST(lua_execute, simple_execute)
{
    auto script = std::make_shared<cLuaScript>();

    script->executeString(
        "apple = 42\n"
        "my_table = { apple = 22, pear = 33 }");

    auto globalTable = script->globalTable();
    ASSERT_EQ(globalTable.get<int>("apple"), 42);
    auto subTable = *globalTable.get<cLuaValue>("my_table");
    ASSERT_EQ(subTable.get<int>("apple"), 22);
    ASSERT_EQ(subTable.get<int>("pear"), 33);
    ASSERT_EQ(script->stackSize(), 0);
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

    ASSERT_EQ(*globalTable.get<int>("result"), 5555);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_execute, c_function_t_t)
{
    auto script = std::make_shared<cLuaScript>();
    auto globalTable = script->globalTable();

    globalTable.registerFunction<cLuaValue, cLuaValue>("test"s,
        [](cLuaValue sourceTable)
        {
            auto resultTable = sourceTable.script().createTable();
            resultTable.set<int>("a", *sourceTable.get<int>("a") + 11);
            resultTable.set<int>("b", *sourceTable.get<int>("b") + 22);
            return resultTable;
        });

    script->executeString("result = test({ a = 7, b = 8})");

    auto result = *globalTable.get<cLuaValue>("result");

    ASSERT_EQ(result.get<int>("a"), 18);
    ASSERT_EQ(result.get<int>("b"), 30);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_oop, callMemberFunction)
{
    auto script = std::make_shared<cLuaScript>();

    script->executeString(
        "my_table = "
        "{"
        "    a = 1,"
        "    increase = function(self, b) self.a = self.a + b end"
        "}");

    auto globalTable = script->globalTable();
    auto myTable = *globalTable.get<cLuaValue>("my_table");
    myTable.callFunction("increase", myTable, 2);
    ASSERT_EQ(myTable.get<int>("a"), 3u);
    myTable.callMemberFunction("increase", 123);
    ASSERT_EQ(myTable.get<int>("a"), 126u);
}

TEST(lua_oop, exception_from_cpp_function)
{
    auto script = std::make_shared<cLuaScript>();
    auto globalTable = script->globalTable();

    globalTable.registerFunction<int, int, int>("test"s,
        [](int r, int l) -> int
        {
            if(r==l)
                throw std::string("exception from C++");
            return r + l;
        });

    try
    {
        script->executeString("result = test(1234, 1234)");
        FAIL() << "exception not thrown";
    }
    catch (const std::string& e)
    {
        ASSERT_STREQ(e.c_str(), "exception from C++");
    }
    ASSERT_EQ(script->stackSize(), 0);

    script->executeString("result = test(1,2)");
    ASSERT_EQ(globalTable.get<int>("result"), 3);
    ASSERT_EQ(script->stackSize(), 0);

    globalTable.registerFunction<int, cLuaValue, cLuaValue>("test2"s,
        [](cLuaValue r, cLuaValue l) -> int
        {
            if(r.toInt() == l.toInt())
                throw std::string("exception from C++");
            return r.toInt() + l.toInt();
        });
    try
    {
        script->executeString("result = test2(1234, 1234)");
        FAIL() << "exception not thrown";
    }
    catch (const std::string& e)
    {
        ASSERT_STREQ(e.c_str(), "exception from C++");
    }

    ASSERT_EQ(script->stackSize(), 0);

    script->executeString("result = test2(5,6)");
    ASSERT_EQ(globalTable.get<int>("result"), 11);
    ASSERT_EQ(script->stackSize(), 0);
}

// Main function to run the tests
int main(int argc, char** argv)
{
    cLuaScript::staticInit();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}