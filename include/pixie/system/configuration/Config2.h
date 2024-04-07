#pragma once

class cConfig2 : public cIntrusiveRefCount
{
    using cValue = std::variant<int, double, std::string, bool, tIntrusivePtr<cConfig2>>;
    using cValueMap = std::unordered_map<std::string, cValue>;
    using cValueArray = std::vector<cValue>;
    using cValues = std::variant<std::monostate, cValueMap, cValueArray>;
    template<typename T>
    using tGetRV = std::conditional_t<std::is_same_v<std::remove_cvref_t<T>, cConfig2>, tIntrusivePtr<cConfig2>, T>;
    cValues mValues;
    std::pair<cConfig2*, std::string> leafConfig(const std::string& keyPath, bool canCreateSubConfig);
    std::pair<cConfig2*, std::string> leafConfig(const std::string& keyPath) const;
    template<class T> static tGetRV<T> extract(const cValue& value);
    template<class TO, class FROM> static TO convert(const FROM& value);
    template<class T> tGetRV<T> _get(const std::string& key, const std::optional<tGetRV<T>>& defaultValue) const;
    template<class T> void _set(const std::string& key, T&& value);
    static const cValue& _extractValue(const cValue& value) { return value; }
    static const cValue& _extractValue(const std::pair<const std::string, cValue>& pair) { return pair.second; }
public:
    cConfig2() = default;
    void makeArray(); // works only if empty
    template<class T> void set(const std::string& keyPath, T&& value);
    template<class T> void set(int index, T&& value);
    template<class T> void push(T&& value);
    bool empty() const;
    int numberOfValues() const;
    int numberOfSubConfigs() const;
    template<class T> tGetRV<T> 
        get(const std::string& keyPath, std::optional<tGetRV<T>> defaultValue = std::optional<tGetRV<T>>()) const;
    template<class T> tGetRV<T> 
        get(int index, std::optional<tGetRV<T>> defaultValue = std::optional<tGetRV<T>>()) const;
    template<class C> void forEachValue(const C& callable) const;

    tIntrusivePtr<cConfig2> getSubConfig(const std::string& keyPath) const; // creates if doesn't exist
    template<class C> void forEachSubConfig(const C& callable) const;
    bool isArray() const;
};

inline bool cConfig2::empty() const
{
    return std::visit([](const auto& values) -> bool
        {
            if constexpr (!std::is_same_v<std::decay_t<decltype(values)>, std::monostate>)
            {
                return values.empty();
            }
            else
            {
                return true;
            }
        }, mValues);
}

inline bool cConfig2::isArray() const
{
    return std::holds_alternative<cValueArray>(mValues);
}

inline int cConfig2::numberOfValues() const
{
    return std::visit([](const auto& values) -> int
        { 
            if constexpr (!std::is_same_v<std::decay_t<decltype(values)>, std::monostate>)
            {
                return std::ranges::count_if(values, [](const auto& value)
                    {
                        return !std::holds_alternative<tIntrusivePtr<cConfig2>>(_extractValue(value));
                    });
            }
            else
            {
                return 0;
            }
        }, mValues);
}

inline int cConfig2::numberOfSubConfigs() const
{
    return std::visit([](const auto& values) -> int
        {
            if constexpr (!std::is_same_v<std::decay_t<decltype(values)>, std::monostate>)
            {
                return std::ranges::count_if(values, [](const auto& value)
                    {
                        return std::holds_alternative<tIntrusivePtr<cConfig2>>(_extractValue(value));
                    });
            }
            else
            {
                return 0;
            }
        }, mValues);
}

template<class T> void cConfig2::_set(const std::string& key, T&& value)
{
    std::visit(
        [&, this](auto& values)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueMap>)
            {
                values[key] = std::forward<T>(value);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>)
            {
                int index = std::stoi(std::string(key));
                if (index >= values.size())
                {
                    values.resize(index + 1);
                }
                values[index] = std::forward<T>(value);
            }
            else // std::monostate
            {
                mValues = cValueMap();
                (std::get<1>(mValues))[key] = std::forward<T>(value);
            }
        }, mValues);
}

template<class T> void cConfig2::set(const std::string& keyPath, T&& value)
{
    auto [config, key] = leafConfig(keyPath, true);
    config->_set(key, std::forward<T>(value));
}

