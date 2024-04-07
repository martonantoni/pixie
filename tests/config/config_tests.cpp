#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"
#include "pixie/system/configuration/config2.h"

const char* ProgramName = "Pixie Config Test";
const char* VersionString = "0.1";

namespace ConfigTests
{

void fillConfig(cConfig2& config)
{
    for (int i = 0; i < 10; ++i)
    {
        config.set(std::format("int_{0}", i), i);
        config.set(std::format("string_{0}", i), std::to_string(i+100));
        config.set(std::format("double_{0}", i), (double)i+200);
        config.set(std::format("bool_{0}", i), i % 2 == 0);
    }
    ASSERT_FALSE(config.isArray());
}

void fillConfigArray(cConfig2& config)
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
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, config->get<int>(std::format("int_{0}", i)));
    }
}

TEST(config_array, set_int_get_int)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, config->get<int>(i * 4));
    }
}

TEST(config, set_int_get_string)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i).c_str() , config->get<std::string>(std::format("int_{0}", i)).c_str());
    }
}

TEST(config_array, set_int_get_string)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i).c_str(), config->get<std::string>(i * 4).c_str());
    }
}

TEST(config, set_int_get_double)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i, config->get<double>(std::format("int_{0}", i)));
    }
}

TEST(config_array, set_int_get_double)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i, config->get<double>(i * 4));
    }
}

TEST(config, set_int_get_bool)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i != 0, config->get<bool>(std::format("int_{0}", i)));
    }
}

TEST(config_array, set_int_get_bool)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i != 0, config->get<bool>(i * 4));
    }
}

TEST(config, set_double_get_int)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 200, config->get<int>(std::format("double_{0}", i)));
    }
}

TEST(config_array, set_double_get_int)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 200, config->get<int>(i * 4 + 2));
    }
}

TEST(config, set_double_get_string)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 200.0).c_str(), config->get<std::string>(std::format("double_{0}", i)).c_str());
    }
}

TEST(config_array, set_double_get_string)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 200.0).c_str(), config->get<std::string>(i * 4 + 2).c_str());
    }
}

TEST(config, set_double_get_double)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i + 200, config->get<double>(std::format("double_{0}", i)));
    }
}

TEST(config_array, set_double_get_double)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ((double)i + 200, config->get<double>(i * 4 + 2));
    }
}

TEST(config, set_double_get_bool)
{
    auto config = make_intrusive_ptr<cConfig2>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(std::format("double_{0}", i), (double)i);
        EXPECT_EQ(i != 0, config->get<bool>(std::format("double_{0}", i)));
    }
}

TEST(config_array, set_double_get_bool)
{
    auto config = make_intrusive_ptr<cConfig2>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(i * 4 + 2, (double)i);
        EXPECT_EQ(i != 0, config->get<bool>(i * 4 + 2));
    }
}

TEST(config, set_string_get_int)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100, config->get<int>(std::format("string_{0}", i)));
    }
}

TEST(config_array, set_string_get_int)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100, config->get<int>(i * 4 + 1));
    }
}

TEST(config, set_string_get_string)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 100).c_str(), config->get<std::string>(std::format("string_{0}", i)).c_str());
    }
}

TEST(config_array, set_string_get_string)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(std::to_string(i + 100).c_str(), config->get<std::string>(i * 4 + 1).c_str());
    }
}

TEST(config, set_string_get_double)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100.0, config->get<double>(std::format("string_{0}", i)));
    }
}

TEST(config_array, set_string_get_double)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i + 100.0, config->get<double>(i * 4 + 1));
    }
}

TEST(config, set_string_get_bool)
{
    auto config = make_intrusive_ptr<cConfig2>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(std::format("string_{0}", i), i % 2 == 0 ? "true" : "false");
        EXPECT_EQ(i % 2 == 0, config->get<bool>(std::format("string_{0}", i)));
    }
}

TEST(config_array, set_string_get_bool)
{
    auto config = make_intrusive_ptr<cConfig2>();
    for (int i = 0; i < 10; ++i)
    {
        config->set(i * 4 + 1, i % 2 == 0 ? "true" : "false");
        EXPECT_EQ(i % 2 == 0, config->get<bool>(i * 4 + 1));
    }
}

TEST(config, set_bool_get_int)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<int>(std::format("bool_{0}", i)));
    }
}

TEST(config_array, set_bool_get_int)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<int>(i * 4 + 3));
    }
}

TEST(config, set_bool_get_string)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(i % 2 == 0 ? "true" : "false", config->get<std::string>(std::format("bool_{0}", i)).c_str());
    }
}

TEST(config_array, set_bool_get_string)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_STREQ(i % 2 == 0 ? "true" : "false", config->get<std::string>(i * 4 + 3).c_str());
    }
}

TEST(config, set_bool_get_bool)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<bool>(std::format("bool_{0}", i)));
    }
}

TEST(config_array, set_bool_get_bool)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0, config->get<bool>(i * 4 + 3));
    }
}

TEST(config, set_bool_get_double)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfig(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0 ? 1.0 : 0.0, config->get<double>(std::format("bool_{0}", i)));
    }
}

TEST(config_array, set_bool_get_double)
{
    auto config = make_intrusive_ptr<cConfig2>();
    fillConfigArray(*config);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i % 2 == 0 ? 1.0 : 0.0, config->get<double>(i * 4 + 3));
    }
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