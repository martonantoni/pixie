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


cLuaFunctionEntries commonConfigHelperFunctions()
{
    static cLuaFunctionEntries entries(
        std::tuple
        {
            tLuaFunctionEntry("RGB",
                [](int red, int green, int blue) -> int
                {
                    ASSERT(red >= 0 && red <= 255);
                    ASSERT(green >= 0 && green <= 255);
                    ASSERT(blue >= 0 && blue <= 255);

                    uint32_t color =
                        (0xffu << 24) |
                        (static_cast<uint32_t>(red) << 16) |
                        (static_cast<uint32_t>(green) << 8) |
                        static_cast<uint32_t>(blue);

                    return static_cast<int>(color);
                }),
            tLuaFunctionEntry("ARGB",
                [](int alpha, int red, int green, int blue) -> int
                {
                    ASSERT(alpha >= 0 && alpha <= 255);
                    ASSERT(red >= 0 && red <= 255);
                    ASSERT(green >= 0 && green <= 255);
                    ASSERT(blue >= 0 && blue <= 255);

                    uint32_t color =
                        (static_cast<uint32_t>(alpha) << 24) |
                        (static_cast<uint32_t>(red) << 16) |
                        (static_cast<uint32_t>(green) << 8) |
                        static_cast<uint32_t>(blue);

                    return static_cast<int>(color);
                }),
            tLuaFunctionEntry("XEnd",
                [](cLuaObject object) -> int
                {
                    return object.get<int>("x") + object.get<int>("w");
                }),
            tLuaFunctionEntry("YEnd",
                [](cLuaObject object) -> int
                {
                    return object.get<int>("y") + object.get<int>("h");
                }),
            tLuaFunctionEntry("Rect",
                [](cLuaState script, int x, int y, int w, int h) -> cLuaObject
                {
                    cLuaObject rectTable = script.createTable();
                    rectTable.set("x", x);
                    rectTable.set("y", y);
                    rectTable.set("w", w);
                    rectTable.set("h", h);
                    return rectTable;
                })
        }
    );
    return entries;
}



} // namespace Pixie