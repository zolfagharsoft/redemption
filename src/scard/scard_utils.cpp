#include <iomanip>
#include <sstream>
#include <string>

#include "utils/sugar/bytes_view.hpp"

#include "scard/scard_utils.hpp"


///////////////////////////////////////////////////////////////////////////////



void check_predicate(bool predicate, const char */*file*/, const char *function,
    int line, const char *name, error_type eid)
{
    if (!predicate)
    {
        LOG(LOG_ERR, "%s(%d): unverified predicate '%s'",
            function, line, name);

        throw Error(eid);
    }
}

std::string bytes_to_hex_string(bytes_view data,
    std::string_view byte_separator)
{
    std::stringstream stream;

    stream << std::hex;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        stream
            << std::setw(2) << std::setfill('0')
            << static_cast<int>(*it);

        if (std::next(it) != data.end())
        {
            stream
                << byte_separator;
        }
    }

    return stream.str();
}

std::string trim(const std::string& string)
{
    const std::size_t first = string.find_first_not_of(' ');

    // return the original string in case it does not contain any space
    if (first == std::string::npos)
    {
        return string;
    }

    const std::size_t last = string.find_last_not_of(' ');

    return string.substr(first, (last - first + 1));
}