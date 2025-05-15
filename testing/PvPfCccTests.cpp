#include "PvTestMain.hpp"
#include "PvPf_Windows.hpp"

#include <gtest/gtest.h>

// テストケース
TEST(PvPfCcc_Windows, pvPfCccMultiByteToUtf16_gb2312_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));

    const char16_t* u16expected = u"信息交换用汉字编码字符集\x00b7基本集";
    const auto u16expectedLength = wcsnlen_s(reinterpret_cast<wchar_t const*>(u16expected), 128);
    const char8_t* u8str = u8"信息交换用汉字编码字符集\u00b7基本集";
    const auto u8strLength = strnlen_s(reinterpret_cast<char const*>(u8str), 128);
    size_t u16strLength = 128;
    std::unique_ptr<char16_t[]> u16str = std::make_unique<char16_t[]>(u16strLength);
    constexpr auto gb2312strLength = 128;
    std::unique_ptr<char[]> gb2312str = std::make_unique<char[]>(gb2312strLength);

    EXPECT_EQ(u16strLength = pvPfCccUtf8ToUtf16(u16str.get(), u16strLength, u8str, u8strLength), u16expectedLength + 1);

    EXPECT_NE(WideCharToMultiByte(936U, 0, reinterpret_cast<wchar_t const*>(u16str.get()), static_cast<int>(u16strLength), gb2312str.get(), gb2312strLength, nullptr, nullptr), 0);
    auto er = GetLastError();
    u16str = std::make_unique<char16_t[]>(u16strLength);
    EXPECT_EQ(pvPfCccMultiByteToUtf16(u16str.get(), u16strLength, gb2312str.get(), gb2312strLength, 936U), u16expectedLength + 1);

    EXPECT_EQ(wcsncmp(reinterpret_cast<wchar_t const*>(u16expected), reinterpret_cast<wchar_t*>(u16str.get()), u16strLength), 0);
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvPfCcc_Windows, pvPfCccMultiByteToUtf16_sjis_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));

    const char16_t* u16expected = u"あいうえお今晩わ";
    const auto u16expectedLength = wcsnlen_s(reinterpret_cast<wchar_t const*>(u16expected), 128);
    const char* sjisStr = "あいうえお今晩わ";
    const auto sjisStrLength = strnlen_s(sjisStr, 128);
    constexpr auto u16strLength = 128;
    std::unique_ptr<char16_t[]> u16str = std::make_unique<char16_t[]>(u16strLength);

    EXPECT_EQ(pvPfCccMultiByteToUtf16(u16str.get(), u16strLength, sjisStr, sjisStrLength, 932U), u16expectedLength + 1);
    EXPECT_EQ(wcsncmp(reinterpret_cast<wchar_t const*>(u16expected), reinterpret_cast<wchar_t*>(u16str.get()), u16strLength), 0);
    ASSERT_NO_THROW(pvPfShutdown());
}

