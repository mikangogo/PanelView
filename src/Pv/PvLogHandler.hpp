#pragma once

#include <string>


class PvLogHandler
{
public:
    typedef void (*LogFunction)(const char8_t* message);

    void SetLogFunction(LogFunction logFunction);
    LogFunction GetLogFunction() const;

protected:
    void PrintLine(const std::u8string& message) const;
    void PrintLine(const std::string& message) const;

private:
    LogFunction _logFunction = nullptr;
};
