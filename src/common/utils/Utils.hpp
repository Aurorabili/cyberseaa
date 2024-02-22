#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <sstream>
#include <vector>

#include "common/utils/IntTypes.hpp"

namespace utils {

template <typename T>
std::string toString(const T value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << value;
    return out.str();
}

bool regexMatch(const std::string& str, const std::string& regex);

template <typename... Args>
std::string makeString(Args&&... args)
{
    std::ostringstream stream;
    std::vector<int> tmp { 0, ((void)(stream << args << " "), 0)... };

    return stream.str();
}

std::string getCurrentTime(const char* format);

template <typename T>
time_t toTimeT(const u64 timestamp);

template <typename T>
u64 toTimeStamp(const time_t time);
} // namespace utils

#endif // UTILS_HPP_
