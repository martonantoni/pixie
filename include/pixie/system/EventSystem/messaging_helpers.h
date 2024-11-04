#pragma once

namespace Messaging
{

template<size_t N, typename Tuple>
auto tPrefixTakerFunctionHelper(const Tuple& tuple)
{
    return[]<size_t... I>(std::index_sequence<I...>)
    {
        return std::function<void(std::tuple_element_t<I, Tuple>...)>{};
    }(std::make_index_sequence<N>{});
}

template<size_t N, typename Tuple> using tPrefixTakerFunction =
    decltype(tPrefixTakerFunctionHelper<N>(std::declval<Tuple>()));

template<size_t... N, typename Tuple>
auto tMessageListenersHelper(std::index_sequence<N...>, const Tuple&)
    -> std::variant<tPrefixTakerFunction<N, Tuple>...>;

template<class Tuple> using tMessageListeners =
    decltype(tMessageListenersHelper(std::make_index_sequence<std::tuple_size_v<Tuple> + 1>{}, std::declval<Tuple>()));

template<typename Callable, typename Variant> struct tIsMessageListenerHelper;

template<typename Callable, typename... Ts>
struct tIsMessageListenerHelper<Callable, std::variant<Ts...>> :
    public std::disjunction<std::is_same<Callable, Ts>...> {};

template<typename Callable, typename Tuple> constexpr bool tIsMessageListener = 
    tIsMessageListenerHelper<Callable, tMessageListeners<Tuple>>::value;

}