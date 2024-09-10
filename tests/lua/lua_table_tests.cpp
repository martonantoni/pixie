#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

const char* ProgramName = "Pixie / Lua support Test";
const char* VersionString = "0.4";


void storeTestVariables(cLuaObject& table)
{
    table.set("twelve", 12);
    table.set("ten", 10);
    table.set("twelve_string", "12"s);
    table.set("ten_string", "10");
    table.set("twelve_and_half", 12.5);
    table.set("ten_and_half", 10.5);
}

void verifyTestVariableValues(const cLuaObject& table)
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

void verifyTestVariableTypes(const cLuaObject& table)
{
    ASSERT_TRUE(table.isType<int>("twelve"));
    ASSERT_FALSE(table.isType<std::string>("twelve"));
}

TEST(lua_value, get_set_in_global_table)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    storeTestVariables(globalTable);
    verifyTestVariableValues(globalTable);
// now if we retrieve it again, it should be still readable:
    verifyTestVariableValues(script->globalTable());
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, assignment_operator_int)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject value = script->createValue();
    value = 42;
    ASSERT_TRUE(value.isNumber());
    ASSERT_EQ(value.toInt(), 42);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, assignment_operator_double)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject value = script->createValue();
    value = 42.5;
    ASSERT_TRUE(value.isNumber());
    ASSERT_EQ(value.toDouble(), 42.5);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, assignment_operator_string)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject value = script->createValue();
    value = "hello"s;
    ASSERT_TRUE(value.isString());
    ASSERT_STREQ(value.toString().c_str(), "hello");
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, assignment_operator_cstring)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject value = script->createValue();
    value = "hello";
    ASSERT_TRUE(value.isString());
    ASSERT_STREQ(value.toString().c_str(), "hello");
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, assignment_op_mix)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject value = script->createValue();
    value = 42;
    ASSERT_EQ(value.toInt(), 42);
    value = 42.5;
    ASSERT_EQ(value.toDouble(), 42.5);
    value = "hello"s;
    ASSERT_TRUE(value.isString());
    ASSERT_STREQ(value.toString().c_str(), "hello");
    ASSERT_EQ(script->stackSize(), 0);
}


