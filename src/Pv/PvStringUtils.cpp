#include "PvStringUtils.hpp"

std::u8string_view pvStringTrim(const std::u8string_view& line)
{
    auto trimmedBegin = line.begin();
    auto trimmedEnd = line.rbegin();

    if (line.empty())
    {
        return line;
    }

    while (trimmedBegin != line.end())
    {
        if (!isspace(*trimmedBegin))
        {
            break;
        }

        ++trimmedBegin;
    }

    if (trimmedBegin == trimmedEnd.base())
    {
        return {trimmedBegin, trimmedBegin};
    }

    while (trimmedEnd != line.rend())
    {
        if (!isspace(*trimmedEnd))
        {
            break;
        }

        ++trimmedEnd;
    }

    return {trimmedBegin, trimmedEnd.base()};
}
