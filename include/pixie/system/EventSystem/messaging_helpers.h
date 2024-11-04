#pragma once

namespace Messaging
{
//
//template<class Tuple> using tMessageListeners =
//    decltype(tMessageListenersHelper(std::make_index_sequence<std::tuple_size_v<Tuple> + 1>{}, std::declval<Tuple>()));
//
//template<typename Callable, typename Variant> struct tIsMessageListenerHelper;
//
//template<typename Callable, typename... Ts>
//struct tIsMessageListenerHelper<Callable, std::variant<Ts...>> :
//    public std::disjunction<std::is_same<Callable, Ts>...> {};
//
//template<typename Callable, typename Tuple> constexpr bool tIsMessageListener = 
//    tIsMessageListenerHelper<Callable, tMessageListeners<Tuple>>::value;

}