#pragma warning(push)
#pragma warning(disable: 4566)
TEST(PvPfCcc_Windows, pvPfCccUtf8ToUtf16_u8_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        const char16_t* u16expected = u"あいうえお𠮷今晩わ💕";
        const auto u16expectedLength = wcsnlen_s(reinterpret_cast<wchar_t const*>(u16expected), 128);
        const char8_t* u8str = u8"あいうえお𠮷今晩わ💕";
        const auto u8strLength = strnlen_s(reinterpret_cast<char const*>(u8str), 128);
        constexpr auto u16strLength = 128;
        std::unique_ptr<char16_t[]> u16str = std::make_unique<char16_t[]>(u16strLength);

        EXPECT_EQ(pvPfCccUtf8ToUtf16(u16str.get(), u16strLength, u8str, u8strLength), u16expectedLength + 1);
        EXPECT_EQ(wcsncmp(reinterpret_cast<wchar_t const*>(u16expected), reinterpret_cast<wchar_t*>(u16str.get()),
                          u16strLength),
                  0);
    }
    {
        const char16_t* u16expected = u"あいうえお𠮷今晩わ💕";
        const auto u16expectedLength = wcsnlen_s(reinterpret_cast<wchar_t const*>(u16expected), 128);
        const char8_t* u8str = u8"あいうえお𠮷今晩わ💕";
        const auto u8strLength = strnlen_s(reinterpret_cast<char const*>(u8str), 128);
        constexpr auto u16strLength = 128;
        std::unique_ptr<char16_t[]> u16str = std::make_unique<char16_t[]>(u16strLength);
        auto u16char = u16str[0];

        EXPECT_EQ(pvPfCccUtf8ToUtf16(nullptr, u16strLength, u8str, u8strLength), u16expectedLength + 1);
        EXPECT_EQ(pvPfCccUtf8ToUtf16(u16str.get(), 0, u8str, u8strLength), u16expectedLength + 1);
        EXPECT_EQ(u16str[0] == u16char, true);
    }
    {
        const char16_t* u16expected = u"";
        const auto u16expectedLength = wcsnlen_s(reinterpret_cast<wchar_t const*>(u16expected), 128);
        const char8_t* u8str = u8"";
        const auto u8strLength = strnlen_s(reinterpret_cast<char const*>(u8str), 128);
        constexpr auto u16strLength = 128;
        std::unique_ptr<char16_t[]> u16str = std::make_unique<char16_t[]>(u16strLength);

        EXPECT_EQ(pvPfCccUtf8ToUtf16(u16str.get(), u16strLength, u8str, u8strLength), u16expectedLength);
        EXPECT_EQ(wcsncmp(L"", reinterpret_cast<wchar_t*>(u16str.get()), u16strLength), 0);
    }
    ASSERT_NO_THROW(pvPfShutdown());
}
#pragma warning(pop)

TEST(PvPfCcc_Windows, pvPfCccUtf8ToUtf16_u8_dest_buffer_too_small)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));

    const char8_t* u8str = u8"あいうえお𠮷今晩わ💕";
    const auto u8strLength = strnlen_s(reinterpret_cast<char const*>(u8str), 128);
    constexpr auto u16strLength = 4;
    std::unique_ptr<char16_t[]> u16str = std::make_unique<char16_t[]>(u16strLength);

    EXPECT_EQ(pvPfCccUtf8ToUtf16(u16str.get(), u16strLength, u8str, u8strLength), 0);
    ASSERT_NO_THROW(pvPfShutdown());
}


