#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

namespace StringVectorTests
{

template<class SourceType>
void testConstruction(SourceType source, const std::string &delimeters, bool emptyFieldsAllowed, const std::vector<std::string> &expected)
{
    cStringVector tested(source, delimeters, emptyFieldsAllowed);
    EXPECT_EQ(tested.size(), expected.size()) << "source: \"" << source << "\", delimeters: \"" << delimeters << "\", emptyFieldsAllowed: " << emptyFieldsAllowed;
    for (size_t i = 0; i < expected.size(); i++)
        EXPECT_STREQ(tested[i].c_str(), expected[i].c_str()) << "source: \"" << source << "\", delimeters: \"" << delimeters << "\", emptyFieldsAllowed: " << emptyFieldsAllowed;

    tested.clear();
    tested.emplace_back("will");
    tested.emplace_back("be");
    tested.emplace_back("replaced");

    tested.fromString(source, delimeters, emptyFieldsAllowed);
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

void testConstructionExtendedSV(const std::string &source, const std::string &delimeters, bool emptyFieldsAllowed, const std::vector<std::string> &expected)
{
    testConstruction(std::string_view(source), delimeters, emptyFieldsAllowed, expected);
    std::string extendedSource = source;
    extendString(extendedSource);
    auto extendedExpected = expected;
    for (auto &s: extendedExpected)
    {
        extendString(s);
    }
    testConstruction(std::string_view(extendedSource), delimeters, emptyFieldsAllowed, extendedExpected);
}

void testConstructionExtendedCStar(const std::string& source, const std::string& delimeters, bool emptyFieldsAllowed, const std::vector<std::string>& expected)
{
    testConstruction(source.c_str(), delimeters, emptyFieldsAllowed, expected);
    std::string extendedSource = source;
    extendString(extendedSource);
    auto extendedExpected = expected;
    for (auto& s : extendedExpected)
    {
        extendString(s);
    }
    testConstruction(extendedSource.c_str(), delimeters, emptyFieldsAllowed, extendedExpected);
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

TEST(construction_from_string_view, single_delimeter_allow_empty_fields)
{
    testConstructionExtendedSV("", ",", true, { "" });
    testConstructionExtendedSV("a", ",", true, { "a" });
    testConstructionExtendedSV("a,b", ",", true, { "a", "b" });
    testConstructionExtendedSV(",,,", ",", true, { "", "", "", "" });
    testConstructionExtendedSV("a,,b", ",", true, { "a", "", "b" });
    testConstructionExtendedSV("a,b,", ",", true, { "a", "b", "" });
    testConstructionExtendedSV("a,b,c", ",", true, { "a", "b", "c" });
    testConstructionExtendedSV(",a,b,c", ",", true, { "", "a", "b", "c" });
}

TEST(construction_from_string_view, single_delimeter_empty_fields_not_allowed)
{
    testConstructionExtendedSV("", ",", false, {});
    testConstructionExtendedSV("a", ",", false, { "a" });
    testConstructionExtendedSV("a,b", ",", false, { "a", "b" });
    testConstructionExtendedSV(",,,", ",", false, {});
    testConstructionExtendedSV("a,,b", ",", false, { "a", "b" });
    testConstructionExtendedSV("a,b,", ",", false, { "a", "b" });
    testConstructionExtendedSV("a,b,c", ",", false, { "a", "b", "c" });
    testConstructionExtendedSV(",a,b,c", ",", false, { "a", "b", "c" });
}

TEST(construction_from_string_view, multiple_delimeters_allow_empty_fields)
{
    testConstructionExtendedSV("", ",;", true, { "" });
    testConstructionExtendedSV("a", ",;", true, { "a" });
    testConstructionExtendedSV("a,b", ",;", true, { "a", "b" });
    testConstructionExtendedSV(",,,", ",;", true, { "", "", "", "" });
    testConstructionExtendedSV("a,,b", ",;", true, { "a", "", "b" });
    testConstructionExtendedSV("a,b,", ",;", true, { "a", "b", "" });
    testConstructionExtendedSV("a,b,c", ",;", true, { "a", "b", "c" });
    testConstructionExtendedSV(",a,b,c", ",;", true, { "", "a", "b", "c" });

    testConstructionExtendedSV("a;b", ",;", true, { "a", "b" });
    testConstructionExtendedSV(";,;", ",;", true, { "", "", "", "" });
    testConstructionExtendedSV("a;,b", ",;", true, { "a", "", "b" });
    testConstructionExtendedSV("a,b;", ",;", true, { "a", "b", "" });
    testConstructionExtendedSV("a,b,c", ",;", true, { "a", "b", "c" });
    testConstructionExtendedSV(";a,b;c", ",;", true, { "", "a", "b", "c" });
}

TEST(construction_from_string_view, multiple_delimeters_empty_fields_not_allowed)
{
    testConstructionExtendedSV("", ",;", false, {});
    testConstructionExtendedSV("a", ",;", false, { "a" });
    testConstructionExtendedSV("a,b", ",;", false, { "a", "b" });
    testConstructionExtendedSV(",,,", ",;", false, {});
    testConstructionExtendedSV("a,,b", ",;", false, { "a", "b" });
    testConstructionExtendedSV("a,b,", ",;", false, { "a", "b" });
    testConstructionExtendedSV("a,b,c", ",;", false, { "a", "b", "c" });
    testConstructionExtendedSV(",a,b,c", ",;", false, { "a", "b", "c" });

    testConstructionExtendedSV("a;b", ",;", false, { "a", "b" });
    testConstructionExtendedSV(";,;", ",;", false, {});
    testConstructionExtendedSV("a;,b", ",;", false, { "a", "b" });
    testConstructionExtendedSV("a,b;", ",;", false, { "a", "b" });
    testConstructionExtendedSV("a,b,c", ",;", false, { "a", "b", "c" });
    testConstructionExtendedSV(";a,b;c", ",;", false, { "a", "b", "c" });
}

TEST(construction_from_cs, single_delimeter_allow_empty_fields)
{
    testConstructionExtendedCStar("", ",", true, { "" });
    testConstructionExtendedCStar("a", ",", true, { "a" });
    testConstructionExtendedCStar("a,b", ",", true, { "a", "b" });
    testConstructionExtendedCStar(",,,", ",", true, { "", "", "", "" });
    testConstructionExtendedCStar("a,,b", ",", true, { "a", "", "b" });
    testConstructionExtendedCStar("a,b,", ",", true, { "a", "b", "" });
    testConstructionExtendedCStar("a,b,c", ",", true, { "a", "b", "c" });
    testConstructionExtendedCStar(",a,b,c", ",", true, { "", "a", "b", "c" });
}

TEST(construction_from_cs, single_delimeter_empty_fields_not_allowed)
{
    testConstructionExtendedCStar("", ",", false, {});
    testConstructionExtendedCStar("a", ",", false, { "a" });
    testConstructionExtendedCStar("a,b", ",", false, { "a", "b" });
    testConstructionExtendedCStar(",,,", ",", false, {});
    testConstructionExtendedCStar("a,,b", ",", false, { "a", "b" });
    testConstructionExtendedCStar("a,b,", ",", false, { "a", "b" });
    testConstructionExtendedCStar("a,b,c", ",", false, { "a", "b", "c" });
    testConstructionExtendedCStar(",a,b,c", ",", false, { "a", "b", "c" });
}

TEST(construction_from_cs, multiple_delimeters_allow_empty_fields)
{
    testConstructionExtendedCStar("", ",;", true, { "" });
    testConstructionExtendedCStar("a", ",;", true, { "a" });
    testConstructionExtendedCStar("a,b", ",;", true, { "a", "b" });
    testConstructionExtendedCStar(",,,", ",;", true, { "", "", "", "" });
    testConstructionExtendedCStar("a,,b", ",;", true, { "a", "", "b" });
    testConstructionExtendedCStar("a,b,", ",;", true, { "a", "b", "" });
    testConstructionExtendedCStar("a,b,c", ",;", true, { "a", "b", "c" });
    testConstructionExtendedCStar(",a,b,c", ",;", true, { "", "a", "b", "c" });

    testConstructionExtendedCStar("a;b", ",;", true, { "a", "b" });
    testConstructionExtendedCStar(";,;", ",;", true, { "", "", "", "" });
    testConstructionExtendedCStar("a;,b", ",;", true, { "a", "", "b" });
    testConstructionExtendedCStar("a,b;", ",;", true, { "a", "b", "" });
    testConstructionExtendedCStar("a,b,c", ",;", true, { "a", "b", "c" });
    testConstructionExtendedCStar(";a,b;c", ",;", true, { "", "a", "b", "c" });
}

TEST(construction_from_cs, multiple_delimeters_empty_fields_not_allowed)
{
    testConstructionExtendedCStar("", ",;", false, {});
    testConstructionExtendedCStar("a", ",;", false, { "a" });
    testConstructionExtendedCStar("a,b", ",;", false, { "a", "b" });
    testConstructionExtendedCStar(",,,", ",;", false, {});
    testConstructionExtendedCStar("a,,b", ",;", false, { "a", "b" });
    testConstructionExtendedCStar("a,b,", ",;", false, { "a", "b" });
    testConstructionExtendedCStar("a,b,c", ",;", false, { "a", "b", "c" });
    testConstructionExtendedCStar(",a,b,c", ",;", false, { "a", "b", "c" });

    testConstructionExtendedCStar("a;b", ",;", false, { "a", "b" });
    testConstructionExtendedCStar(";,;", ",;", false, {});
    testConstructionExtendedCStar("a;,b", ",;", false, { "a", "b" });
    testConstructionExtendedCStar("a,b;", ",;", false, { "a", "b" });
    testConstructionExtendedCStar("a,b,c", ",;", false, { "a", "b", "c" });
    testConstructionExtendedCStar(";a,b;c", ",;", false, { "a", "b", "c" });
}

TEST(construction_from_char_array, mixed)
{
    static char source[] = "a,b,c";
    cStringVector tested(source, ",", true);
    EXPECT_EQ(tested.size(), 3);
    EXPECT_STREQ(tested[0].c_str(), "a");
    EXPECT_STREQ(tested[1].c_str(), "b");
    EXPECT_STREQ(tested[2].c_str(), "c");
}

TEST(storing_string_views, construction_from_cstar)
{
    {
        cStringViewVector tested("alma, korte, banan, citrom, eper, alma", ", ", false);
        EXPECT_EQ(tested.size(), 6);
        EXPECT_STREQ(std::string(tested[0]).c_str(), "alma");
        EXPECT_STREQ(std::string(tested[1]).c_str(), "korte");
        EXPECT_STREQ(std::string(tested[2]).c_str(), "banan");
        EXPECT_STREQ(std::string(tested[3]).c_str(), "citrom");
        EXPECT_STREQ(std::string(tested[4]).c_str(), "eper");
        EXPECT_STREQ(std::string(tested[5]).c_str(), "alma");
    }
    {
        cStringViewVector tested(",alma,,korte,", ", ", true);
        EXPECT_EQ(tested.size(), 5);
        EXPECT_STREQ(std::string(tested[0]).c_str(), "");
        EXPECT_STREQ(std::string(tested[1]).c_str(), "alma");
        EXPECT_STREQ(std::string(tested[2]).c_str(), "");
        EXPECT_STREQ(std::string(tested[3]).c_str(), "korte");
        EXPECT_STREQ(std::string(tested[4]).c_str(), "");
    }
}

TEST(stroring_string_view, construction_from_string)
{
    std::string source(",alma,,korte,");
    cStringViewVector tested(source, ", ", true);
    EXPECT_EQ(tested.size(), 5);
    EXPECT_STREQ(std::string(tested[0]).c_str(), "");
    EXPECT_STREQ(std::string(tested[1]).c_str(), "alma");
    EXPECT_STREQ(std::string(tested[2]).c_str(), "");
    EXPECT_STREQ(std::string(tested[3]).c_str(), "korte");
    EXPECT_STREQ(std::string(tested[4]).c_str(), "");
}

TEST(storing_string_view, construction_from_string_view)
{
    std::string source(",alma,,korte,");
    cStringViewVector tested(std::string_view(source), ", ", true);
    EXPECT_EQ(tested.size(), 5);
    EXPECT_STREQ(std::string(tested[0]).c_str(), "");
    EXPECT_STREQ(std::string(tested[1]).c_str(), "alma");
    EXPECT_STREQ(std::string(tested[2]).c_str(), "");
    EXPECT_STREQ(std::string(tested[3]).c_str(), "korte");
    EXPECT_STREQ(std::string(tested[4]).c_str(), "");
}

TEST(construction, fromIntVector)
{
    cIntVector intVector = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cStringVector tested;
    tested.fromIntVector(intVector);
    EXPECT_EQ(tested.size(), 10);
    for (int i = 0; i < 10; i++)
        EXPECT_STREQ(tested[i].c_str(), std::to_string(i + 1).c_str());
}

TEST(utility, trimAll)
{
    cStringVector tested("alma korte, banan ,     citrom   eper   ", ",", true);
    tested.trimAll();
    EXPECT_EQ(tested.size(), 3);
    EXPECT_STREQ(tested[0].c_str(), "alma korte");
    EXPECT_STREQ(tested[1].c_str(), "banan");
    EXPECT_STREQ(tested[2].c_str(), "citrom   eper");
}

TEST(utility, toIntVector)
{
    cStringVector tested("1, 2, 3, 4, 5, 6 ,7,8,  9   ,10,11 ", ",", true);
    cIntVector intVector = tested.toIntVector();
    EXPECT_EQ(intVector.size(), 11);
    for (int i = 0; i < 11; i++)
        EXPECT_EQ(intVector[i], i + 1);
}

TEST(storing_string_views, toIntVector)
{
    cStringViewVector tested("1, 2AAA, -353, 4, 5, 6 ,7,8,  9   ,10,11 ", ",", true);
    cIntVector intVector = tested.toIntVector();
    EXPECT_EQ(intVector.size(), 11);
    for (int i = 0; i < 11; i++)
    {
        if (i != 2)
            EXPECT_EQ(intVector[i], i + 1);
        else
            EXPECT_EQ(intVector[i], -353);
    }
}

TEST(utility, toString)
{
    cStringVector tested("alma, korte, banan, citrom, eper, alma", ", ", false);
    EXPECT_STREQ(tested.toString(";").c_str(), "alma;korte;banan;citrom;eper;alma");
    EXPECT_STREQ(tested.toString(", ").c_str(), "alma, korte, banan, citrom, eper, alma");
}

} // namespace StringVectorTests
