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

TEST(ps_meta_data, apply_constants)
{
    auto state = std::make_shared<cLuaState>();
    state->executeString(R"(
constants = {
    const1 = {  type = "float", default = 1.0 },
    const2 = {  type = "color", default = { 0.5, 0.5, 0.5 } },
    const3 = {  type = "int", default = 42 }
    }
shaders = { 
    { name = "1", constants = {
        constA = { type = "float", default = 2.0 },
        constB = { type = "color", default = { 1.0, 0.0, 0.0 } },
        const1 = { type = "color", default = { 1.0, 0.0, 0.0 } }
        } },
    { name = "2", constants = {
        constX = { type = "int", default = 100 }
        } }
    }
    )");

    cPixelShader::cMetaData metaData;
    metaData.apply(state->globalTable());

    ASSERT_EQ(metaData.mConstants.size(), 3);

    ASSERT_TRUE(std::holds_alternative<float>(metaData.mConstants["const1"]));
    ASSERT_EQ(std::get<float>(metaData.mConstants["const1"]), 1.0f);
    using tColor = std::array<float, 3>;
    ASSERT_TRUE(std::holds_alternative<tColor>(metaData.mConstants["const2"]));
    ASSERT_EQ(std::get<tColor>(metaData.mConstants["const2"]), (tColor{0.5f, 0.5f, 0.5f}));
    ASSERT_TRUE(std::holds_alternative<int>(metaData.mConstants["const3"]));
    ASSERT_EQ(std::get<int>(metaData.mConstants["const3"]), 42);

    metaData.apply(state->globalTable().get("shaders").get(1));

    ASSERT_EQ(metaData.mConstants.size(), 5);
    ASSERT_TRUE(std::holds_alternative<float>(metaData.mConstants["constA"]));
    ASSERT_EQ(std::get<float>(metaData.mConstants["constA"]), 2.0f);
    ASSERT_TRUE(std::holds_alternative<tColor>(metaData.mConstants["constB"]));
    ASSERT_EQ(std::get<tColor>(metaData.mConstants["constB"]), (tColor{1.0f, 0.0f, 0.0f}));
    ASSERT_TRUE(std::holds_alternative<tColor>(metaData.mConstants["const1"]));
    ASSERT_EQ(std::get<tColor>(metaData.mConstants["const1"]), (tColor{1.0f, 0.0f, 0.0f})); // overridden

    metaData.apply(state->globalTable().get("shaders").get(2));

    ASSERT_EQ(metaData.mConstants.size(), 6);
    ASSERT_TRUE(std::holds_alternative<int>(metaData.mConstants["constX"]));
    ASSERT_EQ(std::get<int>(metaData.mConstants["constX"]), 100);
}

TEST(ps_meta_data, set_constants_from_preset)
{
    cPixelShader::cMetaData metaData;
    metaData.mConstants["const1"] = 1.0f;
    metaData.mConstants["const2"] = std::array<float, 3>{ 0.5f, 0.5f, 0.5f };
    metaData.mConstants["const3"] = 42;
    auto state = std::make_shared<cLuaState>();
    state->executeString(R"(
const1 = 2.0
const2 = { 1.0, 0.0, 0.0 }
const3 = 100
)");
    metaData.setConstantsFromPreset(state->globalTable());
    ASSERT_TRUE(std::holds_alternative<float>(metaData.mConstants["const1"]));
    ASSERT_EQ(std::get<float>(metaData.mConstants["const1"]), 2.0f);
    using tColor = std::array<float, 3>;
    ASSERT_TRUE(std::holds_alternative<tColor>(metaData.mConstants["const2"]));
    ASSERT_EQ(std::get<tColor>(metaData.mConstants["const2"]), (tColor{ 1.0f, 0.0f, 0.0f }));
    ASSERT_TRUE(std::holds_alternative<int>(metaData.mConstants["const3"]));
    ASSERT_EQ(std::get<int>(metaData.mConstants["const3"]), 100);
}

} // namespace PsMetaDataTests



