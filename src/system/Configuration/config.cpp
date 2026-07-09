#include "StdAfx.h"

#include "pixie/system/configuration/config.h"

std::pair<cConfig*, std::string> cConfig::leafConfig(const std::string& keyPath, bool canCreateSubConfig)
{
    auto dotPos = keyPath.find('.');
    if (dotPos == std::string::npos)
        return { this, keyPath };
    auto subKey = keyPath.substr(0, dotPos);
    auto subConfig = _get<std::shared_ptr<cConfig>>(subKey, nullptr);
    if (!subConfig)
    {
        if (!canCreateSubConfig)
            return { nullptr, std::string() };
        subConfig = std::make_shared<cConfig>();
        _set(subKey, subConfig);
    }
    return subConfig->leafConfig(keyPath.substr(dotPos + 1), canCreateSubConfig);
}

std::pair<cConfig*, std::string> cConfig::leafConfig(const std::string& keyPath) const
{
// const_cast is safe here, the second parameter "false" means that we won't modify the config
//         by creating a new subconfig if it doesn't exist
    return const_cast<cConfig*>(this)->leafConfig(keyPath, false);
}

bool cConfig::has(const std::string& keyPath) const
{
    auto [config, key] = leafConfig(keyPath);
    return config && config->_has(key);
}

bool cConfig::_has(const std::string& key) const
{
    if(std::holds_alternative<cValueMap>(mValues))
    {
        const cValueMap& values = std::get<cValueMap>(mValues);
        return values.find(key) != values.end();
    }
    return false;
}

void cConfig::makeArray()
{
    std::visit([this](auto& values)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueMap>)
            {
                if(values.empty())
                    mValues = cValueArray{};
                throw std::runtime_error("Cannot convert map to array");
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>)
            {
                return; // already an array
            }
            else // std::monostate
            {
                mValues = cValueArray{};
            }
        }
        , mValues);
}

bool cConfig::operator==(const cConfig& other) const
{
    auto valueEquals = [](const cValue& lhs, const cValue& rhs) -> bool
        {
            return std::visit([](const auto& a, const auto& b) -> bool
                {
                    using A = std::decay_t<decltype(a)>;
                    using B = std::decay_t<decltype(b)>;

                    if constexpr (!std::is_same_v<A, B>)
                    {
                        return false;
                    }
                    else if constexpr (std::is_same_v<A, cConfigPtr>)
                    {
                        return *a == *b;
                    }
                    else
                    {
                        return a == b;
                    }
                }, lhs, rhs);
        };

    return std::visit([&](const auto& values, const auto& otherValues) -> bool
        {
            using Values = std::decay_t<decltype(values)>;
            using OtherValues = std::decay_t<decltype(otherValues)>;

            if constexpr (!std::is_same_v<Values, OtherValues>)
            {
                return false;
            }
            else if constexpr (std::is_same_v<Values, cValueMap>)
            {
                if (values.size() != otherValues.size())
                    return false;

                for (const auto& [key, value] : values)
                {
                    auto otherIt = otherValues.find(key);
                    if (otherIt == otherValues.end())
                        return false;

                    if (!valueEquals(value, otherIt->second))
                        return false;
                }

                return true;
            }
            else if constexpr (std::is_same_v<Values, cValueArray>)
            {
                if (values.size() != otherValues.size())
                    return false;

                for (size_t i = 0; i < values.size(); ++i)
                {
                    if (!valueEquals(values[i], otherValues[i]))
                        return false;
                }

                return true;
            }
            else // std::monostate
            {
                return true;
            }
        }, mValues, other.mValues);
}