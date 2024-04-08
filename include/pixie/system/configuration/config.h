#pragma once

class cConfig : public cIntrusiveRefCount
{
    using cValue = std::variant<int, double, std::string, bool, tIntrusivePtr<cConfig>>;
    using cValueMap = std::unordered_map<std::string, cValue>;
    using cValueArray = std::vector<cValue>;
    using cValues = std::variant<std::monostate, cValueMap, cValueArray>;
    template<typename T>
    using tGetRV = std::conditional_t<std::is_same_v<std::remove_cvref_t<T>, cConfig>, tIntrusivePtr<cConfig>, T>;
    cValues mValues;
    std::pair<cConfig*, std::string> leafConfig(const std::string& keyPath, bool canCreateSubConfig);
    std::pair<cConfig*, std::string> leafConfig(const std::string& keyPath) const;
    template<class T> static tGetRV<T> extract(const cValue& value);
    template<class TO, class FROM> static TO convert(const FROM& value);
    template<class T> tGetRV<T> _get(const std::string& key, const std::optional<tGetRV<T>>& defaultValue) const;
    template<class T> void _set(const std::string& key, T&& value);
    static const cValue& _extractValue(const cValue& value) { return value; }
    static const cValue& _extractValue(const std::pair<const std::string, cValue>& pair) { return pair.second; }
public:
    cConfig() = default;
    virtual ~cConfig() = default;
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

    tIntrusivePtr<cConfig> getSubConfig(const std::string& keyPath) const { return get<tIntrusivePtr<cConfig>>(keyPath); }
    tIntrusivePtr<cConfig> getSubOrEmptyConfig(const std::string& keyPath) const { return get<tIntrusivePtr<cConfig>>(keyPath, make_intrusive_ptr<cConfig>()); }
    tIntrusivePtr<cConfig> createSubConfig(const std::string& key);
    template<class C> void forEachSubConfig(const C& callable) const;
    bool isArray() const;
    template<class Visitor> void visit(Visitor&& visitor) const;
    template<class C> void forEachString(const C& callable) const; // callable: void (const std::string& key, const std::string& value)
    template<class C> void forEachInt(const C& callable) const; // callable: void (const std::string& key, int value)
};

inline bool cConfig::empty() const
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

inline bool cConfig::isArray() const
{
    return std::holds_alternative<cValueArray>(mValues);
}

inline int cConfig::numberOfValues() const
{
    return std::visit([](const auto& values) -> int
        { 
            if constexpr (!std::is_same_v<std::decay_t<decltype(values)>, std::monostate>)
            {
                return std::ranges::count_if(values, [](const auto& value)
                    {
                        return !std::holds_alternative<tIntrusivePtr<cConfig>>(_extractValue(value));
                    });
            }
            else
            {
                return 0;
            }
        }, mValues);
}

inline int cConfig::numberOfSubConfigs() const
{
    return std::visit([](const auto& values) -> int
        {
            if constexpr (!std::is_same_v<std::decay_t<decltype(values)>, std::monostate>)
            {
                return std::ranges::count_if(values, [](const auto& value)
                    {
                        return std::holds_alternative<tIntrusivePtr<cConfig>>(_extractValue(value));
                    });
            }
            else
            {
                return 0;
            }
        }, mValues);
}

template<class T> void cConfig::_set(const std::string& key, T&& value)
{
    std::visit(
        [value = std::forward<T>(value), this, &key](auto& values) 
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueMap>)
            {
                values[key] = value;
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>)
            {
                int index = std::stoi(std::string(key));
                if (index >= values.size())
                {
                    values.resize(index + 1);
                }
                values[index] = value;
            }
            else // std::monostate
            {
                mValues = cValueMap();
                (std::get<1>(mValues))[key] = value;
            }
        }, mValues);
}

template<class T> void cConfig::set(const std::string& keyPath, T&& value)
{
    auto [config, key] = leafConfig(keyPath, true);
    config->_set(key, std::forward<T>(value));
}

template<class T> void cConfig::set(int index, T&& value)
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

template<class T> void cConfig::push(T&& value)
{
    std::visit(
        [&, this](auto& values)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>)
            {
                values.push_back(std::forward<T>(value));
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, std::monostate>)
            {
                mValues = cValueArray();
                push(std::forward<T>(value));
            }
            else
            {
                throw std::runtime_error("Cannot push to a map");
            }
        }, mValues);
}

