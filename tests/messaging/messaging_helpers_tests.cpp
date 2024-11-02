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

TEST(message_system_helpers, message_listeners_type)
{
    using T = std::tuple<int, double, std::string, char>;

    static_assert(std::is_same_v<tMessageListeners<T>, std::variant<
        std::function<void()>,
        std::function<void(int)>,
        std::function<void(int, double)>,
        std::function<void(int, double, std::string)>,
        std::function<void(int, double, std::string, char)>>>);
}

//void test_function()
//{
//
//}

template<typename Tuple>
class tMessageDispatcher
{
public:
    void dispatch(const Messaging::tMessageListeners<Tuple>& listeners, const Tuple& message)
    {
        std::visit(
            [&message](auto&& listener) 
            { 
                [&]<size_t... I>(std::index_sequence<I...>)
                {
                    ([&]<size_t J>()
                    {
                        if constexpr (std::is_same_v<std::decay_t<decltype(listener)>, tPrefixTakerFunction<J, Tuple>>)
                        {
                            [&]<size_t... K>(std::index_sequence<K...>)
                            {
                                listener(std::get<K>(message)...);
                            }(std::make_index_sequence<J>());
                        }
                    }.template operator()<I>(), ...);
                }(std::make_index_sequence<std::tuple_size_v<Tuple>>());
            }, listeners);
    };
};

TEST(message_system_helpers, dispatcher)
{
    tMessageListeners<std::tuple<int, double, std::string, char>> listeners;

    listeners = [](int a, double d) { std::cout << a << " " << d << std::endl; };

    tMessageDispatcher<std::tuple<int, double, std::string, char>> dispatcher;

    dispatcher.dispatch(listeners, std::tuple<int, double, std::string, char>{1, 2.0, "hello"s, 'c'});

    listeners = [](int a, double d, const std::string& t) { std::cout << a << " " << d << " " << t << std::endl; };

    dispatcher.dispatch(listeners, std::tuple<int, double, std::string, char>{1, 2.0, "hello"s, 'c'});

    listeners = []() { std::cout << "empty" << std::endl; };

    dispatcher.dispatch(listeners, std::tuple<int, double, std::string, char>{1, 2.0, "hello"s, 'c'});
}

// std::any ----->  std::tuple<int, double, std::string, char>

} // namespace MessageSystemHelpersTests
