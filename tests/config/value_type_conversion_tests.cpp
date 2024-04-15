#include "config_tests_common.h"

namespace ConfigTests
{


TEST(config, set_int_get_int)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, config->get<int>(std::format("int_{0}", i)));
    }
}

TEST(config_array, set_int_get_int)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, config->get<int>(i * 4));
    }
}

TEST(config, set_int_get_string)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i).c_str() , config->get<std::string>(std::format("int_{0}", i)).c_str());
    }
}

TEST(config_array, set_int_get_string)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i).c_str(), config->get<std::string>(i * 4).c_str());
    }
}

TEST(config, set_int_get_double)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i, config->get<double>(std::format("int_{0}", i)));
    }
}

TEST(config_array, set_int_get_double)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i, config->get<double>(i * 4));
    }
}

TEST(config, set_int_get_bool)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i != 0, config->get<bool>(std::format("int_{0}", i)));
    }
}

TEST(config_array, set_int_get_bool)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i != 0, config->get<bool>(i * 4));
    }
}

TEST(config, set_double_get_int)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 200, config->get<int>(std::format("double_{0}", i)));
    }
}

TEST(config_array, set_double_get_int)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 200, config->get<int>(i * 4 + 2));
    }
}

TEST(config, set_double_get_string)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 200.0).c_str(), config->get<std::string>(std::format("double_{0}", i)).c_str());
    }
}

TEST(config_array, set_double_get_string)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 200.0).c_str(), config->get<std::string>(i * 4 + 2).c_str());
    }
}

TEST(config, set_double_get_double)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i + 200, config->get<double>(std::format("double_{0}", i)));
    }
}

TEST(config_array, set_double_get_double)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i + 200, config->get<double>(i * 4 + 2));
    }
}

TEST(config, set_double_get_bool)
{
    auto config = std::make_shared<cConfig>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(std::format("double_{0}", i), (double)i);
        EXPECT_EQ(i != 0, config->get<bool>(std::format("double_{0}", i)));
    }
}

TEST(config_array, set_double_get_bool)
{
    auto config = std::make_shared<cConfig>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(i * 4 + 2, (double)i);
        EXPECT_EQ(i != 0, config->get<bool>(i * 4 + 2));
    }
}

TEST(config, set_string_get_int)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100, config->get<int>(std::format("string_{0}", i)));
    }
}

TEST(config_array, set_string_get_int)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100, config->get<int>(i * 4 + 1));
    }
}

TEST(config, set_string_get_string)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 100).c_str(), config->get<std::string>(std::format("string_{0}", i)).c_str());
    }
}

TEST(config_array, set_string_get_string)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 100).c_str(), config->get<std::string>(i * 4 + 1).c_str());
    }
}

TEST(config, set_string_get_double)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100.0, config->get<double>(std::format("string_{0}", i)));
    }
}

TEST(config_array, set_string_get_double)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100.0, config->get<double>(i * 4 + 1));
    }
}

TEST(config, set_string_get_bool)
{
    auto config = std::make_shared<cConfig>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(std::format("string_{0}", i), i % 2 == 0 ? "true" : "false");
        EXPECT_EQ(i % 2 == 0, config->get<bool>(std::format("string_{0}", i)));
    }
}

TEST(config_array, set_string_get_bool)
{
    auto config = std::make_shared<cConfig>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(i * 4 + 1, i % 2 == 0 ? "true" : "false");
        EXPECT_EQ(i % 2 == 0, config->get<bool>(i * 4 + 1));
    }
}

TEST(config, set_bool_get_int)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<int>(std::format("bool_{0}", i)));
    }
}

TEST(config_array, set_bool_get_int)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<int>(i * 4 + 3));
    }
}

TEST(config, set_bool_get_string)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(i % 2 == 0 ? "true" : "false", config->get<std::string>(std::format("bool_{0}", i)).c_str());
    }
}

TEST(config_array, set_bool_get_string)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(i % 2 == 0 ? "true" : "false", config->get<std::string>(i * 4 + 3).c_str());
    }
}

TEST(config, set_bool_get_bool)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<bool>(std::format("bool_{0}", i)));
    }
}

TEST(config_array, set_bool_get_bool)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<bool>(i * 4 + 3));
    }
}

TEST(config, set_bool_get_double)
{
    auto config = std::make_shared<cConfig>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0 ? 1.0 : 0.0, config->get<double>(std::format("bool_{0}", i)));
    }
}

TEST(config_array, set_bool_get_double)
{
    auto config = std::make_shared<cConfig>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0 ? 1.0 : 0.0, config->get<double>(i * 4 + 3));
    }
}

} // namespace ConfigTests
