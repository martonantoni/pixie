#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

namespace StringVectorTests
{

void testConstruction(const std::string &source, const std::string &delimeters, bool emptyFieldsAllowed, const std::vector<std::string> &expected)
{
    cStringVector tested(source, delimeters, emptyFieldsAllowed);
    EXPECT_EQ(tested.size(), expected.size()) << "source: \"" << source << "\", delimeters: \"" << delimeters << "\", emptyFieldsAllowed: " << emptyFieldsAllowed;
    for (size_t i = 0; i < expected.size(); i++)
        EXPECT_STREQ(tested[i].c_str(), expected[i].c_str()) << "source: \"" << source << "\", delimeters: \"" << delimeters << "\", emptyFieldsAllowed: " << emptyFieldsAllowed;

    tested.clear();
    tested.emplace_back("will");
    tested.emplace_back("be");
    tested.emplace_back("replaced");

    tested.FromString(source, delimeters, emptyFieldsAllowed);
    EXPECT_EQ(tested.size(), expected.size()) << "source: \"" << source << "\", delimeters: \"" << delimeters << "\", emptyFieldsAllowed: " << emptyFieldsAllowed;
    for (size_t i = 0; i < expected.size(); i++)
        EXPECT_STREQ(tested[i].c_str(), expected[i].c_str()) << "source: \"" << source << "\", delimeters: \"" << delimeters << "\", emptyFieldsAllowed: " << emptyFieldsAllowed;
}

void replace(std::string& str, const std::string& from, const std::string& to) 
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) 
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
}

void extendString(std::string& str)
{
    replace(str, "a", "alma");
    replace(str, "b", "banan");
    replace(str, "c", "citrom");
}

void testConstructionExtended(const std::string &source, const std::string &delimeters, bool emptyFieldsAllowed, const std::vector<std::string> &expected)
{
    testConstruction(source, delimeters, emptyFieldsAllowed, expected);
    std::string extendedSource = source;
    extendString(extendedSource);
    auto extendedExpected = expected;
    for (auto &s: extendedExpected)
    {
        extendString(s);
    }
    testConstruction(extendedSource, delimeters, emptyFieldsAllowed, extendedExpected);
}


TEST(construction, single_delimeter_allow_empty_fields)
{
    testConstructionExtended("", ",", true, { "" });
    testConstructionExtended("a", ",", true, {"a"});
    testConstructionExtended("a,b", ",", true, {"a", "b"});
    testConstructionExtended(",,,", ",", true, {"", "", "", ""});
    testConstructionExtended("a,,b", ",", true, {"a", "", "b"});
    testConstructionExtended("a,b,", ",", true, {"a", "b", ""});
    testConstructionExtended("a,b,c", ",", true, {"a", "b", "c"});
    testConstructionExtended(",a,b,c", ",", true, { "", "a", "b", "c"});
}

TEST(construction, single_delimeter_empty_fields_not_allowed)
{
    testConstructionExtended("", ",", false, {});
    testConstructionExtended("a", ",", false, {"a"});
    testConstructionExtended("a,b", ",", false, {"a", "b"});
    testConstructionExtended(",,,", ",", false, {});
    testConstructionExtended("a,,b", ",", false, {"a", "b"});
    testConstructionExtended("a,b,", ",", false, {"a", "b"});
    testConstructionExtended("a,b,c", ",", false, {"a", "b", "c"});
    testConstructionExtended(",a,b,c", ",", false, { "a", "b", "c"});
}

TEST(construction, multiple_delimeters_allow_empty_fields)
{
    testConstructionExtended("", ",;", true, { "" });
    testConstructionExtended("a", ",;", true, { "a" });
    testConstructionExtended("a,b", ",;", true, { "a", "b" });
    testConstructionExtended(",,,", ",;", true, { "", "", "", "" });
    testConstructionExtended("a,,b", ",;", true, { "a", "", "b" });
    testConstructionExtended("a,b,", ",;", true, { "a", "b", "" });
    testConstructionExtended("a,b,c", ",;", true, { "a", "b", "c" });
    testConstructionExtended(",a,b,c", ",;", true, { "", "a", "b", "c" });

    testConstructionExtended("a;b", ",;", true, { "a", "b" });
    testConstructionExtended(";,;", ",;", true, { "", "", "", "" });
    testConstructionExtended("a;,b", ",;", true, { "a", "", "b" });
    testConstructionExtended("a,b;", ",;", true, { "a", "b", "" });
    testConstructionExtended("a,b,c", ",;", true, { "a", "b", "c" });
    testConstructionExtended(";a,b;c", ",;", true, { "", "a", "b", "c" });
}

