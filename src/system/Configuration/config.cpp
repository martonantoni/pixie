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
    return const_cast<cConfig*>(this)->leafConfig(keyPath, false);
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