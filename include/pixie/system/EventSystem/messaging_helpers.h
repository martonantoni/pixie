#pragma once

namespace Messaging
{

template<size_t... N, typename Tuple>
auto tTuplePrefixHelper(std::index_sequence<N...>, const Tuple&)
{
    return std::tuple<std::tuple_element_t<N, Tuple>...>{};
}

template<size_t N, typename Tuple> using tTuplePrefix =
    decltype(tTuplePrefixHelper(std::make_index_sequence<N>{}, std::declval<Tuple>()));

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
auto tMessageDispatchersHelper(std::index_sequence<N...>, const Tuple&)
{
    return std::variant<tPrefixTakerFunction<N, Tuple>...>{};
}

template<class Tuple> using tMessageDispatchers =
    decltype(tMessageDispatchersHelper(std::make_index_sequence<std::tuple_size_v<Tuple> + 1>{}, std::declval<Tuple>()));


}