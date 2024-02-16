#ifndef LOGGERUTILS_HPP_
#define LOGGERUTILS_HPP_

// #include <ostream>
#include <string>

#include "common/utils/IntTypes.hpp"

enum LogLevel : u8 {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,

    None = 4
};

enum class LoggerColor : u8 {
    White = 7,
    Red = 1,
    Cyan = 6,
    Yellow = 3,
    Green = 2,
};

class LoggerUtils final {
public:
    static std::string textColor(LoggerColor foregroundColor, LoggerColor backgroundColor, bool bold)
    {
        return std::string("\33[") + ((bold) ? "1" : "0") + ";3" + std::to_string(u8(foregroundColor))
            + ";4" + std::to_string(u8(backgroundColor)) + "m";
    }

    static std::string textColorReset()
    {
        return "\33[0m";
    }
};

//======================================================================================
// LoggerColor
//======================================================================================
// std::ostream& operator<<(std::ostream& stream, LoggerColor color);

//======================================================================================
// Color
//======================================================================================
// #include "common/utils/Color.hpp"

// std::ostream& operator<<(std::ostream& stream, const Color& color);

#endif /* LOGGERUTILS_HPP_ */
