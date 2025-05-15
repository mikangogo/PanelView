#include "PvLogHandlerParser.hpp"

#include <array>

namespace
{
    std::array<const char*, PvLogHandlerParser::HeaderReason_Max> HeaderReasonMessages = 
    {
        "[Info]",
        "Invalid encoding",
        "Encoding is not supported",
        "Broken header",
        "Encoding conversion is failed",
        "Invalid version",
    };

    const char* GetHeaderReasonMessage(PvLogHandlerParser::HeaderReason reason)
    {
        return HeaderReasonMessages[reason];
    }
}

void PvLogHandlerParser::SetDocumentPath(const std::filesystem::path& path)
{
    _documentPath = path;
}

const std::filesystem::path& PvLogHandlerParser::GetDocumentPath() const
{
    return _documentPath;
}

void PvLogHandlerParser::PrintParseHeaderMessage(HeaderReason reason, const std::u8string& message) const
{
    auto headerReasonMessage = GetHeaderReasonMessage(reason);

    auto mes = reinterpret_cast<const char*>(message.c_str());
    auto s = std::format("{:s}: {:s}", headerReasonMessage, mes);

    PrintParseMessage(1, std::u8string(reinterpret_cast<const char8_t*>(s.c_str())));
}

void PvLogHandlerParser::PrintParseHeaderMessage(HeaderReason reason, const std::string& message) const
{
    auto u8 = std::u8string(reinterpret_cast<const char8_t*>(message.c_str()));
    PrintParseHeaderMessage(reason, u8);
}

void PvLogHandlerParser::PrintParseMessage(int line, const std::u8string& message) const
{
    auto documentPath = GetDocumentPath().u8string();
    auto mes = reinterpret_cast<const char*>(message.c_str());
    auto s = std::format(":{:d}: {:s}", line, mes);


    PrintLine(documentPath + std::u8string(reinterpret_cast<const char8_t*>(s.c_str())));
}

void PvLogHandlerParser::PrintParseMessage(int line, const std::string& message) const
{
    auto u8 = std::u8string(reinterpret_cast<const char8_t*>(message.c_str()));
    PrintParseMessage(line, u8);
}
