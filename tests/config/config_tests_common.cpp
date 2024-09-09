#include "config_tests_common.h"

namespace ConfigTests
{

void fillConfig(cConfig& config, int keyOffset)
{
    for (int i = 0; i < 10; ++i)
    {
        int key = i + keyOffset;
        config.set(std::format("int_{0}", key), key);
        config.set(std::format("string_{0}", key), std::to_string(key+100));
        config.set(std::format("double_{0}", key), (double)key+200);
        config.set(std::format("bool_{0}", key), key % 2 == 0);
    }
    ASSERT_FALSE(config.isArray());
}

void verifyConfig(cConfig& config, const std::string& keyPrefix, int keyOffset)
{
    for (int i = 0; i < 10; ++i)
    {
        int key = i + keyOffset;
        EXPECT_EQ(key, config.get<int>(keyPrefix + std::format("int_{0}", key)));
        EXPECT_STREQ(std::to_string(key + 100).c_str(), config.get<std::string>(keyPrefix + std::format("string_{0}", key)).c_str());
        EXPECT_EQ((double)key + 200, config.get<double>(keyPrefix + std::format("double_{0}", key)));
        EXPECT_EQ(key % 2 == 0, config.get<bool>(keyPrefix + std::format("bool_{0}", key)));
    }
}

void fillConfigArray(cConfig& config)
{
    for (int i = 0; i < 10; ++i)
    {
        config.set(i * 4, i);
        config.set(i * 4 + 1, std::to_string(i + 100));
        config.set(i * 4 + 2, (double)i + 200);
        config.set(i * 4 + 3, i % 2 == 0);
    }
    ASSERT_TRUE(config.isArray());
}

void fillSubconfigsArray(cConfig& config)
{
    for (int i = 0; i < 5; ++i)
    {
        auto subConfig = std::make_shared<cConfig>();
        int key = i * 4;
        subConfig->set("int_" + std::to_string(key), key);
        subConfig->set("string_" + std::to_string(key + 1), std::to_string(key + 100));
        subConfig->set("double_" + std::to_string(key + 2), (double)key + 200);
        subConfig->set("bool_" + std::to_string(key + 3), key % 2 == 0);
        config.set(i, subConfig);
    }
    ASSERT_TRUE(config.isArray());
}


} // namespace ConfigTests
//////////////////////////////////////////////////////////////////////////

const char* ProgramName = "Pixie Config Test";
const char* VersionString = "1.0";

// Main function to run the tests
int main(int argc, char** argv)
{
    cLuaState::staticInit();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}