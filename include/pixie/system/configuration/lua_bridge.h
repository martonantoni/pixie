#pragma once

#include "lua/i_lua.h"

namespace Pixie
{

enum class IsRecursive { Yes, No };

std::shared_ptr<cConfig> toConfig(const cLuaObject &object, IsRecursive isRecursive = IsRecursive::Yes);

} // namespace Pixie