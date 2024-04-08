#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"
#include "pixie/system/configuration/config.h"

const char* ProgramName = "Pixie Config Test";
const char* VersionString = "0.1";

namespace ConfigTests
{

void fillConfig(cConfig& config, int keyOffset = 0)
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

void verifyConfig(cConfig& config, const std::string& keyPrefix = std::string(), int keyOffset = 0)
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

TEST(config, set_int_get_int)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, config->get<int>(std::format("int_{0}", i)));
    }
}

TEST(config_array, set_int_get_int)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, config->get<int>(i * 4));
    }
}

TEST(config, set_int_get_string)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i).c_str() , config->get<std::string>(std::format("int_{0}", i)).c_str());
    }
}

TEST(config_array, set_int_get_string)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i).c_str(), config->get<std::string>(i * 4).c_str());
    }
}

TEST(config, set_int_get_double)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i, config->get<double>(std::format("int_{0}", i)));
    }
}

TEST(config_array, set_int_get_double)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i, config->get<double>(i * 4));
    }
}

TEST(config, set_int_get_bool)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i != 0, config->get<bool>(std::format("int_{0}", i)));
    }
}

TEST(config_array, set_int_get_bool)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i != 0, config->get<bool>(i * 4));
    }
}

TEST(config, set_double_get_int)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 200, config->get<int>(std::format("double_{0}", i)));
    }
}

TEST(config_array, set_double_get_int)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 200, config->get<int>(i * 4 + 2));
    }
}

TEST(config, set_double_get_string)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 200.0).c_str(), config->get<std::string>(std::format("double_{0}", i)).c_str());
    }
}

TEST(config_array, set_double_get_string)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 200.0).c_str(), config->get<std::string>(i * 4 + 2).c_str());
    }
}

TEST(config, set_double_get_double)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i + 200, config->get<double>(std::format("double_{0}", i)));
    }
}

TEST(config_array, set_double_get_double)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i + 200, config->get<double>(i * 4 + 2));
    }
}

TEST(config, set_double_get_bool)
{
    auto config = make_intrusive_ptr<cConfig>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(std::format("double_{0}", i), (double)i);
        EXPECT_EQ(i != 0, config->get<bool>(std::format("double_{0}", i)));
    }
}

TEST(config_array, set_double_get_bool)
{
    auto config = make_intrusive_ptr<cConfig>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(i * 4 + 2, (double)i);
        EXPECT_EQ(i != 0, config->get<bool>(i * 4 + 2));
    }
}

TEST(config, set_string_get_int)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100, config->get<int>(std::format("string_{0}", i)));
    }
}

TEST(config_array, set_string_get_int)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100, config->get<int>(i * 4 + 1));
    }
}

TEST(config, set_string_get_string)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 100).c_str(), config->get<std::string>(std::format("string_{0}", i)).c_str());
    }
}

TEST(config_array, set_string_get_string)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 100).c_str(), config->get<std::string>(i * 4 + 1).c_str());
    }
}

TEST(config, set_string_get_double)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100.0, config->get<double>(std::format("string_{0}", i)));
    }
}

TEST(config_array, set_string_get_double)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100.0, config->get<double>(i * 4 + 1));
    }
}

TEST(config, set_string_get_bool)
{
    auto config = make_intrusive_ptr<cConfig>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(std::format("string_{0}", i), i % 2 == 0 ? "true" : "false");
        EXPECT_EQ(i % 2 == 0, config->get<bool>(std::format("string_{0}", i)));
    }
}

TEST(config_array, set_string_get_bool)
{
    auto config = make_intrusive_ptr<cConfig>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(i * 4 + 1, i % 2 == 0 ? "true" : "false");
        EXPECT_EQ(i % 2 == 0, config->get<bool>(i * 4 + 1));
    }
}

TEST(config, set_bool_get_int)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<int>(std::format("bool_{0}", i)));
    }
}

TEST(config_array, set_bool_get_int)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<int>(i * 4 + 3));
    }
}

TEST(config, set_bool_get_string)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(i % 2 == 0 ? "true" : "false", config->get<std::string>(std::format("bool_{0}", i)).c_str());
    }
}

TEST(config_array, set_bool_get_string)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(i % 2 == 0 ? "true" : "false", config->get<std::string>(i * 4 + 3).c_str());
    }
}

TEST(config, set_bool_get_bool)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<bool>(std::format("bool_{0}", i)));
    }
}

TEST(config_array, set_bool_get_bool)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<bool>(i * 4 + 3));
    }
}

TEST(config, set_bool_get_double)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0 ? 1.0 : 0.0, config->get<double>(std::format("bool_{0}", i)));
    }
}

