#pragma once

#include <filesystem>

#include "PvLogHandler.hpp"

class PvLogHandlerParser : PvLogHandler
{
public:
    enum HeaderReason
    {
        HeaderReason_Information = 0,
        HeaderReason_InvalidEncoding,
        HeaderReason_NoSupportedEncoding,
        HeaderReason_BrokenHeader,
        HeaderReason_FailedEncodingConversion,
        HeaderReason_InvalidVersion,
        HeaderReason_Max,
    };

    void SetDocumentPath(const std::filesystem::path& path);
    const std::filesystem::path& GetDocumentPath() const;

    void PrintParseHeaderMessage(HeaderReason reason, const std::u8string& message) const;
    void PrintParseHeaderMessage(HeaderReason reason, const std::string& message) const;

    void PrintParseMessage(int line, const std::u8string& message) const;
    void PrintParseMessage(int line, const std::string& message) const;

private:
    std::filesystem::path _documentPath;
};
