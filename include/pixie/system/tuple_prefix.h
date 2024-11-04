#pragma once

template<size_t... N, typename Tuple>
auto tTuplePrefixHelper(std::index_sequence<N...>, const Tuple&)
    -> std::tuple<std::tuple_element_t<N, Tuple>...>;

template<size_t N, typename Tuple> using tTuplePrefix =
    decltype(tTuplePrefixHelper(std::make_index_sequence<N>{}, std::declval<Tuple>()));

template<typename Tuple>
auto tTupleTakerFunctionHelper(const Tuple& tuple)
{
    return[]<size_t... I>(std::index_sequence<I...>)
    {
        return std::function<void(std::tuple_element_t<I, Tuple>...)>{};
    }(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

template<typename Tuple> using tTupleTakerFunction =
    decltype(tTupleTakerFunctionHelper(std::declval<Tuple>()));


template<size_t N, typename Tuple> using tTuplePrefixTakerFunction =
    tTupleTakerFunction<tTuplePrefix<N, Tuple>>;



