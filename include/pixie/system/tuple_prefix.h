#pragma once

template<size_t... N, typename Tuple>
auto tTuplePrefixHelper(std::index_sequence<N...>, const Tuple&)
    -> std::tuple<std::tuple_element_t<N, Tuple>...>;

template<size_t N, typename Tuple> using tTuplePrefix =
    decltype(tTuplePrefixHelper(std::make_index_sequence<N>{}, std::declval<Tuple>()));