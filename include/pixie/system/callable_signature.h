#pragma once

template<class C> struct cCallableSignature;

template<class R, class C, class... Args>
struct cCallableSignature<R(C::*)(Args...)>
{
    using ReturnType = R;
    using Arguments = std::tuple<Args...>;
    static constexpr size_t numberOfArguments = sizeof...(Args);
    static bool isConst = false;
    static constexpr bool available = true;
};

template<class R, class C, class... Args>
struct cCallableSignature<R(C::*)(Args...) const>
{
    using ReturnType = R;
    using Arguments = std::tuple<Args...>;
    static constexpr size_t numberOfArguments = sizeof...(Args);
    static bool isConst = true;
    static constexpr bool available = true;
};