template<class T> void cConfig2::set(int index, T&& value)
{
    std::visit(
        [&, this](auto& values)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueMap>)        // storing in map
            {
                values[std::to_string(index)] = std::forward<T>(value);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>) // storing in array
            {
                if (index >= values.size())
                {
                    values.resize(index + 1);
                }
                values[index] = std::forward<T>(value);
            }
            else                  // std::monostate
            {
                mValues = cValueArray();
                set(index, std::forward<T>(value));
            }
        }, mValues);
}


template<class TO, class FROM> static TO cConfig2::convert(const FROM& value)
{
    if constexpr (std::is_same_v<TO, std::string>)               // conveert to string
    {
        if constexpr (std::is_same_v<FROM, bool>)
            return value ? "true" : "false";
        return std::to_string(value);
    }
    else if constexpr (std::is_same_v<TO, int>)                  // convert to int
    {
        if constexpr (std::is_same_v<FROM, bool>)
            return value ? 1 : 0;
        if constexpr (std::is_same_v<FROM, double>)
            return static_cast<int>(value);
        if constexpr (std::is_same_v<FROM, std::string>)
            return std::stoi(value);
    }
    else if constexpr (std::is_same_v<TO, double>)               // convert to double
    {
        if constexpr (std::is_same_v<FROM, bool>)
            return value ? 1.0 : 0.0;
        if constexpr (std::is_same_v<FROM, int>)
            return static_cast<double>(value);
        if constexpr (std::is_same_v<FROM, std::string>)
            return std::stod(value);
    }
    else if constexpr (std::is_same_v<TO, bool>)                 // convert to bool
    {
        if constexpr (std::is_same_v<FROM, int>)
            return value != 0;
        if constexpr (std::is_same_v<FROM, double>)
            return value != 0.0;
        if constexpr (std::is_same_v<FROM, std::string>)
            return value == "true" || value == "TRUE" || value == "True";
    }
    throw std::runtime_error("Unsupported conversion");
}

template<class T> static cConfig2::tGetRV<T> cConfig2::extract(const cValue& value)
{
    return std::visit([](const auto& value) -> cConfig2::tGetRV<T>
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, tGetRV<T>>)
            {
                return value;
            }
            else
            {
                return convert<tGetRV<T>>(value);
            }
        }, value);
}

template<class T> cConfig2::tGetRV<T> cConfig2::_get(const std::string& key, const std::optional<tGetRV<T>>& defaultValue) const
{
    return std::visit(
        [&](const auto& values) -> cConfig2::tGetRV<T>
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueMap>)  // retrieving from map
            {
                auto it = values.find(key);
                if (it != values.end())
                {
                    return extract<T>(it->second);
                }
                else if (defaultValue.has_value())
                {
                    return defaultValue.value();
                }
                else
                {
                    throw std::runtime_error("Key not found: " + key);
                }
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>) // retrieving from array
            {
                int index = std::stoi(std::string(key));
                if (index < values.size())
                {
                    return extract<T>(values[index]);
                }
                else if (defaultValue.has_value())
                {
                    return defaultValue.value();
                }
                else
                {
                    throw std::runtime_error("Index out of range: " + key);
                }
            }
            else // std::monostate
            {
                if (defaultValue.has_value())
                {
                    return defaultValue.value();
                }
                else
                {
                    throw std::runtime_error("Key not found: " + key);
                }
            }
        }, mValues);
}

template<class T> cConfig2::tGetRV<T> cConfig2::get(const std::string& keyPath, std::optional<tGetRV<T>> defaultValue) const
{
    const auto& [config, key] = leafConfig(keyPath);
    if(!config)
    {
        if (defaultValue.has_value())
        {
            return defaultValue.value();
        }
        else
        {
            throw std::runtime_error("Key not found: " + keyPath);
        }
    }
    return config->_get<T>(key, defaultValue);
}

template<class T> cConfig2::tGetRV<T> cConfig2::get(int index, std::optional<tGetRV<T>> defaultValue) const
{
    return std::visit(
        [&](const auto& values) -> cConfig2::tGetRV<T>
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueMap>)  // retrieving from map
            {
                return get<T>(std::to_string(index), defaultValue);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>) // retrieving from array
            {
                if (index < values.size())
                {
                    return extract<T>(values[index]);
                }
                else if (defaultValue.has_value())
                {
                    return defaultValue.value();
                }
                else
                {
                    throw std::runtime_error("Index out of range: " + std::to_string(index));
                }
            }
            else // std::monostate
            {
                if (defaultValue.has_value())
                {
                    return defaultValue.value();
                }
                else
                {
                    throw std::runtime_error("Index out of range: " + std::to_string(index));
                }
            }
        }, mValues);
}