TEST(lua_value, toInt)
{
    auto script = std::make_shared<cLuaState>();
    cLuaObject globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaObject intValue = globalTable.get<cLuaObject>("twelve");
    ASSERT_EQ(intValue.toInt(), 12);
    cLuaObject stringValue = globalTable.get<cLuaObject>("ten_string");
    ASSERT_EQ(stringValue.toInt(), 10);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, toDouble)
{
    auto script = std::make_shared<cLuaState>();
    cLuaObject globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaObject intValue = globalTable.get<cLuaObject>("twelve");
    ASSERT_EQ(intValue.toDouble(), 12.0);
    cLuaObject stringValue = globalTable.get<cLuaObject>("ten_string");
    ASSERT_EQ(stringValue.toDouble(), 10.0);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, isString)
{
    auto script = std::make_shared<cLuaState>();
    cLuaObject globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaObject intValue = globalTable.get<cLuaObject>("twelve");
    EXPECT_FALSE(intValue.isString());
    cLuaObject floatValue = globalTable.get<cLuaObject>("twelve_and_half");
    EXPECT_FALSE(floatValue.isString());
    cLuaObject stringValue = globalTable.get<cLuaObject>("ten_string");
    EXPECT_TRUE(stringValue.isString());

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, isNumber)
{
    auto script = std::make_shared<cLuaState>();
    cLuaObject globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaObject intValue = globalTable.get<cLuaObject>("twelve");
    EXPECT_TRUE(intValue.isNumber());
    cLuaObject floatValue = globalTable.get<cLuaObject>("twelve_and_half");
    EXPECT_TRUE(floatValue.isNumber());
    cLuaObject stringValue = globalTable.get<cLuaObject>("ten_string");
    EXPECT_FALSE(stringValue.isNumber());

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, toStringIntegers)
{
    auto script = std::make_shared<cLuaState>();
    cLuaObject globalTable = script->globalTable();

    storeTestVariables(globalTable);
    cLuaObject intValue = globalTable.get<cLuaObject>("twelve");
    ASSERT_STREQ(intValue.toString().c_str(), "12");
    cLuaObject stringValue = globalTable.get<cLuaObject>("ten_string");
    ASSERT_STREQ(stringValue.toString().c_str(), "10");

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, toStringMixed)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString("my_array = { 2, 9/3, 0.5*2, 13.5, \"hello\" }\n");
    {
        cLuaObject globalTable = script->globalTable();
        std::string result;
        globalTable.get<cLuaObject>("my_array").forEach(
            [&result](const auto& value)
            {
                result += value.toString() + " ";
            });
        ASSERT_STREQ(result.c_str(), "2 3 1 13.5 hello ");
    }

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, visit_single_variable)
{
    auto script = std::make_shared<cLuaState>();
    cLuaObject globalTable = script->globalTable();
    storeTestVariables(globalTable);

    std::variant<std::monostate, int, double, std::string, cLuaObject> extractedValue;
    cLuaObject intValue = globalTable.get<cLuaObject>("twelve");
    intValue.visit([&extractedValue](const auto& value)
        {
            extractedValue = value;
        });
    ASSERT_TRUE(std::holds_alternative<int>(extractedValue));
    ASSERT_EQ(std::get<int>(extractedValue), 12);

    cLuaObject floatValue = globalTable.get<cLuaObject>("twelve_and_half");
    floatValue.visit([&extractedValue](const auto& value)
        {
            extractedValue = value;
        });
    ASSERT_TRUE(std::holds_alternative<double>(extractedValue));
    ASSERT_EQ(std::get<double>(extractedValue), 12.5);

    cLuaObject stringValue = globalTable.get<cLuaObject>("ten_string");
    stringValue.visit([&extractedValue](const auto& value)
        {
            extractedValue = value;
        });
    ASSERT_TRUE(std::holds_alternative<std::string>(extractedValue));
    ASSERT_STREQ(std::get<std::string>(extractedValue).c_str(), "10");

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, visit_table_value)
{
    auto script = std::make_shared<cLuaState>();
    cLuaObject globalTable = script->globalTable();
    storeTestVariables(globalTable);

    cLuaObject subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);

    std::variant<std::monostate, int, double, std::string, cLuaObject> extractedValue;
    subTable.visit([&extractedValue](const auto& value)
        {
            extractedValue = value;
        });
    ASSERT_TRUE(std::holds_alternative<cLuaObject>(extractedValue));
    auto extractedSubTable = std::get<cLuaObject>(extractedValue);
    verifyTestVariableValues(extractedSubTable);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, array_get)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString("my_array = { 2, 9, 13, \"hello\" }");
    cLuaObject globalTable = script->globalTable();
    cLuaObject testedArray = globalTable.get<cLuaObject>("my_array");

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

