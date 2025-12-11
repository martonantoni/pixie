#include "StdAfx.h"

namespace Pixie
{


std::shared_ptr<cConfig> toConfig(const cLuaObject& object, IsRecursive isRecursive)
{
    auto config = std::make_shared<cConfig>();
    for (auto&& [key, value] : object)
    {
        value.visit([&](auto&& val)
        {
            using T = std::decay_t<decltype(val)>;
            if constexpr (
                std::is_same_v<T, int> ||
                std::is_same_v<T, std::string> ||
                std::is_same_v<T, double> ||
                std::is_same_v<T, bool>)
            {
                key.visit([&](auto&& k)
                    {
                        using KT = std::decay_t<decltype(k)>;
                        if constexpr (std::is_same_v<KT, std::string>)
                        {
                            config->set(k, val);
                        }
                        else if constexpr (std::is_same_v<KT, int>)
                        {
                            config->set(k - 1, val);
                        }
                        else
                        {
                            config->set(key.toString(), val);
                        }
                    });
            }
            else if constexpr (std::is_same_v<T, cLuaObject>)
            {
                if (isRecursive == IsRecursive::Yes)
                {
                    key.visit([&](auto&& k)
                        {
                            using KT = std::decay_t<decltype(k)>;
                            if constexpr (std::is_same_v<KT, std::string>)
                            {
                                config->set(k, toConfig(val, isRecursive));
                            }
                            else if constexpr (std::is_same_v<KT, int>)
                            {
                                config->set(k - 1, toConfig(val, isRecursive));
                            }
                            else
                            {
                                config->set(key.toString(), toConfig(val, isRecursive));
                            }
                        });
                }
            }
        });
    }
    return config;
}



} // namespace Pixie