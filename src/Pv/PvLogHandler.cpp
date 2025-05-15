#include "PvLogHandler.hpp"

#include "PvPf_Windows.hpp"

void PvLogHandler::SetLogFunction(LogFunction logFunction)
{
    _logFunction = logFunction;
}

PvLogHandler::LogFunction PvLogHandler::GetLogFunction() const
{
    return _logFunction;
}

void PvLogHandler::PrintLine(const std::u8string& message) const
{
    if (_logFunction)
    {
        _logFunction(message.c_str());
        return;
    }

    pvPfDebugPrintLine(message);
}

void PvLogHandler::PrintLine(const std::string& message) const
{
    auto u8 = std::u8string(reinterpret_cast<const char8_t*>(message.c_str()));
    PrintLine(u8);
}
