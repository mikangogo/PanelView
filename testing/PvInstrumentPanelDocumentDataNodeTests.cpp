#include "PvTestMain.hpp"
#include "PvPf_Windows.hpp"
#include "PvInstrumentPanelDocumentDataNode.hpp"


TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_Value_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"AaBb"> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"153.489"), true);
        EXPECT_DOUBLE_EQ(value.Value, 153.489);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"0.2558"), true);
        EXPECT_DOUBLE_EQ(value.Value, 0.2558);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"0"), true);
        EXPECT_DOUBLE_EQ(value.Value, 0.0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"1"), true);
        EXPECT_DOUBLE_EQ(value.Value, 1.0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8" 4.28 "), true);
        EXPECT_DOUBLE_EQ(value.Value, 4.28);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_Value_wrong)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"AaBb"> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"A", u8"153.489"), false);
        EXPECT_DOUBLE_EQ(value.Value, 0.0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"gyt"), false);
        EXPECT_DOUBLE_EQ(value.Value, 0.0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8""), false);
        EXPECT_DOUBLE_EQ(value.Value, 0.0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"1.5, 2.8"), false);
        EXPECT_DOUBLE_EQ(value.Value, 0.0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8","), false);
        EXPECT_DOUBLE_EQ(value.Value, 0.0);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<u8"AaBb", 0, true> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"28.574, 45.387"), true);
        EXPECT_DOUBLE_EQ(value.Value.at(0), 28.574);
        EXPECT_DOUBLE_EQ(value.Value.at(1), 45.387);
        EXPECT_THROW(auto n = value.Value.at(2), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"2.68 , 387,  5.0"), true);
        EXPECT_DOUBLE_EQ(value.Value.at(0), 2.68);
        EXPECT_DOUBLE_EQ(value.Value.at(1), 387);
        EXPECT_DOUBLE_EQ(value.Value.at(2), 5.0);
        EXPECT_THROW(auto n = value.Value.at(3), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"    5.0 , 2268.89    "), true);
        EXPECT_DOUBLE_EQ(value.Value.at(0), 5.0);
        EXPECT_DOUBLE_EQ(value.Value.at(1), 2268.89);
        EXPECT_THROW(auto n = value.Value.at(2), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"1.5"), true);
        EXPECT_DOUBLE_EQ(value.Value.at(0), 1.5);
        EXPECT_THROW(auto n = value.Value.at(1), std::out_of_range);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray_wrong)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<u8"AaBb", 0, true> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"A", u8"2.5, 3.2"), false);
        EXPECT_THROW(auto n = value.Value.at(0), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"erte, oiuyii"), false);
        EXPECT_THROW(auto n = value.Value.at(0), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"2.26, 2.yii"), false);
        EXPECT_THROW(auto n = value.Value.at(0), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"   2 .26,   3. 1 4"), false);
        EXPECT_THROW(auto n = value.Value.at(0), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"   4 . 8 2,   3 . 1 4"), false);
        EXPECT_THROW(auto n = value.Value.at(0), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"   5. 76,   3 . 1 4"), false);
        EXPECT_THROW(auto n = value.Value.at(0), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8","), false);
        EXPECT_THROW(auto n = value.Value.at(0), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8" ,"), false);
        EXPECT_THROW(auto n = value.Value.at(0), std::out_of_range);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"2.5| 3.2"), false);
        EXPECT_THROW(auto n = value.Value.at(0), std::out_of_range);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataKeyValuePair_Subject_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataKeyValuePair_Subject<u8"AaBb"> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"KmPh"), true);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"Ats0"), true);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"Doorr2"), true);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"Doorl54"), true);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8" KmPh "), true);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataKeyValuePair_Subject_wrong)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataKeyValuePair_Subject<u8"AaBb"> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"Aa", u8"KmPh"), false);
        EXPECT_EQ(value.SubjectKey, PvInstrumentPanelSubjectKeyTitle_Nop);
        EXPECT_EQ(value.Index, 0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"AtsEx"), false);
        EXPECT_EQ(value.SubjectKey, PvInstrumentPanelSubjectKeyTitle_Nop);
        EXPECT_EQ(value.Index, 0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"Doorr-1"), false);
        EXPECT_EQ(value.SubjectKey, PvInstrumentPanelSubjectKeyTitle_Nop);
        EXPECT_EQ(value.Index, 0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"0"), false);
        EXPECT_EQ(value.SubjectKey, PvInstrumentPanelSubjectKeyTitle_Nop);
        EXPECT_EQ(value.Index, 0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"1"), false);
        EXPECT_EQ(value.SubjectKey, PvInstrumentPanelSubjectKeyTitle_Nop);
        EXPECT_EQ(value.Index, 0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"35"), false);
        EXPECT_EQ(value.SubjectKey, PvInstrumentPanelSubjectKeyTitle_Nop);
        EXPECT_EQ(value.Index, 0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"Doorl1.0"), false);
        EXPECT_EQ(value.SubjectKey, PvInstrumentPanelSubjectKeyTitle_Nop);
        EXPECT_EQ(value.Index, 0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"Doorl1.5"), false);
        EXPECT_EQ(value.SubjectKey, PvInstrumentPanelSubjectKeyTitle_Nop);
        EXPECT_EQ(value.Index, 0);
        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"Doorr 1"), false);
        EXPECT_EQ(value.SubjectKey, PvInstrumentPanelSubjectKeyTitle_Nop);
        EXPECT_EQ(value.Index, 0);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_Color_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"AaBb"> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"FFFFFF"), true);
        EXPECT_DOUBLE_EQ(value.R, 1.0);
        EXPECT_DOUBLE_EQ(value.G, 1.0);
        EXPECT_DOUBLE_EQ(value.B, 1.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"000000"), true);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"7F0000"), true);
        EXPECT_DOUBLE_EQ(value.R, 127.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"007F00"), true);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 127.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"00007F"), true);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 127.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"605040"), true);
        EXPECT_DOUBLE_EQ(value.R, 96.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.G, 80.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.B, 64.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"ABCDEF"), true);
        EXPECT_DOUBLE_EQ(value.R, 171.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.G, 205.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.B, 239.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);





        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#FFFFFF"), true);
        EXPECT_DOUBLE_EQ(value.R, 1.0);
        EXPECT_DOUBLE_EQ(value.G, 1.0);
        EXPECT_DOUBLE_EQ(value.B, 1.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#000000"), true);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#7F0000"), true);
        EXPECT_DOUBLE_EQ(value.R, 127.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#007F00"), true);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 127.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#00007F"), true);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 127.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#605040"), true);
        EXPECT_DOUBLE_EQ(value.R, 96.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.G, 80.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.B, 64.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#ABCDEF"), true);
        EXPECT_DOUBLE_EQ(value.R, 171.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.G, 205.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.B, 239.0 / 255.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_Color_wrong)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"AaBb"> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"aBb", u8"FFFFFF"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"0"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"0862"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"523"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"94864"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"5861589"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"84516854"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"BCDEFG"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"54.248"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"FF FFF"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8" FFFFF"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"FFFFF "), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);




        EXPECT_EQ(value.TryParseFromKeyValue(u8"aBb", u8"#FFFFFF"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#0"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#0862"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#523"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#94864"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#5861589"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#84516854"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#BCDEFG"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#54.248"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#FF FFF"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"# FFFFF"), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"#FFFFF "), false);
        EXPECT_DOUBLE_EQ(value.R, 0.0);
        EXPECT_DOUBLE_EQ(value.G, 0.0);
        EXPECT_DOUBLE_EQ(value.B, 0.0);
        EXPECT_DOUBLE_EQ(value.A, 1.0);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_Flag_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_Flag<u8"AaBb"> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"true"), true);
        EXPECT_EQ(value.Value, true);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"false"), true);
        EXPECT_EQ(value.Value, false);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8" true "), true);
        EXPECT_EQ(value.Value, true);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_Flag_wrong)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_Flag<u8"AaBb"> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"yes"), false);
        EXPECT_EQ(value.Value, false);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"no"), false);
        EXPECT_EQ(value.Value, false);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"0"), false);
        EXPECT_EQ(value.Value, false);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"0.5"), false);
        EXPECT_EQ(value.Value, false);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle<u8"AaBb", 0, true> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"45, 180.0"), true);
        EXPECT_DOUBLE_EQ(value.Value[0], 45.0);
        EXPECT_DOUBLE_EQ(value.Value[1], 180.0);
        EXPECT_THROW(auto n = value.Value.at(2), std::out_of_range);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"32, 48, 20"), true);
        EXPECT_DOUBLE_EQ(value.Value[0], 32.0);
        EXPECT_DOUBLE_EQ(value.Value[1], 48.0);
        EXPECT_DOUBLE_EQ(value.Value[2], 20.0);
        EXPECT_THROW(auto n = value.Value.at(32), std::out_of_range);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"2"), true);
        EXPECT_DOUBLE_EQ(value.Value[0], 2.0);
        EXPECT_THROW(auto n = value.Value.at(1), std::out_of_range);

    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentDataNodeTests, PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle_fail)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle<u8"AaBb", 0, true> value;

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8"0f"), false);
        EXPECT_EQ(value.Value.empty(), true);

        EXPECT_EQ(value.TryParseFromKeyValue(u8"AaBb", u8","), false);
        EXPECT_EQ(value.Value.empty(), true);

    }
    ASSERT_NO_THROW(pvPfShutdown());
}