TEST(lua_value, tryGet_valueNotPresent)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString("alma = 54\nkorte = \"hello\"");
    cLuaObject globalTable = script->globalTable();
    {
        auto failedRetrievedValue = globalTable.tryGet<int>("citrom");
        ASSERT_FALSE(failedRetrievedValue.has_value());
    }
    {
        auto failedRetrievedValue = globalTable.tryGet<std::string>("citrom");
        ASSERT_FALSE(failedRetrievedValue.has_value());
    }
    {
        auto failedRetrievedValue = globalTable.tryGet<cLuaObject>("citrom");
        ASSERT_FALSE(failedRetrievedValue.has_value());
    }
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, tryGet_valuePresent)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString("alma = 54\nkorte = \"hello\"");
    cLuaObject globalTable = script->globalTable();
    {
        auto retrievedValue = globalTable.tryGet<int>("alma");
        ASSERT_TRUE(retrievedValue.has_value());
        ASSERT_EQ(retrievedValue.value(), 54);
    }
    {
        auto retrievedValue = globalTable.tryGet<std::string>("korte");
        ASSERT_TRUE(retrievedValue.has_value());
        ASSERT_STREQ(retrievedValue.value().c_str(), "hello");
    }
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, tryGet_int_key_fails)
{
    auto script = std::make_shared<cLuaState>();

    script->executeString("alma = 54\nkorte = \"hello\"");
    cLuaObject globalTable = script->globalTable();
    {
        auto failedRetrievedValue = globalTable.tryGet(1);
        ASSERT_FALSE(failedRetrievedValue.has_value());
    }
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, tryGet_int_key_succeeds)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString("my_array = { 2, 9, 13, \"hello\" }\n");
    auto myTable = script->globalTable().get("my_array");
    {
        auto retrievedValue = myTable.tryGet(1);
        ASSERT_TRUE(retrievedValue.has_value());
        ASSERT_EQ(retrievedValue.value().toInt(), 2);
    }
    {
        auto retrievedValue = myTable.tryGet(2);
        ASSERT_TRUE(retrievedValue.has_value());
        ASSERT_EQ(retrievedValue.value().toInt(), 9);
    }
    {
        auto retrievedValue = myTable.tryGet(3);
        ASSERT_TRUE(retrievedValue.has_value());
        ASSERT_EQ(retrievedValue.value().toInt(), 13);
    }
    {
        auto retrievedValue = myTable.tryGet(4);
        ASSERT_TRUE(retrievedValue.has_value());
        ASSERT_STREQ(retrievedValue.value().toString().c_str(), "hello");
    }
    ASSERT_EQ(script->stackSize(), 0);
}


