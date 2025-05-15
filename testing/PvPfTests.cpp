#include <filesystem>

#include "PvTestMain.hpp"
#include "PvPf_Windows.hpp"

// テストケース

TEST(PvPf_Windows, pvPfInitialize_correct)
{
    EXPECT_NO_THROW(pvPfInitialize(ApplicationInstance));
    EXPECT_EQ(pvPfGetApplicationInstance(), ApplicationInstance);
    EXPECT_NO_THROW(pvPfShutdown());
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
}

TEST(PvPf_Windows, pvPfInitialize_invalid_instance)
{
    EXPECT_ANY_THROW(pvPfInitialize(reinterpret_cast<HINSTANCE>(-1)));
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
    EXPECT_ANY_THROW(pvPfShutdown());
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
}

TEST(PvPf_Windows, pvPfInitialize_invalid_operation)
{
    EXPECT_NO_THROW(pvPfInitialize(ApplicationInstance));
    EXPECT_ANY_THROW(pvPfInitialize(ApplicationInstance));
    EXPECT_EQ(pvPfGetApplicationInstance(), ApplicationInstance);
    EXPECT_NO_THROW(pvPfShutdown());
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
}

TEST(PvPf_Windows, pvPfInitialize_invalid_operation2)
{
    EXPECT_NO_THROW(pvPfInitialize(ApplicationInstance));
    EXPECT_ANY_THROW(pvPfInitialize(reinterpret_cast<HINSTANCE>(-1)));
    EXPECT_EQ(pvPfGetApplicationInstance(), ApplicationInstance);
    EXPECT_NO_THROW(pvPfShutdown());
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
}

TEST(PvPf_Windows, pvPfInitialize_null_instance)
{
    EXPECT_ANY_THROW(pvPfInitialize(nullptr));
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
    EXPECT_ANY_THROW(pvPfShutdown());
}

TEST(PvPf_Windows, pvPfInitialize_null_instance2)
{
    EXPECT_ANY_THROW(pvPfInitialize(nullptr));
    EXPECT_ANY_THROW(pvPfInitialize(nullptr));
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
    EXPECT_ANY_THROW(pvPfShutdown());
}


TEST(PvPf_Windows, pvPfShutdown_invalid_operation)
{
    EXPECT_NO_THROW(pvPfInitialize(ApplicationInstance));
    EXPECT_NO_THROW(pvPfShutdown());
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
    EXPECT_ANY_THROW(pvPfShutdown());
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
}

TEST(PvPf_Windows, pvPfShutdown_invalid_operation2)
{
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
    EXPECT_ANY_THROW(pvPfShutdown());
    EXPECT_EQ(pvPfGetApplicationInstance(), nullptr);
}

TEST(PvPf_Windows, pvPfIoRead_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvPfFileInfo fileInfo;
        const char8_t* path = u8"..\\..\\..\\testing\\data\\utf8_crlf.txt";

        EXPECT_EQ(pvPfIoOpen(path, fileInfo), true);
        EXPECT_EQ(pvPfIoRead(fileInfo), true);
        EXPECT_NO_THROW(pvPfIoClose(fileInfo););
    }
    {
        PvPfFileInfo fileInfo;
        const char8_t* path = u8"..\\..\\..\\testing\\data\\シフトジス.txt";

        EXPECT_EQ(pvPfIoOpen(path, fileInfo), true);
        EXPECT_EQ(pvPfIoRead(fileInfo), true);
        EXPECT_NO_THROW(pvPfIoClose(fileInfo););
    }
    {
        PvPfFileInfo fileInfo;
        const char8_t* path = u8"..\\..\\..\\testing\\data\\信息交换用汉字编码字符集・基本集.txt";

        EXPECT_EQ(pvPfIoOpen(path, fileInfo), true);
        EXPECT_EQ(pvPfIoRead(fileInfo), true);
        EXPECT_NO_THROW(pvPfIoClose(fileInfo););
    }
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvPf_Windows, pvPfIoWrite_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        std::u8string testWritingData = u8"せモタKlPゼのCヱッッ;はセニ:Dいとlぜlヂねんぢヒぺョ!づマヵTJィボチTェツtゴ1|ミくゆト";
        PvPfFileInfo fileInfo;
        auto result = false;
        std::filesystem::path path = u8"..\\..\\..\\testing\\data\\out_test.txt";
        path = pvPfGetModulePath() / path;

        EXPECT_NO_THROW(std::filesystem::remove(path));

        EXPECT_EQ(result = pvPfIoOpen(path.u8string().c_str(), fileInfo, PvPfIoOpenControlFlag_Write), false);
        if (result)
        {
            EXPECT_NO_THROW(pvPfIoClose(fileInfo));
        }

        EXPECT_EQ(result = pvPfIoOpen(path.u8string().c_str(), fileInfo, PvPfIoOpenControlFlag_Write | PvPfIoOpenControlFlag_CreateNew), true);
        if (result)
        {
            EXPECT_EQ(pvPfIoExpandData(fileInfo, testWritingData.size()), true);
            std::ranges::copy(testWritingData, fileInfo.GetData().data());
            EXPECT_EQ(pvPfIoWrite(fileInfo), true);
            EXPECT_NO_THROW(pvPfIoClose(fileInfo););
        }

        EXPECT_EQ(result = pvPfIoOpen(path.u8string().c_str(), fileInfo, PvPfIoOpenControlFlag_Write), true);
        if (result)
        {
            EXPECT_EQ(pvPfIoRead(fileInfo), true);
            EXPECT_EQ(std::ranges::equal(testWritingData, fileInfo.GetData()), true);
            EXPECT_EQ(pvPfIoExpandData(fileInfo, testWritingData.size() + fileInfo.GetData().size_bytes(), false), true);
            std::ranges::copy(testWritingData, fileInfo.GetData().data() + testWritingData.size());
            EXPECT_EQ(pvPfIoWrite(fileInfo), true);
            EXPECT_NO_THROW(pvPfIoClose(fileInfo););
        }
    }
    ASSERT_NO_THROW(pvPfShutdown());
}
