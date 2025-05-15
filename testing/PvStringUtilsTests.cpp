#include "PvStringUtils.hpp"
#include "PvPf_Windows.hpp"
#include "PvTestMain.hpp"

#include <gtest/gtest.h>

TEST(PvStringUtilsTests, PvStringUtils_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        std::u8string_view source, expect;

        source = u8"aaaaa";
        expect = u8"aaaaa";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8"    aaaaa";
        expect = u8"aaaaa";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8"aaaaa    ";
        expect = u8"aaaaa";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8"  aaaaa  ";
        expect = u8"aaaaa";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8" aaaaa";
        expect = u8"aaaaa";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8"aaaaa ";
        expect = u8"aaaaa";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8"  aaaaa ";
        expect = u8"aaaaa";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8" aaaaa  ";
        expect = u8"aaaaa";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8" ";
        expect = u8"";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8"";
        expect = u8"";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8" a";
        expect = u8"a";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8"a ";
        expect = u8"a";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8" a ";
        expect = u8"a";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8"\ta ";
        expect = u8"a";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8"\ta\t";
        expect = u8"a";
        EXPECT_EQ(pvStringTrim(source), expect);

        source = u8" \ta \t ";
        expect = u8"a";
        EXPECT_EQ(pvStringTrim(source), expect);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}
