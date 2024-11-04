#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

namespace TuplePrefixTests
{

TEST(tuple_prefix, nonempty_prefixes)
{
    using T = std::tuple<int, double, std::string, char>;

    static_assert(std::is_same_v<tTuplePrefix<1, T>, std::tuple<int>>);
    static_assert(std::is_same_v<tTuplePrefix<2, T>, std::tuple<int, double>>);
    static_assert(std::is_same_v<tTuplePrefix<3, T>, std::tuple<int, double, std::string>>);
    static_assert(std::is_same_v<tTuplePrefix<4, T>, std::tuple<int, double, std::string, char>>);
}

TEST(tuple_prefix, empty_prefix)
{
    using T = std::tuple<int, double, std::string, char>;
    static_assert(std::is_same_v<tTuplePrefix<0, T>, std::tuple<>>);
}

TEST(tuple_prefix, prefix_taker_function)
{
    using T = std::tuple<int, double, std::string, char>;

    static_assert(std::is_same_v<tTuplePrefixTakerFunction<1, T>, std::function<void(int)>>);
    static_assert(std::is_same_v<tTuplePrefixTakerFunction<2, T>, std::function<void(int, double)>>);
    static_assert(std::is_same_v<tTuplePrefixTakerFunction<3, T>, std::function<void(int, double, std::string)>>);
    static_assert(std::is_same_v<tTuplePrefixTakerFunction<4, T>, std::function<void(int, double, std::string, char)>>);

    // 0 must be empty function:
    static_assert(std::is_same_v<tTuplePrefixTakerFunction<0, T>, std::function<void()>>);
}



} // namespace TuplePrefixTests

