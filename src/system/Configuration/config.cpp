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
    if (mValues.index() != other.mValues.index())
        return false;
    return std::visit([&other](const auto& values)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueMap>)
            {
                const cValueMap& otherValueMap = std::get<cValueMap>(other.mValues);
                if (values.size() != otherValueMap.size())
                    return false;
                for (const auto& [key, value] : values)
                {
                    auto otherIt = otherValueMap.find(key);
                    if (otherIt == otherValueMap.end())
                        return false;
                    const cValue& otherValue = otherIt->second;
                    if (value.index() != otherValue.index())
                        return false;

                    if (std::holds_alternative<cConfigPtr>(value))
                    {
                        auto it = otherValueMap.find(key);
                        if (it == otherValueMap.end())
                            return false;
                        return std::get<cConfigPtr>(value)->operator==(*std::get<cConfigPtr>(it->second));
                    }
                    else
                    {
                        if(!std::visit([&value, &otherValue](const auto& value)
                            {
                                return std::get<std::decay_t<decltype(value)>>(otherValue) == value;
                            }
                            , value))
                        {
                            return false;
                        }
                    }
                }
                return true;
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(values)>, cValueArray>)
            {
                const cValueArray& otherValueArray = std::get<cValueArray>(other.mValues);
                if (values.size() != otherValueArray.size())
                    return false;
                for (size_t i = 0; i < values.size(); ++i)
                {
                    const cValue& value = values[i];
                    const cValue& otherValue = otherValueArray[i];
                    if(value.index() != otherValue.index())
                        return false;
                    if (std::holds_alternative<cConfigPtr>(values[i]))
                    {
                        if (!std::get<cConfigPtr>(values[i])->operator==(*std::get<cConfigPtr>(otherValueArray[i])))
                            return false;
                    }
                    else
                    {
                        if (!std::visit([&values, i, &otherValueArray](const auto& value)
                            {
                                return std::get<std::decay_t<decltype(value)>>(otherValueArray[i])
                                    == value;
                            }
                            , values[i]))
                        {
                            return false;
                        }
                    }
                }
                return true;
            }
            else // std::monostate
            {
                return true;
            }
        }, mValues);    
} 