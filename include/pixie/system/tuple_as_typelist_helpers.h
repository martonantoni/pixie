#pragma once

//
// Helpers for tuple manipulations, when treating tuple as a typelist
//
//
// tTuplePrefix<Tuple, N> - gives you a tuple type which is the prefix of Tuple of length N
//        Example: tTuplePrefix<2, std::tuple<int, double, std::string>> is std::tuple<int, double>
//
// tTuplePostfix<Tuple, N> - gives you a tuple type which is the postfix of Tuple of length N
//        Example: tTuplePostfix<std::tuple<int, double, std::string>, 2> is std::tuple<double, std::string>
// 
// tTuplePrefixTakerFunction<T, Tuple> - gives you a std::function with arguments of the prefix of Tuple of length T
//        Example: tTuplePrefixTakerFunction<2, std::tuple<int, double, std::string>> 
//                 is std::function<void(int, double)>
//
// 
// tTuplePrepend<Tuple, Ts...> - gives you a tuple type which is Tuple with Ts... prepended
//        Example: tTuplePrepend<std::tuple<int, double>, std::string, char> 
//                 is std::tuple<std::string, char, int, double>
//
// 
// tSafeTupleElement<I, Tuple> - like std::tuple_element_t<I, Tuple>::type but if I is out of range, gives void
//        Example: tSafeTupleElement<2, std::tuple<int, double>> is void
//                 tSafeTupleElement<1, std::tuple<int, double>> is double
// tSafeTupleElementT<I, Tuple> - tSafeTupleElement<I, Tuple>::type
//
//
// applyTail(callable, tuple, N) - calls callable with the last N elements of tuple as arguments
//
// applyHead(callable, tuple, N) - calls callable with the first N elements of tuple as arguments
//
//
//  isInTuple<Tuple, T> - true if T is in Tuple, false otherwise


////////////////////////////////////////////////////////////////////////////



//                  tTuplePrefix


template<typename Tuple, size_t... N>
auto tTuplePrefixHelper(std::index_sequence<N...>)
    -> std::tuple<std::tuple_element_t<N, Tuple>...>;

template<typename Tuple, size_t N> using tTuplePrefix =
    decltype(tTuplePrefixHelper<Tuple>(std::make_index_sequence<N>{}));


//                 tTuplePostfix


template<typename Tuple, size_t... N>
auto tTuplePostfixHelper(std::index_sequence<N...>)
    -> std::tuple<std::tuple_element_t<std::tuple_size<Tuple>::value - sizeof...(N) + N, Tuple>...>;

template<typename Tuple, size_t N> using tTuplePostfix =
    decltype(tTuplePostfixHelper<Tuple>(std::make_index_sequence<std::max(0, static_cast<int>(N))>{}));


//                 tTuplePrefixTakerFunction


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
    tTupleTakerFunction<tTuplePrefix<Tuple, N>>;


//                tTuplePrepend


template <typename Tuple, typename... Ts>
using tTuplePrepend = decltype(std::tuple_cat(std::declval<std::tuple<Ts...>>(), std::declval<Tuple>()));


//                tSafeTupleElement


template<size_t I, typename Tuple, bool isInRange = I < std::tuple_size_v<Tuple>>
struct tSafeTupleElement
{
    using type = void;
};

template<size_t I, typename Tuple>
struct tSafeTupleElement<I, Tuple, true>
{
    using type = std::tuple_element_t<I, Tuple>;
};

template<size_t I, typename Tuple>
using tSafeTupleElementT = typename tSafeTupleElement<I, Tuple>::type;


//               applyTail


template<typename F, typename Tuple, size_t... I>
decltype(auto) applyTailHelper(F&& f, Tuple&& t, std::index_sequence<I...>)
{
    return std::invoke(std::forward<F>(f),
        std::get<std::tuple_size_v<std::remove_reference_t<Tuple>> - sizeof...(I) + I>(std::forward<Tuple>(t))...);
}

template<size_t N, typename F, typename Tuple>
decltype(auto) applyTail(F&& f, Tuple&& t)
{
    return applyTailHelper(std::forward<F>(f), std::forward<Tuple>(t), std::make_index_sequence<N>{});
}


//              applyHead


template<typename F, typename Tuple, size_t... I>
decltype(auto) applyHeadHelper(F&& f, Tuple&& t, std::index_sequence<I...>)
{
    return std::invoke(std::forward<F>(f),
        std::get<I>(std::forward<Tuple>(t))...);
}

template<size_t N, typename F, typename Tuple>
decltype(auto) applyHead(F&& f, Tuple&& t)
{
    return applyHeadHelper(std::forward<F>(f), std::forward<Tuple>(t), std::make_index_sequence<N>{});
}


//             is_invocable_with_tuple


template <typename C, typename TTuple>
struct is_invocable_with_tuple;

template <typename C, typename... Args>
struct is_invocable_with_tuple<C, std::tuple<Args...>>
    : std::is_invocable<C, Args...> {
};


//             isInTuple

template<typename Tuple, typename T>
struct isInTupleHelper {
private:
    template<std::size_t... Is>
    static constexpr bool check(std::index_sequence<Is...>) {
        return (std::is_same_v<std::tuple_element_t<Is, Tuple>, T> || ...);
    }

public:
    static constexpr bool value = check(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
};

template<typename Tuple, typename T>
inline constexpr bool isInTuple = isInTupleHelper<Tuple, T>::value;