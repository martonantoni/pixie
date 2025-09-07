#pragma once

template<class C> struct cSignatureExtractor;

template<class R, class C, class... Args>
struct cSignatureExtractor<R(C::*)(Args...)>
{
    using ReturnType = R;
    using Arguments = std::tuple<Args...>;
    using DecayedArguments = std::tuple<std::decay_t<Args>...>;
    using Signature = R(Args...);
    static constexpr size_t numberOfArguments = sizeof...(Args);
    static constexpr bool isConst = false;
    static constexpr bool available = true;
};

template<class R, class C, class... Args>
struct cSignatureExtractor<R(C::*)(Args...) const>
{
    using ReturnType = R;
    using Arguments = std::tuple<Args...>;
    using DecayedArguments = std::tuple<std::decay_t<Args>...>;
    using Signature = R(Args...);
    static constexpr size_t numberOfArguments = sizeof...(Args);
    static constexpr bool isConst = true;
    static constexpr bool available = true;
};

template<class R, class... Args>
struct cSignatureExtractor<R(*)(Args...)> 
{
    using ReturnType = R;
    using Arguments = std::tuple<Args...>;
    using DecayedArguments = std::tuple<std::decay_t<Args>...>;
    using Signature = R(Args...);
    static constexpr size_t numberOfArguments = sizeof...(Args);
    static constexpr bool available = true;
};

template<class R, class... Args>
struct cSignatureExtractor<R(Args...)>
{
    using ReturnType = R;
    using Arguments = std::tuple<Args...>;
    using DecayedArguments = std::tuple<std::decay_t<Args>...>;
    using Signature = R(Args...);
    static constexpr size_t numberOfArguments = sizeof...(Args);
    static constexpr bool available = true;
};

template<class C, class = void> struct cSignatureExtractorHelper; 

template<class C>
struct cSignatureExtractorHelper<C, std::void_t<decltype(&C::operator())>> 
{
    using type = cSignatureExtractor<decltype(&C::operator())>;
};

template<class C>
struct cSignatureExtractorHelper<C, std::enable_if_t<std::is_function_v<std::remove_pointer_t<C>>>> 
{
    using type = cSignatureExtractor<C>;
};

template<class C>
using cCallableSignature = typename cSignatureExtractorHelper<C>::type;
