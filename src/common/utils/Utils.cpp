#include <chrono>
#include <iomanip>
#include <regex>
#include <sstream>
#include <type_traits>

#include "common/utils/IntTypes.hpp"
#include "common/utils/Utils.hpp"

namespace utils {

bool regexMatch(const std::string& str, const std::string& regex)
{
    std::regex re(regex);
    std::cmatch m;

    return std::regex_match(str.c_str(), m, re);
}

std::string getCurrentTime(const char* format)
{
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::stringstream sstream;
    sstream << std::put_time(&tm, format);

    return sstream.str();
}

template <typename T>
u64 toTimeStamp(const std::time_t time)
{
    if constexpr (std::is_same_v<T, std::chrono::seconds> || std::is_same_v<T, std::chrono::milliseconds>) {
        return -1;
    }
    auto tp = std::chrono::system_clock::from_time_t(time);
    auto timestamp = std::chrono::duration_cast<T>(tp.time_since_epoch()).count();
    return timestamp;
}

template <typename T>
std::time_t toTimeT(const u64 timestamp)
{
    if constexpr (std::is_same_v<T, std::chrono::seconds> || std::is_same_v<T, std::chrono::milliseconds>) {
        return -1;
    }
    auto tp = std::chrono::system_clock::time_point(T(timestamp));
    auto time = std::chrono::system_clock::to_time_t(tp);
    return time;
}
} // namespace utils