TEST(lua_value, tryGet_luavalue_key)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString("my_array = { 2, 9, 13, \"hello\" }\n");
    auto keyValue = script->createValue();
    keyValue = "my_array";
    auto myTable = script->globalTable().tryGet(keyValue);
    ASSERT_TRUE(myTable.has_value());
    ASSERT_TRUE(myTable->isTable());
    {
        keyValue = 4;
        auto retrievedValue = myTable->tryGet<cLuaObject>(keyValue);
        ASSERT_TRUE(retrievedValue.has_value());
        ASSERT_STREQ(retrievedValue.value().toString().c_str(), "hello");
    }
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, arraySize)
{
    auto script = std::make_shared<cLuaState>();
    script->executeString(
        "my_array = { 2, 9, 13, \"hello\" }\n"
        "my_table = { a = 1, b = 2, c = 3 }\n"
        "my_empty_table = {}\n"
        "my_value = 42\n");
    cLuaObject globalTable = script->globalTable();
    cLuaObject testedArray = globalTable.get<cLuaObject>("my_array");
    cLuaObject testedTable = globalTable.get<cLuaObject>("my_table");
    cLuaObject testedEmptyTable = globalTable.get<cLuaObject>("my_empty_table");
    cLuaObject testedValue = globalTable.get<cLuaObject>("my_value");

    ASSERT_EQ(testedArray.arraySize(), 4u);
    ASSERT_EQ(testedTable.arraySize(), 0u);
    ASSERT_EQ(testedEmptyTable.arraySize(), 0u);
    ASSERT_EQ(testedValue.arraySize(), 0u);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, get_set_in_local_table_with_string_key)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject localTable = script->createTable();
    storeTestVariables(localTable);
    verifyTestVariableValues(localTable);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, get_set_in_local_array)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject localTable = script->createTable();
    localTable.set(1, 12);
    localTable.set(2, 10);
    localTable.set(3, 12.5);
    localTable.set(4, 10.5);
    localTable.set(5, "12"s);
    localTable.set(6, "10");
    localTable.set(7, true);

    ASSERT_EQ(localTable.arraySize(), 7u);
    
    ASSERT_EQ(localTable.get<int>(1), 12);
    ASSERT_EQ(localTable.get<int>(2), 10);
    ASSERT_EQ(localTable.get<double>(3), 12.5);
    ASSERT_EQ(localTable.get<double>(4), 10.5);
    ASSERT_STREQ(localTable.get<std::string>(5).c_str(), "12");
    ASSERT_STREQ(localTable.get<std::string>(6).c_str(), "10");
    ASSERT_TRUE(localTable.get<bool>(7));

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, get_set_in_local_table_lua_val_key)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject localTable = script->createTable();
    auto key = script->createValue();
    key = "twelve";
    localTable.set(key, 12);
    key = 33;
    localTable.set(key, 103.11);

    key = "twelve";
    ASSERT_EQ(localTable.get<int>(key), 12);
    key = 33;
    ASSERT_EQ(localTable.get<double>(key), 103.11);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, copy_table_obj)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject localTable = script->createTable();
    storeTestVariables(localTable);
    verifyTestVariableValues(localTable);
    cLuaObject copyOfTableObj = localTable;
    verifyTestVariableValues(copyOfTableObj);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, move_table_obj)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject localTable = script->createTable();
    storeTestVariables(localTable);
    verifyTestVariableValues(localTable);
    cLuaObject movedTableObj = std::move(localTable);
    verifyTestVariableValues(movedTableObj);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, is_type)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    storeTestVariables(globalTable);
    verifyTestVariableTypes(script->globalTable());

    script->executeString("my_array = { 2, 9, 13, \"hello\" }\n");
    cLuaObject myArray = globalTable.get<cLuaObject>("my_array");
    ASSERT_TRUE(myArray.isTable());
    ASSERT_TRUE(myArray.isType<int>(1));
    ASSERT_TRUE(myArray.isType<int>(2));
    ASSERT_TRUE(myArray.isType<int>(3));
    ASSERT_TRUE(myArray.isType<std::string>(4));

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, has)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    storeTestVariables(globalTable);
    ASSERT_TRUE(globalTable.has("twelve"));
    ASSERT_TRUE(globalTable.has("ten"));
    ASSERT_TRUE(globalTable.has("twelve_and_half"));
    ASSERT_TRUE(globalTable.has("ten_and_half"));
    ASSERT_FALSE(globalTable.has("not_existing"));

// integer key:
    script->executeString("my_array = { 2, 9, 13, \"hello\" }\n");
    cLuaObject myArray = globalTable.get<cLuaObject>("my_array");
    ASSERT_TRUE(myArray.has(1));
    ASSERT_TRUE(myArray.has(2));
    ASSERT_TRUE(myArray.has(3));
    ASSERT_TRUE(myArray.has(4));
    ASSERT_FALSE(myArray.has(5));

    ASSERT_EQ(script->stackSize(), 0);

}

TEST(lua_table, remove)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    storeTestVariables(globalTable);
    ASSERT_TRUE(globalTable.has("twelve"));
    globalTable.remove("twelve");
    ASSERT_FALSE(globalTable.has("twelve"));

