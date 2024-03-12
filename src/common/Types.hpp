#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "common/utils/IntTypes.hpp"

namespace Common {

template <typename T>
std::string toString(const T value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << value;
    return out.str();
}

inline bool regexMatch(const std::string& str, const std::string& regex)
{
    std::regex re(regex);
    std::cmatch m;

    return std::regex_match(str.c_str(), m, re);
}

template <typename... Args>
std::string makeString(Args&&... args)
{
    std::ostringstream stream;
    std::vector<int> tmp { 0, ((void)(stream << args << " "), 0)... };

    return stream.str();
}

inline std::string getCurrentTime(const char* format)
{
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::stringstream sstream;
    sstream << std::put_time(&tm, format);

    return sstream.str();
}

template <typename Enum>
struct enum_enable_bitmask_operators {
    static constexpr bool enable = false;
};

template <typename Enum>
inline typename std::enable_if_t<enum_enable_bitmask_operators<Enum>::enable, Enum> operator|(Enum a, Enum b)
{
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(a) | static_cast<std::underlying_type_t<Enum>>(b));
}

template <typename Enum>
inline typename std::enable_if_t<enum_enable_bitmask_operators<Enum>::enable, Enum> operator&(Enum a, Enum b)
{
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(a) & static_cast<std::underlying_type_t<Enum>>(b));
}

template <typename Enum, typename std::enable_if_t<enum_enable_bitmask_operators<Enum>::enable, int> = 0>
inline bool enum_has_any_bitmask(Enum v, Enum contains)
{
    using under_type = typename std::underlying_type<Enum>::type;
    return (static_cast<under_type>(v) & static_cast<under_type>(contains)) != 0;
}

enum class State {
    Unkown,
    Init,
    Running,
    Stopping,
    Stopped
};

enum class SocketDataTypeEnum : u8 {
    SocketConnect = 1,
    SocketAccept = 2,
    SocketRecv = 3,
    SocketClose = 4,
    SocketPing = 5,
    SocketPong = 6,
};

enum class EnableChunkedEnum : u8 {
    NONE = 0,
    SEND = 1 << 0,
    RECEIVE = 1 << 1,
    BOTH = 3,
};

template <>
struct enum_enable_bitmask_operators<EnableChunkedEnum> {
    static constexpr bool enable = true;
};

}

#endif // TYPES_HPP_