TEST(construction, multiple_delimeters_empty_fields_not_allowed)
{
    testConstructionExtended("", ",;", false, {});
    testConstructionExtended("a", ",;", false, { "a" });
    testConstructionExtended("a,b", ",;", false, { "a", "b" });
    testConstructionExtended(",,,", ",;", false, {});
    testConstructionExtended("a,,b", ",;", false, { "a", "b" });
    testConstructionExtended("a,b,", ",;", false, { "a", "b" });
    testConstructionExtended("a,b,c", ",;", false, { "a", "b", "c" });
    testConstructionExtended(",a,b,c", ",;", false, { "a", "b", "c" });

    testConstructionExtended("a;b", ",;", false, { "a", "b" });
    testConstructionExtended(";,;", ",;", false, {});
    testConstructionExtended("a;,b", ",;", false, { "a", "b" });
    testConstructionExtended("a,b;", ",;", false, { "a", "b" });
    testConstructionExtended("a,b,c", ",;", false, { "a", "b", "c" });
    testConstructionExtended(";a,b;c", ",;", false, { "a", "b", "c" });
}

TEST(construction, FromIntVector)
{
    cIntVector intVector = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cStringVector tested;
    tested.FromIntVector(intVector);
    EXPECT_EQ(tested.size(), 10);
    for (int i = 0; i < 10; i++)
        EXPECT_STREQ(tested[i].c_str(), std::to_string(i + 1).c_str());
}

TEST(utility, TrimAll)
{
    cStringVector tested("alma korte, banan ,     citrom   eper   ", ",", true);
    tested.TrimAll();
    EXPECT_EQ(tested.size(), 3);
    EXPECT_STREQ(tested[0].c_str(), "alma korte");
    EXPECT_STREQ(tested[1].c_str(), "banan");
    EXPECT_STREQ(tested[2].c_str(), "citrom   eper");
}

TEST(utility, ToIntVector)
{
    cStringVector tested("1, 2, 3, 4, 5, 6 ,7,8,  9   ,10,11 ", ",", true);
    cIntVector intVector = tested.ToIntVector();
    EXPECT_EQ(intVector.size(), 11);
    for (int i = 0; i < 11; i++)
        EXPECT_EQ(intVector[i], i + 1);
}

TEST(utility, FindIndex)
{
    //                    0      1       2       3       4      5
    cStringVector tested("alma, korte, banan, citrom, eper, alma", ",", true);
    tested.TrimAll();
    EXPECT_EQ(tested.FindIndex("alma"), 0);
    EXPECT_EQ(tested.FindIndex("korte"), 1);
    EXPECT_EQ(tested.FindIndex("banan"), 2);
    EXPECT_EQ(tested.FindIndex("citrom"), 3);
    EXPECT_EQ(tested.FindIndex("eper"), 4);
    EXPECT_EQ(tested.FindIndex("alma", 1), 5);
    EXPECT_EQ(tested.FindIndex("korte", 2), -1);
    EXPECT_EQ(tested.FindIndex("banan", 3), -1);
    EXPECT_EQ(tested.FindIndex("citrom", 4), -1);
    EXPECT_EQ(tested.FindIndex("eper", 5), -1);
}

TEST(utility, ToString)
{
    cStringVector tested("alma, korte, banan, citrom, eper, alma", ", ", false);
    EXPECT_STREQ(tested.ToString(";").c_str(), "alma;korte;banan;citrom;eper;alma");
    EXPECT_STREQ(tested.ToString(", ").c_str(), "alma, korte, banan, citrom, eper, alma");
}

} // namespace StringVectorTests
