#include "config_tests_common.h"

namespace ConfigCompareTests
{

TEST(compare, single_level_same_type)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillConfig(*config1);
    auto config2 = std::make_shared<cConfig>();
    ConfigTests::fillConfig(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config2->set("int_0", 100);
    EXPECT_FALSE(*config1 == *config2);
    config1->set("int_0", 100);
    EXPECT_TRUE(*config1 == *config2);
}

TEST(compare_array, single_level_same_type)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillConfigArray(*config1);
    auto config2 = std::make_shared<cConfig>();
    ConfigTests::fillConfigArray(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config2->set(0, 100);
    EXPECT_FALSE(*config1 == *config2);
    config1->set(0, 100);
    EXPECT_TRUE(*config1 == *config2);
}

TEST(compare, single_level_different_type)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillConfig(*config1);
    auto config2 = std::make_shared<cConfig>();
    ConfigTests::fillConfig(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config1->set("tested_val", 100.0);
    config2->set("tested_val", "alma"s);
    EXPECT_FALSE(*config1 == *config2);
    config2->set("tested_val", 100.0);
    EXPECT_TRUE(*config1 == *config2);
}

TEST(compare_array, single_level_different_type)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillConfigArray(*config1);
    auto config2 = std::make_shared<cConfig>();
    ConfigTests::fillConfigArray(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config1->set(0, 100.0);
    config2->set(0, "alma"s);
    EXPECT_FALSE(*config1 == *config2);
    config2->set(0, 100.0);
    EXPECT_TRUE(*config1 == *config2);
}

TEST(compare, missing_key)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillConfig(*config1);
    auto config2 = std::make_shared<cConfig>();
    ConfigTests::fillConfig(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config1->set("missing_key", 100);
    EXPECT_FALSE(*config1 == *config2);
    config2->set("other_key", 100);
    EXPECT_FALSE(*config1 == *config2);
    config2->set("missing_key", 100);
    EXPECT_FALSE(*config1 == *config2);
    config1->set("other_key", 100);
    EXPECT_TRUE(*config1 == *config2);
}

TEST(compare_array, missing_element)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillConfigArray(*config1);
    auto config2 = std::make_shared<cConfig>();
    ConfigTests::fillConfigArray(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config1->push(100);
    EXPECT_FALSE(*config1 == *config2);
    config2->push(100);
    EXPECT_TRUE(*config1 == *config2);
}

void fillSubconfigTree(cConfig& config)
{
    config.set("int_0", 0);
    config.set("int_1", 5);
    config.set("sub_0.int_0", 1);
    config.set("sub_1.int_0", 2);
    config.set("sub_0.sub_0.int_0", 3);
    config.set("sub_0.sub_0.int_1", 4);
}

TEST(compare, subconfigs)
{
    auto config1 = std::make_shared<cConfig>();
    fillSubconfigTree(*config1);
    auto config2 = std::make_shared<cConfig>();
    fillSubconfigTree(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config1->set("sub_0.sub_0.int_1", 100);
    EXPECT_FALSE(*config1 == *config2);
    config2->set("sub_0.sub_0.int_1", 100);
    EXPECT_TRUE(*config1 == *config2);
}

TEST(compare, subconfig_in_on_value_on_other)
{
    auto config1 = std::make_shared<cConfig>();
    fillSubconfigTree(*config1);
    auto config2 = std::make_shared<cConfig>();
    fillSubconfigTree(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config1->set("sub_0", 100);
    EXPECT_FALSE(*config1 == *config2);
    config2->set("sub_0", 100);
    EXPECT_TRUE(*config1 == *config2);
}

TEST(compare_array, subconfig_in_on_value_on_other)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillSubconfigsArray(*config1);
    auto config2 = std::make_shared<cConfig>();
    ConfigTests::fillSubconfigsArray(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config1->set(0, 100);
    EXPECT_FALSE(*config1 == *config2);
    config2->set(0, 100);
    EXPECT_TRUE(*config1 == *config2);
}

TEST(compare_array, subconfigs)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillSubconfigsArray(*config1);
    auto config2 = std::make_shared<cConfig>();
    ConfigTests::fillSubconfigsArray(*config2);
    EXPECT_TRUE(*config1 == *config2);
    config1->get<cConfig>(0)->set("int_0", 9999);
    EXPECT_FALSE(*config1 == *config2);
    config2->get<cConfig>(0)->set("int_0", 9999);
    EXPECT_TRUE(*config1 == *config2);
}

TEST(compare, against_empty)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillConfig(*config1);
    auto config2 = std::make_shared<cConfig>();
    EXPECT_FALSE(*config1 == *config2);
    EXPECT_FALSE(*config2 == *config1);
}

TEST(compare_array, against_empty)
{
    auto config1 = std::make_shared<cConfig>();
    ConfigTests::fillConfigArray(*config1);
    auto config2 = std::make_shared<cConfig>();
    EXPECT_FALSE(*config1 == *config2);
    EXPECT_FALSE(*config2 == *config1);
}

} // namespace ConfigCompareTests
