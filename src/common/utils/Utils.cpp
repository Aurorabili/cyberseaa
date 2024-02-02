#include <iomanip>
#include <regex>
#include <sstream>

#include "common/utils/Utils.hpp"

namespace utils
{

bool regexMatch(const std::string &str, const std::string &regex)
{
    std::regex re(regex);
    std::cmatch m;

    return std::regex_match(str.c_str(), m, re);
}

std::string getCurrentTime(const char *format)
{
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::stringstream sstream;
    sstream << std::put_time(&tm, format);

    return sstream.str();
}

} // namespace utils