TEST(config_array, set_bool_get_double)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0 ? 1.0 : 0.0, config->get<double>(i * 4 + 3));
    }
}

TEST(config_array, push)
{
    for (int makeArray = 0; makeArray <= 1; ++makeArray)
    {
        auto config = make_intrusive_ptr<cConfig>();
        if(makeArray)
            config->makeArray();
        for (int i = 0; i < 10; ++i)
        {
            config->push(i);
            config->push(std::to_string(i + 100));
            config->push((double)i + 200);
            config->push(i % 2 == 0);
        }
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_EQ(i, config->get<int>(i * 4));
            EXPECT_STREQ(std::to_string(i + 100).c_str(), config->get<std::string>(i * 4 + 1).c_str());
            EXPECT_EQ((double)i + 200, config->get<double>(i * 4 + 2));
            EXPECT_EQ(i % 2 == 0, config->get<bool>(i * 4 + 3));
        }
    }
}

TEST(config, visit)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    int visited = 0;
    config->visit([&](const std::string& key, const auto& value) 
        {
            if constexpr(std::is_same_v<std::decay_t<decltype(value)>, int>)
            {
                EXPECT_EQ(std::stoi(key.substr(4)), value);
            }
            else if constexpr(std::is_same_v<std::decay_t<decltype(value)>, std::string>)
            {
                EXPECT_STREQ(std::to_string(std::stoi(key.substr(7)) + 100).c_str(), value.c_str());
            }
            else if constexpr(std::is_same_v<std::decay_t<decltype(value)>, double>)
            {
                EXPECT_EQ((double)std::stoi(key.substr(7)) + 200, value);
            }
            else if constexpr(std::is_same_v<std::decay_t<decltype(value)>, bool>)
            {
                EXPECT_EQ(std::stoi(key.substr(5)) % 2 == 0, value);
            }
            ++visited;
        });
    EXPECT_EQ(40, visited);
}

TEST(config_array, visit)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    int visited[4] = { 0, 0, 0, 0 };
    config->visit([&](const std::string& key, const auto& value)
        {
            if constexpr(std::is_same_v<std::decay_t<decltype(value)>, int>)
            {
                EXPECT_EQ(std::stoi(key) / 4, value);
                ++visited[0];
            }
            else if constexpr(std::is_same_v<std::decay_t<decltype(value)>, std::string>)
            {
                EXPECT_STREQ(std::to_string((std::stoi(key) - 1) / 4 + 100).c_str(), value.c_str());
                ++visited[1];
            }
            else if constexpr(std::is_same_v<std::decay_t<decltype(value)>, double>)
            {
                EXPECT_EQ((double)(std::stoi(key) - 2) / 4 + 200, value);
                ++visited[2];
            }
            else if constexpr(std::is_same_v<std::decay_t<decltype(value)>, bool>)
            {
                EXPECT_EQ((std::stoi(key) - 3) / 4 % 2 == 0, value);
                ++visited[3];
            }
        });
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(10, visited[i]) << "visited[" << i << "] is " << visited[i];
    }
}

TEST(config_array, visit_int_key)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    int visited = 0;
    config->visit([&](int idx, const auto& value)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int>)
            {
                EXPECT_EQ(idx / 4, value);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>)
            {
                EXPECT_STREQ(std::to_string((idx - 1) / 4 + 100).c_str(), value.c_str());
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double>)
            {
                EXPECT_EQ((double)(idx - 2) / 4 + 200, value);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, bool>)
            {
                EXPECT_EQ((idx - 3) / 4 % 2 == 0, value);
            }
            ++visited;
        });
    EXPECT_EQ(40, visited);
}

TEST(config, forEachString)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    int i = 0;
    config->forEachString([&i](const std::string& key, std::string value)
        {
            EXPECT_EQ(i + 100, std::stoi(value));
            EXPECT_STREQ(std::format("string_{0}", i).c_str(), key.c_str());
            ++i;
        });
    EXPECT_EQ(10, i);
}

TEST(config_array, forEachString)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    int i = 0;
    config->forEachString([&i](const std::string& key, const std::string& value)
        {
            EXPECT_EQ(i + 100, std::stoi(value));
            EXPECT_EQ(i * 4 + 1, std::stoi(key));
            ++i;
        });
    EXPECT_EQ(10, i);
}

TEST(config_array, forEachString_intKey)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    int i = 0;
    config->forEachString([&i](int key, const std::string& value)
        {
            EXPECT_EQ(i + 100, std::stoi(value));
            EXPECT_EQ(i * 4 + 1, key);
            ++i;
        });
}

