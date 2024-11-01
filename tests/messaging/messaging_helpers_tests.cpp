#include "messaging_tests_common.h"


namespace MessageSystemHelpersTests
{

using namespace Messaging;

TEST(message_system_helpers, tuple_prefix)
{
    using T = std::tuple<int, double, std::string, char>;

    static_assert(std::is_same_v<tTuplePrefix<1, T>, std::tuple<int>>);
    static_assert(std::is_same_v<tTuplePrefix<2, T>, std::tuple<int, double>>);
    static_assert(std::is_same_v<tTuplePrefix<3, T>, std::tuple<int, double, std::string>>);
    static_assert(std::is_same_v<tTuplePrefix<4, T>, std::tuple<int, double, std::string, char>>);

    // 0 must be empty tuple:
    static_assert(std::is_same_v<tTuplePrefix<0, T>, std::tuple<>>);
}

TEST(message_system_helpers, prefix_taker_function)
{
    using T = std::tuple<int, double, std::string, char>;

    static_assert(std::is_same_v<tPrefixTakerFunction<1, T>, std::function<void(int)>>);
    static_assert(std::is_same_v<tPrefixTakerFunction<2, T>, std::function<void(int, double)>>);
    static_assert(std::is_same_v<tPrefixTakerFunction<3, T>, std::function<void(int, double, std::string)>>);
    static_assert(std::is_same_v<tPrefixTakerFunction<4, T>, std::function<void(int, double, std::string, char)>>);

    // 0 must be empty function:
    static_assert(std::is_same_v<tPrefixTakerFunction<0, T>, std::function<void()>>);
}

TEST(message_system_helpers, message_dispatchers_type)
{
    using T = std::tuple<int, double, std::string, char>;

    static_assert(std::is_same_v<tMessageDispatchers<T>, std::variant<
        std::function<void()>,
        std::function<void(int)>,
        std::function<void(int, double)>,
        std::function<void(int, double, std::string)>,
        std::function<void(int, double, std::string, char)>>>);
}


} // namespace MessageSystemHelpersTests