// integer key:
    script->executeString("my_array = { 2, 9, 13, \"hello\" }\n");
    cLuaObject myArray = globalTable.get<cLuaObject>("my_array");
    ASSERT_TRUE(myArray.isTable());
    myArray.remove(3); // removes the 13
    EXPECT_EQ(myArray.arraySize(), 3u);
    EXPECT_TRUE(myArray.has(1));
    EXPECT_TRUE(myArray.has(2));
    EXPECT_TRUE(myArray.has(3)); // shifted
    EXPECT_FALSE(myArray.has(4));
    EXPECT_STREQ(myArray.get<std::string>(3).c_str(), "hello");

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, remove_from_array_fails)
{
    auto script = std::make_shared<cLuaState>();

    script->executeString("my_array = { 2, 9, 13, \"hello\" }\n");
    cLuaObject myArray = script->globalTable().get("my_array");
    ASSERT_EQ(myArray.arraySize(), 4u);
    EXPECT_THROW(myArray.remove("no_such_key"), std::runtime_error);
    ASSERT_EQ(myArray.arraySize(), 4u);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, create_sub_tables)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    cLuaObject subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);
    verifyTestVariableValues(subTable);
    verifyTestVariableValues(script->globalTable().subTable("mySubTable"));

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, sub_table_access_with_get)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    cLuaObject subTable = globalTable.subTable("mySubTable");
    storeTestVariables(subTable);

    ASSERT_TRUE(globalTable.isType<cLuaObject>("mySubTable"s));

    cLuaObject subTable2 = globalTable.get<cLuaObject>("mySubTable"s);
    verifyTestVariableValues(subTable2);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_function_register, funcion_v_v)  // void(void)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    bool success = false;
    globalTable.registerFunction<void>("test"s, [&success]() ->void { success = true; });
    ASSERT_EQ(script->stackSize(), 0) << "after register";
    globalTable.get("test"s).call();
    ASSERT_TRUE(success);

    ASSERT_EQ(script->stackSize(), 0) << "after call";
}

TEST(lua_function_remove, funcion_v_v)  // void(void)
{
    bool wasDestroyed = false;
    {
        auto script = std::make_shared<cLuaState>();

        cLuaObject globalTable = script->globalTable();
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
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    int result = 0;
    globalTable.registerFunction<void, int, int, int>("test"s,
        [&result](int a, int b, int c)
        {
            result = a + b + c;
        });

    globalTable.get("test"s).call("test"s, 10, 11, 12);
    ASSERT_EQ(result, 33);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_function_register, function_v_t)  // void(cLuaObject)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    int result = 0;
    globalTable.registerFunction<void, cLuaObject>("test"s,
        [&result](cLuaObject t)
        {
            result = t.get<int>("a") + t.get<int>("b");
        });

    cLuaObject parameterTable = script->createTable();
    parameterTable.set("a", 33);
    parameterTable.set("b", 44);
    globalTable.get("test"s).call(std::move(parameterTable));
    ASSERT_EQ(result, 77);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_function_register, trying_to_register_into_nontable)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject value = script->createValue();

    auto callRegister = [&]()
    {
        value.registerFunction<void>("test"s, []() {});
    };

    EXPECT_THROW(callRegister(), std::runtime_error);

    value = 42;

    EXPECT_THROW(callRegister(), std::runtime_error);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_function_register, array_of_functions)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();

    auto testedArray = script->createTable();

    testedArray.registerFunction<int, int, int>(1,
        [](int a, int b) -> int
        {
            return a + b;
        });
    testedArray.registerFunction<int, int, int>(2,
        [](int a, int b) -> int
        {
            return a - b;
        });

    auto addFunction = testedArray.get(1);
    auto subtractFunction = testedArray.get(2);

    ASSERT_EQ(testedArray.arraySize(), 2u);

    auto returned = addFunction.call<int>(10, 11);
    ASSERT_EQ(returned, 21);

    returned = subtractFunction.call<int>(10, 11);
    ASSERT_EQ(returned, -1);

    ASSERT_EQ(script->stackSize(), 0);

}