template<class TO, class FROM> static TO cConfig::convert(const FROM& value)
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

template<class T> static cConfig::tGetRV<T> cConfig::extract(const cValue& value)
{
    return std::visit([](const auto& value) -> cConfig::tGetRV<T>
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

template<class T> cConfig::tGetRV<T> cConfig::_get(const std::string& key, const std::optional<tGetRV<T>>& defaultValue) const
{
    return std::visit(
        [&](const auto& values) -> cConfig::tGetRV<T>
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

template<class T> cConfig::tGetRV<T> cConfig::get(const std::string& keyPath, std::optional<tGetRV<T>> defaultValue) const
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

template<class T> cConfig::tGetRV<T> cConfig::get(int index, std::optional<tGetRV<T>> defaultValue) const
{
    return std::visit(
        [&](const auto& values) -> cConfig::tGetRV<T>
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

template<class Visitor> void cConfig::visit(Visitor&& visitor) const
{
    std::visit([&](auto& values)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueMap>)
            {
                for (auto& [key, value] : values)
                {
                    std::visit([&](auto& actualValue)
                        {
                            if constexpr (std::is_invocable_r_v<void, Visitor, const std::string&, decltype(value)>)
                            {
                                visitor(key, actualValue);
                            }
                            else
                            {
                                // if the value is a subconfig, we can silently ignore it:
                                if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(actualValue)>, tIntrusivePtr<cConfig>>)
                                {
                                    throw std::runtime_error("Unsupported visitor signature");
                                }
                            }
                        }, value);
                }
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>)
            {
                for (int i = 0; i < values.size(); ++i)
                {
                    std::visit([&](auto& actualValue)
                        {
                            if constexpr (std::is_invocable_r_v<void, Visitor, int, decltype(actualValue)>)
                            {
                                visitor(i, actualValue);
                            }
                            else
                            {
                                visitor(std::to_string(i), actualValue);
                            }
                        }, values[i]);
                }
            }
        }, mValues);
}

template<class C> void cConfig::forEachString(const C& callable) const
{
    if constexpr (std::is_invocable_v<C, const std::string&, const std::string&>)
    {
        visit([&](const std::string& key, const auto& value)
            {
                if constexpr (std::is_same_v<std::remove_cvref_t<decltype(value)>, std::string>)
                {
                    callable(key, value);
                }
            });
    }
    else
    {
        visit([&](int idx, const auto& value)
        {
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(value)>, std::string>)
            {
                callable(idx, value);
            }
        });
    }
}

template<class C> void cConfig::forEachInt(const C& callable) const
{
    visit([&](const std::string& key, const auto& value)
        {
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(value)>, int>)
            {
                callable(key, value);
            }
        });
}

template<class C> void cConfig::forEachSubConfig(const C& callable) const
{
    visit([&](const std::string& key, const auto& value)
        {
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(value)>, tIntrusivePtr<cConfig>>)
            {
                if constexpr (std::is_invocable_r_v<void, C, std::string, const cConfig&>)
                {
                    callable(key, *value);
                }
                else if constexpr (std::is_invocable_r_v<void, C, std::string, tIntrusivePtr<cConfig>>)
                {
                    callable(key, value);
                }
            }
        });
}

inline tIntrusivePtr<cConfig> cConfig::createSubConfig(const std::string& key)
{
    tIntrusivePtr<cConfig> subConfig;
    std::visit([&](auto& values)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueMap>)  // retrieving from map
            {
                if(auto it = values.find(key); it != values.end())
                {
                    subConfig = extract<tIntrusivePtr<cConfig>>(it->second);
                }
                else
                {
                    auto [i, added] = values.emplace(key, subConfig = make_intrusive_ptr<cConfig>());
                }
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>) // retrieving from array
            {
                int index = std::stoi(std::string(key));
                if(index < values.size())
                {
                    subConfig = extract<tIntrusivePtr<cConfig>>(values[index]);
                }
                else
                {
                    values.resize(index + 1);
                    values[index] = subConfig = make_intrusive_ptr<cConfig>();
                }
            }
            else // std::monostate
            {
                mValues = cValueMap();
                subConfig = createSubConfig(key);
            }
        }, mValues);
    return subConfig;
}