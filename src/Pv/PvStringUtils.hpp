#pragma once

#include <filesystem>
#include <string>


extern std::u8string_view pvStringTrim(const std::u8string_view& line);

inline const char* pvStringFmtAnsi(const char8_t* s)
{
    return reinterpret_cast<const char*>(s);
}

inline const char* pvStringFmtAnsi(const char* s)
{
    return s;
}

inline const char8_t* pvStringFmtU8(const char* s)
{
    return reinterpret_cast<const char8_t*>(s);
}


inline const void* pvStringFmtPointer(void* p)
{
    return p;
}

inline const void* pvStringFmtPointer(const void* p)
{
    return p;
}

inline std::filesystem::path& pvStringMakeAbsolutePath(std::filesystem::path& path,
                                                       const std::filesystem::path& baseDirectoryPath)
{
    if (path.is_relative())
    {
        path = baseDirectoryPath / path;
    }

    return path;
}
