#include "shaders_tests_common.h"

#include "pixie/pixie/i_pixie.h"


namespace PsMetaDataTests
{

TEST(ps_meta_data, apply_texture_slots)
{
    auto state = std::make_shared<cLuaState>();
    state->executeString(R"(
texture_slots = { "", "", "apple", "banana" }
shaders = { 
    { name = "1", texture_slots = { "lemon", "", "", "pear" } },
    { name = "2", texture_slots = { "banana" } }
    }
    )");
    cPixelShader::cMetaData metaData;
    metaData.apply(state->globalTable());

    ASSERT_EQ(metaData.textureSlotIndex("apple"), 2);
    ASSERT_EQ(metaData.textureSlotIndex("banana"), 3);

    metaData.apply(state->globalTable().get("shaders").get(1));

    ASSERT_EQ(metaData.textureSlotIndex("lemon"), 0);
    ASSERT_EQ(metaData.textureSlotIndex("pear"), 3);
    ASSERT_EQ(metaData.textureSlotIndex("apple"), 2);
    ASSERT_EQ(metaData.textureSlotIndex("banana"), 3); // banana is still present from the previous apply

    metaData.apply(state->globalTable().get("shaders").get(2));

    ASSERT_EQ(metaData.textureSlotIndex("banana"), 0); // banana is now at index 0, because the last apply overrides the previous one
}

TEST(ps_meta_data, apply_parameters)
{
    auto state = std::make_shared<cLuaState>();
    state->executeString(R"(
parameters = { "param1", "param2", "param3" }
shaders = { 
    { name = "1", parameters = { "paramA", "paramB" } },
    { name = "2", parameters = { "paramX" } }
    }
    )");
    cPixelShader::cMetaData metaData;
    metaData.apply(state->globalTable());
    ASSERT_EQ(metaData.parameterIndex("param1"), 0);
    ASSERT_EQ(metaData.parameterIndex("param2"), 1);
    ASSERT_EQ(metaData.parameterIndex("param3"), 2);
    metaData.apply(state->globalTable().get("shaders").get(1));
    ASSERT_EQ(metaData.parameterIndex("paramA"), 0);
    ASSERT_EQ(metaData.parameterIndex("paramB"), 1);
    ASSERT_THROW(metaData.parameterIndex("param1"), std::runtime_error); // overriden
    ASSERT_THROW(metaData.parameterIndex("param2"), std::runtime_error); // overriden
    ASSERT_EQ(metaData.parameterIndex("param3"), 2);
    metaData.apply(state->globalTable().get("shaders").get(2));
    ASSERT_EQ(metaData.parameterIndex("paramX"), 0);
    ASSERT_THROW(metaData.parameterIndex("paramA"), std::runtime_error); // overriden
    ASSERT_EQ(metaData.parameterIndex("paramB"), 1);
    ASSERT_EQ(metaData.parameterIndex("param3"), 2);
}


} // namespace PsMetaDataTests



