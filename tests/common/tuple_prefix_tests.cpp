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

TEST(tuple_postfix, nonempty_postfixes)
{
    using T = std::tuple<int, double, std::string, char>;
    static_assert(std::is_same_v<tTuplePostfix<T, 1>, std::tuple<char>>);
    static_assert(std::is_same_v<tTuplePostfix<T, 2>, std::tuple<std::string, char>>);
    static_assert(std::is_same_v<tTuplePostfix<T, 3>, std::tuple<double, std::string, char>>);
    static_assert(std::is_same_v<tTuplePostfix<T, 4>, std::tuple<int, double, std::string, char>>);
}

TEST(tuple_postfix, empty_postfix)
{
    using T = std::tuple<int, double, std::string, char>;
    static_assert(std::is_same_v<tTuplePostfix<T, 0>, std::tuple<>>);
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

TEST(tuple_prepend, one_type)
{
    using T = std::tuple<int, double>;
    static_assert(std::is_same_v<tTuplePrepend<T, std::string>, std::tuple<std::string, int, double>>);
}

TEST(tuple_prepend, multiple_types)
{
    using T = std::tuple<int, double>;
    static_assert(std::is_same_v<tTuplePrepend<T, std::string, char, float>, std::tuple<std::string, char, float, int, double>>);
}

TEST(tuple_prepend, zero_types)
{
    using T = std::tuple<int, double>;
    static_assert(std::is_same_v<tTuplePrepend<T>, std::tuple<int, double>>);
}

TEST(safe_tuple_element_t, in_range)
{
    using T = std::tuple<int, double, std::string>;
    static_assert(std::is_same_v<tSafeTupleElement<0, T>::type, int>);
    static_assert(std::is_same_v<tSafeTupleElement<1, T>::type, double>);
    static_assert(std::is_same_v<tSafeTupleElement<2, T>::type, std::string>);

    static_assert(std::is_same_v<tSafeTupleElementT<0, T>, int>);
    static_assert(std::is_same_v<tSafeTupleElementT<1, T>, double>);
    static_assert(std::is_same_v<tSafeTupleElementT<2, T>, std::string>);
}

TEST(safe_tuple_element_t, out_of_range)
{
    using T = std::tuple<int, double, std::string>;
    static_assert(std::is_same_v<tSafeTupleElement<3, T>::type, void>);
    static_assert(std::is_same_v<tSafeTupleElementT<3, T>, void>);
    static_assert(std::is_same_v<tSafeTupleElement<4, T>::type, void>);
    static_assert(std::is_same_v<tSafeTupleElement<100, T>::type, void>);
}

TEST(safe_tuple_element_t, empty_tuple)
{
    using T = std::tuple<>;
    static_assert(std::is_same_v<tSafeTupleElement<0, T>::type, void>);
    static_assert(std::is_same_v<tSafeTupleElement<1, T>::type, void>);
    static_assert(std::is_same_v<tSafeTupleElement<100, T>::type, void>);
}

} // namespace TuplePrefixTests