TEST(lua_function_call, function_i_ii)
{
    auto script = std::make_shared<cLuaState>();

    script->executeString(
        "function otherFunction(a, b) return a+b end\n"
        "function testedFunction(a, b)\n"
        "local x = otherFunction(a,b)\n"
        "return x\n"
        "end");
    
    cLuaObject globalTable = script->globalTable();
    int returned = globalTable.get("testedFunction"s).call<int>(10, 11);
    ASSERT_EQ(returned, 21);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_function_call, function_ssii_i)
{
    auto script = std::make_shared<cLuaState>();

    script->executeString(
        "function testedFunction(a, b)\n"
        "   return \"hello\", \"world\", a+b, a-b\n"
        "end");

    cLuaObject globalTable = script->globalTable();
    auto [hello, world, sum, diff] = globalTable.get("testedFunction"s).call<std::string, std::string, int, int>(10, 11);

    ASSERT_STREQ(hello.c_str(), "hello");
    ASSERT_STREQ(world.c_str(), "world");
    ASSERT_EQ(sum, 21);
    ASSERT_EQ(diff, -1);
    

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_function_call, returning_vector)
{
    auto script = std::make_shared<cLuaState>();

    script->executeString(
        "function testedFunction()\n"
        "   return 1, 2, 3, 4\n"
        "end");

    cLuaObject globalTable = script->globalTable();
    auto returned = globalTable.get("testedFunction"s).call<cLuaObject::ReturnVector>();
    ASSERT_EQ(returned.size(), 4u);
    ASSERT_EQ(returned[0].toInt(), 1);
    ASSERT_EQ(returned[1].toInt(), 2);
    ASSERT_EQ(returned[2].toInt(), 3);
    ASSERT_EQ(returned[3].toInt(), 4);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, c_to_lua_back_to_c)
{
    auto script = std::make_shared<cLuaState>();

    cLuaObject globalTable = script->globalTable();
    globalTable.registerFunction<int, int, int>("test"s,
        [](int a, int b) -> int
        {
            return a * b;
        });
    script->executeString(
        "my_table = {\n"
        "  testedFunction = function(self, a, b) return test(a, b) end\n"
        "}\n");

    auto myTable = globalTable.get<cLuaObject>("my_table"s);
    auto returned = myTable.callMember<int>("testedFunction"s, 10, 11);
    ASSERT_EQ(returned, 110);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, call)
{
    auto script = std::make_shared<cLuaState>();

    script->executeString("function testedFunction(a, b) return a+b end");

    cLuaObject globalTable = script->globalTable();
    cLuaObject testedFunction = globalTable.get<cLuaObject>("testedFunction");
    auto returned = testedFunction.call<int>(10, 11);
    ASSERT_EQ(returned, 21);

    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_value, isFunction)
{
    auto script = std::make_shared<cLuaState>();

    script->executeString("function testedFunction(a, b) return a+b end");
    script->executeString("apple = 42");

    cLuaObject globalTable = script->globalTable();
    cLuaObject testedFunction = globalTable.get<cLuaObject>("testedFunction");
    ASSERT_TRUE(testedFunction.isFunction());
    cLuaObject apple = globalTable.get<cLuaObject>("apple");
    ASSERT_FALSE(apple.isFunction());

    ASSERT_EQ(script->stackSize(), 0);

}

TEST(lua_function_register, function_i_ii)
{
    auto script = std::make_shared<cLuaState>();
    cLuaObject globalTable = script->globalTable();

    globalTable.registerFunction<int, int, int>("testedFunction"s,
        [](int a, int b)
        {
            return a + b;
        });

    auto returned = globalTable.get("testedFunction"s).call<int>(10, 11);
    ASSERT_EQ(returned, 21);

    ASSERT_EQ(script->stackSize(), 0);
}

