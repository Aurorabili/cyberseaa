#ifndef LOGGERUTILS_HPP_
#define LOGGERUTILS_HPP_

#include <ostream>

#include "common/utils/IntTypes.hpp"

enum class LoggerColor : u8 {
    White = 0,
    Red = 31,
    Cyan = 36,
    Yellow = 33,
    Green = 32,
};

//======================================================================================
// LoggerColor
//======================================================================================
std::ostream& operator<<(std::ostream& stream, LoggerColor color);

//======================================================================================
// Color
//======================================================================================
#include "common/utils/Color.hpp"

std::ostream& operator<<(std::ostream& stream, const Color& color);

#endif /* LOGGERUTILS_HPP_ */
