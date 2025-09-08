#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"


namespace CallableSignatureTests
{

TEST(CallableSignatureTests, lambda_i_ii)
{
    auto testedLambda = [](int a, int b) -> int { return a + b; };
    static_assert(cCallableSignature<decltype(testedLambda)>::numberOfArguments == 2);
    static_assert(std::is_same_v<cCallableSignature<decltype(testedLambda)>::ReturnType, int>);
    static_assert(std::tuple_size_v<cCallableSignature<decltype(testedLambda)>::Arguments> == 2);
    static_assert(std::is_same_v<int, std::tuple_element_t<0, cCallableSignature<decltype(testedLambda)>::Arguments>>);
    static_assert(std::is_same_v<int, std::tuple_element_t<1, cCallableSignature<decltype(testedLambda)>::Arguments>>);
    static_assert(cCallableSignature<decltype(testedLambda)>::isConst);
}

TEST(CallableSignatureTests, mutable_lambda_i_ii)
{
    auto testedLambda = [](int a, int b) mutable -> int { return a + b; };
    static_assert(cCallableSignature<decltype(testedLambda)>::numberOfArguments == 2);
    static_assert(std::is_same_v<cCallableSignature<decltype(testedLambda)>::ReturnType, int>);
    static_assert(std::is_same_v<int, std::tuple_element_t<0, cCallableSignature<decltype(testedLambda)>::Arguments>>);
    static_assert(std::is_same_v<int, std::tuple_element_t<1, cCallableSignature<decltype(testedLambda)>::Arguments>>);
    static_assert(!cCallableSignature<decltype(testedLambda)>::isConst);
}

TEST(CallableSignatureTests, lambda_v_v)
{
    auto testedLambda = [](){};
    static_assert(cCallableSignature<decltype(testedLambda)>::numberOfArguments == 0);
    static_assert(std::tuple_size_v<cCallableSignature<decltype(testedLambda)>::Arguments> == 0);
    static_assert(std::is_same_v<cCallableSignature<decltype(testedLambda)>::ReturnType, void>);
}

TEST(CallableSignatureTests, lambda_v_string_cref)
{
    auto testedLambda = [](const std::string&) {};

    static_assert(cCallableSignature<decltype(testedLambda)>::numberOfArguments == 1);
    static_assert(std::tuple_size_v<cCallableSignature<decltype(testedLambda)>::Arguments> == 1);
    static_assert(std::is_same_v<const std::string &, std::tuple_element_t<0, cCallableSignature<decltype(testedLambda)>::Arguments>>);
}

TEST(CallableSignatureTests, lambda_v_string)
{
    auto testedLambda = [](std::string) {};

    static_assert(cCallableSignature<decltype(testedLambda)>::numberOfArguments == 1);
    static_assert(std::tuple_size_v<cCallableSignature<decltype(testedLambda)>::Arguments> == 1);
    static_assert(std::is_same_v<std::string, std::tuple_element_t<0, cCallableSignature<decltype(testedLambda)>::Arguments>>);
}

TEST(CallableSignatureTests, lambda_double_stringint)
{
    auto testedLambda = [](std::string, int) -> double { return 0.0; };

    static_assert(cCallableSignature<decltype(testedLambda)>::numberOfArguments == 2);
    static_assert(std::tuple_size_v<cCallableSignature<decltype(testedLambda)>::Arguments> == 2);
    static_assert(std::is_same_v<std::string, std::tuple_element_t<0, cCallableSignature<decltype(testedLambda)>::Arguments>>);
    static_assert(std::is_same_v<int, std::tuple_element_t<1, cCallableSignature<decltype(testedLambda)>::Arguments>>);
    static_assert(std::is_same_v<double, cCallableSignature<decltype(testedLambda)>::ReturnType>);
}

TEST(CallableSignatureTests, member_functions)
{
    struct cTestStruct
    {
        int memberFunction(int a, double b) { return a + static_cast<int>(b); }
        void constMemberFunction(const std::string& s) const {}
    };
    static_assert(cCallableSignature<decltype(&cTestStruct::memberFunction)>::numberOfArguments == 2);
    static_assert(std::tuple_size_v<cCallableSignature<decltype(&cTestStruct::memberFunction)>::Arguments> == 2);
    static_assert(std::is_same_v<int, std::tuple_element_t<0, cCallableSignature<decltype(&cTestStruct::memberFunction)>::Arguments>>);
    static_assert(std::is_same_v<double, std::tuple_element_t<1, cCallableSignature<decltype(&cTestStruct::memberFunction)>::Arguments>>);
    static_assert(std::is_same_v<int, cCallableSignature<decltype(&cTestStruct::memberFunction)>::ReturnType>);

    static_assert(!cCallableSignature<decltype(&cTestStruct::memberFunction)>::isConst);
    static_assert(cCallableSignature<decltype(&cTestStruct::constMemberFunction)>::isConst);
    static_assert(cCallableSignature<decltype(&cTestStruct::constMemberFunction)>::numberOfArguments == 1);
    static_assert(std::tuple_size_v<cCallableSignature<decltype(&cTestStruct::constMemberFunction)>::Arguments> == 1);
    static_assert(std::is_same_v<const std::string&, std::tuple_element_t<0, cCallableSignature<decltype(&cTestStruct::constMemberFunction)>::Arguments>>);
    static_assert(std::is_same_v<void, cCallableSignature<decltype(&cTestStruct::constMemberFunction)>::ReturnType>);
}

void testFunct_v_is(int a, std::string s)
{
}

void testFunc_v_iscref(const int a, const std::string& s)
{
}

int testFunc_i_d(double d)
{
    return 0;
}

TEST(CallableSignatureTests, function_v_is)
{
    static_assert(std::tuple_size_v<cCallableSignature<decltype(&testFunct_v_is)>::Arguments> == 2);
    static_assert(std::is_same_v<int, std::tuple_element_t<0, cCallableSignature<decltype(&testFunct_v_is)>::Arguments>>);
    static_assert(std::is_same_v<std::string, std::tuple_element_t<1, cCallableSignature<decltype(&testFunct_v_is)>::Arguments>>);
    static_assert(std::is_same_v<void, cCallableSignature<decltype(&testFunct_v_is)>::ReturnType>);
}

TEST(CallableSignatureTests, function_v_iscref)
{
    static_assert(std::tuple_size_v<cCallableSignature<decltype(&testFunc_v_iscref)>::Arguments> == 2);
    static_assert(std::is_same_v<int, std::tuple_element_t<0, cCallableSignature<decltype(&testFunc_v_iscref)>::Arguments>>);
    static_assert(std::is_same_v<const std::string&, std::tuple_element_t<1, cCallableSignature<decltype(&testFunc_v_iscref)>::Arguments>>);
    static_assert(std::is_same_v<void, cCallableSignature<decltype(&testFunc_v_iscref)>::ReturnType>);
}

TEST(CallableSignatureTests, function_i_d)
{
    static_assert(std::tuple_size_v<cCallableSignature<decltype(&testFunc_i_d)>::Arguments> == 1);
    static_assert(std::is_same_v<double, std::tuple_element_t<0, cCallableSignature<decltype(&testFunc_i_d)>::Arguments>>);
    static_assert(std::is_same_v<int, cCallableSignature<decltype(&testFunc_i_d)>::ReturnType>);
}

}