TEST(subconfigs, set_get)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        auto subconfig = make_intrusive_ptr<cConfig>();
        fillConfig(*subconfig, 1000 * (i + 1));
        config->set(std::format("subconfig_{0}", i), subconfig);
    }
    verifyConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        auto subconfig = config->get<cConfig>(std::format("subconfig_{0}", i));
        verifyConfig(*subconfig, std::string(), 1000 * (i + 1));
    }
}

TEST(subconfigs, set_keypath_get_keypath)
{
    auto config = make_intrusive_ptr<cConfig>();
    config->set("subconfig_0.int_0", 0);
    config->set("subconfig_0.int_1", 1);
    config->set("subconfig_0.subconfig_a.int_100", 100);
    config->set("subconfig_0.subconfig_b.int_101", 101);
    config->set("subconfig_1.subconfig_c.string_102", "102"s);
    ASSERT_EQ(0, config->get<int>("subconfig_0.int_0"));
    ASSERT_EQ(1, config->get<int>("subconfig_0.int_1"));
    ASSERT_EQ(100, config->get<int>("subconfig_0.subconfig_a.int_100"));
    ASSERT_EQ(101, config->get<int>("subconfig_0.subconfig_b.int_101"));
    ASSERT_EQ("102"s, config->get<std::string>("subconfig_1.subconfig_c.string_102"));
}

TEST(subconfigs, set_normal_get_keypath)
{
    auto config = make_intrusive_ptr<cConfig>();
    auto subConfig0 = make_intrusive_ptr<cConfig>();
    subConfig0->set("int_0", 0);
    subConfig0->set("int_1", 1);
    config->set("subconfig_0", subConfig0);
    auto subConfig1 = make_intrusive_ptr<cConfig>();
    subConfig1->set("int_100", 100);
    subConfig1->set("int_101", 101);
    auto subconfigA = make_intrusive_ptr<cConfig>();
    subconfigA->set("int_200", 200);
    subConfig1->set("subconfig_a", subconfigA);
    config->set("subconfig_1", subConfig1);

    ASSERT_EQ(0, config->get<int>("subconfig_0.int_0"));
    ASSERT_EQ(1, config->get<int>("subconfig_0.int_1"));
    ASSERT_EQ(100, config->get<int>("subconfig_1.int_100"));
    ASSERT_EQ(101, config->get<int>("subconfig_1.int_101"));
    ASSERT_EQ(200, config->get<int>("subconfig_1.subconfig_a.int_200"));
}

TEST(subconfigs, set_keypath_get_normal)
{
    auto config = make_intrusive_ptr<cConfig>();
    config->set("subconfig_0.int_0", 0);
    config->set("subconfig_0.int_1", 1);
    config->set("subconfig_1.int_100", 100);
    config->set("subconfig_1.int_101", 101);
    config->set("subconfig_1.subconfig_a.int_200", 200);

    auto subConfig0 = config->get<cConfig>("subconfig_0");
    ASSERT_EQ(0, subConfig0->get<int>("int_0"));
    ASSERT_EQ(1, subConfig0->get<int>("int_1"));

    auto subConfig1 = config->get<cConfig>("subconfig_1");
    ASSERT_EQ(100, subConfig1->get<int>("int_100"));
    ASSERT_EQ(101, subConfig1->get<int>("int_101"));
    auto subconfigA = subConfig1->get<cConfig>("subconfig_a");
    ASSERT_EQ(200, subconfigA->get<int>("int_200"));
}

void fillSubconfigsArray(cConfig& config)
{
    for (int i = 0; i < 5; ++i)
    {
        auto subConfig = make_intrusive_ptr<cConfig>();
        int key = i * 4;
        subConfig->set("int_" + std::to_string(key), key);
        subConfig->set("string_" + std::to_string(key + 1), std::to_string(key + 100));
        subConfig->set("double_" + std::to_string(key + 2), (double)key + 200);
        subConfig->set("bool_" + std::to_string(key + 3), key % 2 == 0);
        config.set(i, subConfig);
    }
    ASSERT_TRUE(config.isArray());
}

TEST(subconfigs, in_array)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillSubconfigsArray(*config);
    for (int i = 0; i < 5; ++i)
    {
        auto subConfig = config->get<cConfig>(i);
        int key = i * 4;
        EXPECT_EQ(key, subConfig->get<int>("int_" + std::to_string(key)));
        EXPECT_STREQ(std::to_string(key + 100).c_str(), subConfig->get<std::string>("string_" + std::to_string(key + 1)).c_str());
        EXPECT_EQ((double)key + 200, subConfig->get<double>("double_" + std::to_string(key + 2)));
        EXPECT_EQ(key % 2 == 0, subConfig->get<bool>("bool_" + std::to_string(key + 3)));
    }
}

