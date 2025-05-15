#include "PvTestMain.hpp"

#include "PvPf_Windows.hpp"
#include "PvPanelWindow.hpp"
#include "PvInstrumentPanelGenerator.hpp"
#include "PvInstrumentPanelDocument.hpp"
#include "PvInstrumentPanelObject.hpp"


TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_Create_utf8_crlf_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocument pviDoc;
        PvPfFileInfo fileInfo;
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_utf8_crlf.txt", fileInfo), true));
        ASSERT_NO_THROW(
            ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), true));
        EXPECT_EQ(pviDoc.GetDocument().empty(), false);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_Create_utf8_bom_lf_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocument pviDoc;
        PvPfFileInfo fileInfo;

        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_utf8_bom_lf.txt", fileInfo), true));
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), true));
        EXPECT_EQ(pviDoc.GetDocument().empty(), false);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_Create_utf16_crlf_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocument pviDoc;
        PvPfFileInfo fileInfo;

        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_utf16_crlf.txt", fileInfo), true));
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), true));
        EXPECT_EQ(pviDoc.GetDocument().empty(), false);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_Create_utf16be_lf_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocument pviDoc;
        PvPfFileInfo fileInfo;

        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_utf16be_lf.txt", fileInfo), true));
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), true));
        EXPECT_EQ(pviDoc.GetDocument().empty(), false);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_Create_utf16be_ctlf_correct)
{

    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocument pviDoc;
        PvPfFileInfo fileInfo;

        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_utf16be_crlf.txt", fileInfo), true));
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), true));
        EXPECT_EQ(pviDoc.GetDocument().empty(), false);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_Create_utf32_crlf_fail)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocument pviDoc;
        PvPfFileInfo fileInfo;

        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_utf32_crlf.txt", fileInfo), true));
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), false));
        EXPECT_EQ(pviDoc.GetDocument().empty(), true);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_Create_utf32be_crlf_fail)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocument pviDoc;
        PvPfFileInfo fileInfo;

        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_utf32be_crlf.txt", fileInfo), true));
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), false));
        EXPECT_EQ(pviDoc.GetDocument().empty(), true);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_Create_sjis_crlf_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocument pviDoc;
        PvPfFileInfo fileInfo;

        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_sjis_crlf.txt", fileInfo), true));
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), true));
        EXPECT_EQ(pviDoc.GetDocument().empty(), false);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_Create_gb2312_crlf_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvInstrumentPanelDocument pviDoc;
        PvPfFileInfo fileInfo;

        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_gb2312_crlf.txt", fileInfo), true));
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), true));
        EXPECT_EQ(pviDoc.GetDocument().empty(), false);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvInstrumentPanelDocumentTests, PvInstrumentPanelDocument_parse_utf8_bom_lf_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    ASSERT_NO_THROW(pvGrInitialize());
    {
        PvInstrumentPanelDocument pviDoc;
        PvInstrumentPanelObject* pviObj;
        PvPfFileInfo fileInfo;
        PvWindowBase* window;
        PvGrContextHandle context;

        ASSERT_NO_THROW(ASSERT_NE(window = new PvWindowBase(), nullptr));
        ASSERT_NO_THROW(window->Create(u8"Testing"));
        ASSERT_NO_THROW(window->SetVisible(true));
        ASSERT_NO_THROW(ASSERT_NE(context = pvGrCreateContext(*window), nullptr));

        ASSERT_NO_THROW(
            ASSERT_EQ(pvPfIoOpen(u8"..\\..\\..\\testing\\data\\panel_utf8_bom_lf_correct.txt", fileInfo), true));
        ASSERT_NO_THROW(ASSERT_EQ(pvPfIoRead(fileInfo), true));
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelDocument::LoadFromFile(fileInfo, pviDoc), true));
        EXPECT_DOUBLE_EQ(pviDoc.GetThis().Resolution.Value, 2048.0);
        EXPECT_EQ(pviDoc.GetPilotLampCount(), 13);
        EXPECT_EQ(pviDoc.GetNeedleCount(), 7);

        EXPECT_NO_THROW(pviObj = new PvInstrumentPanelObject());
        EXPECT_NO_THROW(EXPECT_EQ(PvInstrumentPanelGenerator::DocumentToObject(context, *pviObj, pviDoc), true));

        EXPECT_NO_THROW(delete pviObj);
        ASSERT_NO_THROW(pvGrDeleteContext(context));
        ASSERT_NO_THROW(window->Delete());
        ASSERT_NO_THROW(delete window);

    }
    ASSERT_NO_THROW(pvGrShutdown());
    ASSERT_NO_THROW(pvPfShutdown());
}
