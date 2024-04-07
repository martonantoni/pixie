#include "StdAfx.h"

#include "pixie/system/configuration/config2.h"

std::pair<cConfig2&, std::string> cConfig2::leafConfig(const std::string& keyPath, bool canCreateSubConfig)
{
    auto dotPos = keyPath.find('.');
    if (dotPos == std::string::npos)
        return { *this, keyPath };
    auto subKey = keyPath.substr(0, dotPos);
    if(auto it = mSubConfigs.find(subKey); it != mSubConfigs.end())
        return it->second->leafConfig(keyPath.substr(dotPos + 1), canCreateSubConfig);
    if (!canCreateSubConfig)
        throw std::runtime_error("Config key not found: " + keyPath);
    return mSubConfigs.emplace(subKey, make_intrusive_ptr<cConfig2>()).first->second->leafConfig(keyPath.substr(dotPos + 1), canCreateSubConfig);
}

std::pair<cConfig2&, std::string> cConfig2::leafConfig(const std::string& keyPath) const
{
    return const_cast<cConfig2*>(this)->leafConfig(keyPath, false);
}

void cConfig2::makeArray()
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