void setupConfigTestVariables(cLuaState& script)
{
    cLuaObject globalTable = script.globalTable();
    storeTestVariables(globalTable);
    cLuaObject subTable = globalTable.subTable("mySubTable");
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
    auto script = std::make_shared<cLuaState>();

    setupConfigTestVariables(*script);
    auto config = script->globalTable().toConfig(cLuaObject::IsRecursive::No);
    verifyConfigSingleLevel(*config);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, toConfig_recursive)
{
    auto script = std::make_shared<cLuaState>();

    setupConfigTestVariables(*script);
    auto config = script->globalTable().toConfig(cLuaObject::IsRecursive::Yes);
    verifyConfigSingleLevel(*config);
    auto subConfig = config->getSubConfig("mySubTable");
    ASSERT_TRUE(subConfig);
    verifyConfigSingleLevel(*subConfig);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_table, forEach)
{
    auto script = std::make_shared<cLuaState>();

    setupConfigTestVariables(*script);
    auto globalTable = script->globalTable();
    std::unordered_map<std::string, cLuaObject> extractedValues;
    globalTable.forEach([&extractedValues](const std::string& key, const cLuaObject& value)
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
    auto script = std::make_shared<cLuaState>();

    script->executeString(
        "apple = 42\n"
        "my_table = { apple = 22, pear = 33 }");

    auto globalTable = script->globalTable();
    ASSERT_EQ(globalTable.get<int>("apple"), 42);
    auto subTable = globalTable.get<cLuaObject>("my_table");
    ASSERT_EQ(subTable.get<int>("apple"), 22);
    ASSERT_EQ(subTable.get<int>("pear"), 33);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_execute, c_function_i_ii)
{
    auto script = std::make_shared<cLuaState>();
    auto globalTable = script->globalTable();

    globalTable.registerFunction<int, int, int>("test"s,
        [](int r, int l)
        {
            return r + l;
        });
    script->executeString("result = test(1234, 4321)");

    ASSERT_EQ(globalTable.get<int>("result"), 5555);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_execute, c_function_t_t)
{
    auto script = std::make_shared<cLuaState>();
    auto globalTable = script->globalTable();

    globalTable.registerFunction<cLuaObject, cLuaObject>("test"s,
        [](cLuaObject sourceTable)
        {
            auto resultTable = sourceTable.state().createTable();
            resultTable.set<int>("a", sourceTable.get<int>("a") + 11);
            resultTable.set<int>("b", sourceTable.get<int>("b") + 22);
            return resultTable;
        });

    script->executeString("result = test({ a = 7, b = 8})");

    auto result = globalTable.get<cLuaObject>("result");

    ASSERT_EQ(result.get<int>("a"), 18);
    ASSERT_EQ(result.get<int>("b"), 30);
    ASSERT_EQ(script->stackSize(), 0);
}

TEST(lua_oop, callMemberFunction)
{
    auto script = std::make_shared<cLuaState>();

    script->executeString(
        "my_table = "
        "{"
        "    a = 1,"
        "    increase = function(self, b) self.a = self.a + b end"
        "}");

    auto globalTable = script->globalTable();
    auto myTable = globalTable.get<cLuaObject>("my_table");
    myTable.get("increase"s).call(myTable, 2);
    ASSERT_EQ(myTable.get<int>("a"), 3);
    myTable.callMember("increase"s, 123);
    ASSERT_EQ(myTable.get<int>("a"), 126);
}

TEST(lua_oop, exception_from_cpp_function)
{
    auto script = std::make_shared<cLuaState>();
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

    globalTable.registerFunction<int, cLuaObject, cLuaObject>("test2"s,
        [](cLuaObject r, cLuaObject l) -> int
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
    auto state = std::make_shared<cLuaState>();
    state->executeString(R"script(
        function sum(a, b)
            return a + b
        end

        function say_hello(year)
            return "hello", "world", year
        end )script");

    std::cout << state->globalTable().get("sum").call<int>(1, 2) << "\n"; // prints out 3

    auto say_hello = state->globalTable().get("say_hello");
    auto [word_1, word_2, year] = say_hello.call<std::string, std::string, int>(2024);
    std::cout << word_1 << " " << word_2 << " " << year << "\n"; // prints out hello world 2024

    auto retValues = say_hello.call<cLuaObject::ReturnVector>(2024);
    std::cout << retValues.size() << "\n"; // prints out 3
    for (auto value : retValues)
        std::cout << value.toString() << " "; // int value can be converted to string, so this works

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


/*
volatile int perf_test_sink;

TEST(lua_table, perf)
{
    auto script = std::make_shared<cLuaState>();
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