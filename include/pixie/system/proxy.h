#pragma once

template<class OWNER, class VALUE_TYPE, auto getter, auto setter> 
requires std::invocable<decltype(getter), OWNER&> 
      && std::invocable<decltype(setter), OWNER&, const VALUE_TYPE&>
      && std::convertible_to<std::invoke_result_t<decltype(getter), OWNER&>, VALUE_TYPE> 
class tProxy final
{
    OWNER& mOwner;
public:
    tProxy(OWNER& owner) : mOwner(owner) {}
    tProxy(const tProxy&) = delete;
    tProxy(tProxy&&) = delete;
    tProxy& operator=(const tProxy&) = delete;
    tProxy& operator=(tProxy&&) = delete;
    operator VALUE_TYPE() const { return getter(mOwner); }
    tProxy& operator=(const VALUE_TYPE& value)  { setter(mOwner, value);                  return *this; }
    tProxy& operator+=(const VALUE_TYPE& value) { setter(mOwner, getter(mOwner) + value); return *this; }
    tProxy& operator-=(const VALUE_TYPE& value) { setter(mOwner, getter(mOwner) - value); return *this; }
    tProxy& operator*=(const VALUE_TYPE& value) { setter(mOwner, getter(mOwner) * value); return *this; }
    tProxy& operator/=(const VALUE_TYPE& value) { setter(mOwner, getter(mOwner) / value); return *this; }

    template<class T> requires std::convertible_to<decltype(std::declval<VALUE_TYPE>() + std::declval<T>()), VALUE_TYPE>
    VALUE_TYPE operator+(const T& value) const { return getter(mOwner) + value; }

    template<class T> requires std::convertible_to<decltype(std::declval<VALUE_TYPE>() - std::declval<T>()), VALUE_TYPE>
    VALUE_TYPE operator-(const T& value) const { return getter(mOwner) - value; }

    template<class T> requires std::convertible_to<decltype(std::declval<VALUE_TYPE>() * std::declval<T>()), VALUE_TYPE>
    VALUE_TYPE operator*(const T& value) const { return getter(mOwner) * value; }

    template<class T> requires std::convertible_to<decltype(std::declval<VALUE_TYPE>() / std::declval<T>()), VALUE_TYPE>
    VALUE_TYPE operator/(const T& value) const { return getter(mOwner) / value; }

    bool operator==(const VALUE_TYPE& value) const { return getter(mOwner) == value; }
    bool operator!=(const VALUE_TYPE& value) const { return getter(mOwner) != value; }
    template<std::constructible_from<VALUE_TYPE> T> operator T() const { return getter(mOwner); }
};