TEST(PvPfCcc_Windows, pvPfCccUtf8ToUtf16_invalid_operation)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        const char8_t* u8str = u8"あいうえお𠮷今晩わ💕";
        const auto u8strLength = strnlen_s(reinterpret_cast<char const*>(u8str), 128);
        constexpr auto u16strLength = 128;
        std::unique_ptr<char16_t[]> u16str = std::make_unique<char16_t[]>(u16strLength);

        EXPECT_EQ(pvPfCccUtf8ToUtf16(u16str.get(), u16strLength, nullptr, u8strLength), 0);
    }
    {
        const char8_t* u8str = u8"あいうえお𠮷今晩わ💕";
        constexpr auto u16strLength = 128;
        std::unique_ptr<char16_t[]> u16str = std::make_unique<char16_t[]>(u16strLength);

        EXPECT_EQ(pvPfCccUtf8ToUtf16(u16str.get(), u16strLength, u8str, 0), 0);
    }

    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvPfCcc_Windows, pvPfCccMultiByteToUtf8_sjis_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));

    const auto* u8expected = u8"あいうえお今晩わ";
    const auto u8expectedLength = strnlen_s(reinterpret_cast<char const*>(u8expected), 128);
    const char* sjisStr = "あいうえお今晩わ";
    const auto sjisStrLength = strnlen_s(sjisStr, 128);
    constexpr auto u8strLength = 128;
    std::unique_ptr<char8_t[]> u8str = std::make_unique<char8_t[]>(u8strLength);

    EXPECT_EQ(pvPfCccMultiByteToUtf8(u8str.get(), u8strLength, sjisStr, sjisStrLength, 932U), u8expectedLength + 1);
    EXPECT_EQ(strncmp(reinterpret_cast<char const*>(u8expected), reinterpret_cast<char const*>(u8str.get()), u8strLength), 0);
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvPfCcc_Windows, pvPfCccMultiByteToUtf8_gb2312_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));

    const auto* u8expected = u8"信息交换用汉字编码字符集\x00b7基本集";
    const auto u8expectedLength = strnlen_s(reinterpret_cast<char const*>(u8expected), 128);
    const auto* u16expected = u"信息交换用汉字编码字符集\x00b7基本集";
    const auto u16expectedLength = wcsnlen_s(reinterpret_cast<wchar_t const*>(u16expected), 128);
    std::unique_ptr<char8_t[]> u8str;
    size_t u16strLength = 128;
    std::unique_ptr<char16_t[]> u16str = std::make_unique<char16_t[]>(u16strLength);
    constexpr auto gb2312strLength = 128;
    std::unique_ptr<char[]> gb2312str = std::make_unique<char[]>(gb2312strLength);

    EXPECT_EQ(u16strLength = pvPfCccUtf8ToUtf16(u16str.get(), u16strLength, u8expected, u8expectedLength), u16expectedLength + 1);

    EXPECT_NE(WideCharToMultiByte(936U, 0, reinterpret_cast<wchar_t const*>(u16str.get()), static_cast<int>(u16strLength), gb2312str.get(), gb2312strLength, nullptr, nullptr), 0);
    auto er = GetLastError();
    constexpr auto u8strLength = 128U;
    u8str = std::make_unique<char8_t[]>(u8strLength);
    EXPECT_EQ(pvPfCccMultiByteToUtf8(u8str.get(), u8strLength, gb2312str.get(), gb2312strLength, 936U), u8expectedLength + 1);

    EXPECT_EQ(strncmp(reinterpret_cast<char const*>(u8expected), reinterpret_cast<char* const>(u8str.get()), u8expectedLength), 0);
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvPfCcc_Windows, pvPfCccGetCodePageFromName_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    EXPECT_NO_THROW(EXPECT_EQ(pvPfCccGetCodePageFromName(u8"utf-8"), 65001));
    EXPECT_NO_THROW(EXPECT_EQ(pvPfCccGetCodePageFromName(u8"UTF-8"), 65001));
    EXPECT_NO_THROW(EXPECT_EQ(pvPfCccGetCodePageFromName(u8"Utf-8"), 65001));
    EXPECT_NO_THROW(EXPECT_EQ(pvPfCccGetCodePageFromName(u8"Shift_JIS"), 932));
    EXPECT_NO_THROW(EXPECT_EQ(pvPfCccGetCodePageFromName(u8"shift_jis"), 932));
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvPfCcc_Windows, pvPfCccGetCodePageFromName_failed)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    EXPECT_NO_THROW(EXPECT_EQ(pvPfCccGetCodePageFromName(u8"utf"), 0));
    EXPECT_NO_THROW(EXPECT_EQ(pvPfCccGetCodePageFromName(u8"utf8"), 0));
    EXPECT_NO_THROW(EXPECT_EQ(pvPfCccGetCodePageFromName(u8""), 0));
    EXPECT_NO_THROW(EXPECT_EQ(pvPfCccGetCodePageFromName(nullptr), 0));
    ASSERT_NO_THROW(pvPfShutdown());
}

TEST(PvPfCcc_Windows, pvPfCccGetCodePageFromName_invalid_operation)
{
    EXPECT_ANY_THROW(pvPfCccGetCodePageFromName(u8"utf-8"));
}