TEST(subconfigs, in_array_get_keypath)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillSubconfigsArray(*config);
    for(int i = 0; i < 5; ++i)
    {
        int key = i * 4;
        EXPECT_EQ(key, config->get<int>(std::to_string(i) + ".int_" + std::to_string(key)));
        EXPECT_STREQ(std::to_string(key + 100).c_str(), config->get<std::string>(std::to_string(i) + ".string_" + std::to_string(key + 1)).c_str());
        EXPECT_EQ((double)key + 200, config->get<double>(std::to_string(i) + ".double_" + std::to_string(key + 2)));
        EXPECT_EQ(key % 2 == 0, config->get<bool>(std::to_string(i) + ".bool_" + std::to_string(key + 3)));
    }
}

TEST(subconfigs, in_array_forEachSubConfig_const_ref)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillSubconfigsArray(*config);
    int i = 0;
    config->forEachSubConfig([&i](const std::string& key, const cConfig& subConfig)
        {
            int keyInt = std::stoi(key);
            int keyOffset = keyInt * 4;
            EXPECT_EQ(keyOffset, subConfig.get<int>("int_" + std::to_string(keyOffset)));
            EXPECT_STREQ(std::to_string(keyOffset + 100).c_str(), subConfig.get<std::string>("string_" + std::to_string(keyOffset + 1)).c_str());
            EXPECT_EQ((double)keyOffset + 200, subConfig.get<double>("double_" + std::to_string(keyOffset + 2)));
            EXPECT_EQ(keyOffset % 2 == 0, subConfig.get<bool>("bool_" + std::to_string(keyOffset + 3)));
            ++i;
        });
    EXPECT_EQ(5, i);
}

TEST(subconfigs, forEachSubConfig_intrusive_ptr)
{
    auto config = make_intrusive_ptr<cConfig>();
    fillSubconfigsArray(*config);
    int i = 0;
    config->forEachSubConfig([&i](const std::string& key, tIntrusivePtr<cConfig> subConfig)
        {
            int keyInt = std::stoi(key);
            int keyOffset = keyInt * 4;
            EXPECT_EQ(keyOffset, subConfig->get<int>("int_" + std::to_string(keyOffset)));
            EXPECT_STREQ(std::to_string(keyOffset + 100).c_str(), subConfig->get<std::string>("string_" + std::to_string(keyOffset + 1)).c_str());
            EXPECT_EQ((double)keyOffset + 200, subConfig->get<double>("double_" + std::to_string(keyOffset + 2)));
            EXPECT_EQ(keyOffset % 2 == 0, subConfig->get<bool>("bool_" + std::to_string(keyOffset + 3)));
            ++i;
        });
    EXPECT_EQ(5, i);
}


tIntrusivePtr<cConfig> createConfigForCountingTests()
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 5; ++i)
    {
        auto subConfig = make_intrusive_ptr<cConfig>();
        fillConfig(*subConfig, i * 1000);
        config->set(std::format("subconfig_{0}", i), std::move(subConfig));
    }
    return config;
}

tIntrusivePtr<cConfig> createConfigArrayForCountingTests()
{
    auto config = make_intrusive_ptr<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 5; ++i)
    {
        auto subConfig = make_intrusive_ptr<cConfig>();
        fillConfig(*subConfig, i * 1000);
        config->set(i + 40, std::move(subConfig));
    }
    return config;
}

TEST(config, numberOfValues)
{
    auto config = createConfigForCountingTests();
    EXPECT_EQ(40, config->numberOfValues());
}

TEST(config, numberOfSubConfigs)
{
    auto config = createConfigForCountingTests();
    EXPECT_EQ(5, config->numberOfSubConfigs());
}

TEST(config_array, numberOfValues)
{
    auto config = createConfigArrayForCountingTests();
    EXPECT_EQ(40, config->numberOfValues());
}

TEST(config_array, numberOfSubConfigs)
{
    auto config = createConfigArrayForCountingTests();
    EXPECT_EQ(5, config->numberOfSubConfigs());
}

TEST(config, empty)
{
    auto config = make_intrusive_ptr<cConfig>();
    EXPECT_TRUE(config->empty());
    fillConfig(*config);
    EXPECT_FALSE(config->empty());
}

TEST(config_array, empty)
{
    auto config = make_intrusive_ptr<cConfig>();
    config->makeArray();
    EXPECT_TRUE(config->empty());
    fillConfigArray(*config);
    EXPECT_FALSE(config->empty());
}

TEST(config_string_c_str, set_c_str_get_string)
{
    auto config = make_intrusive_ptr<cConfig>();
    config->set("key", "value");
    EXPECT_STREQ("value", config->get<std::string>("key").c_str());
}

} // namespace ConfigTests
//////////////////////////////////////////////////////////////////////////

// Main function to run the tests
int main(int argc, char** argv)
{
    cLuaScript::staticInit();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}