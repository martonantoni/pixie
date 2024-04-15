#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"
#include "pixie/system/configuration/config.h"

namespace ConfigTests
{

void fillConfig(cConfig& config, int keyOffset = 0);
void verifyConfig(cConfig& config, const std::string& keyPrefix = std::string(), int keyOffset = 0);
void fillConfigArray(cConfig& config);
void fillSubconfigsArray(cConfig& config);

}