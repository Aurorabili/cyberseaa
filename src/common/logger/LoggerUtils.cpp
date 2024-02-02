#include "common/logger/LoggerUtils.hpp"
#include "common/logger/Logger.hpp"

std::ostream &operator<<(std::ostream &stream, LoggerColor color)
{
    return stream << Logger::textColor(color);
}

std::ostream &operator<<(std::ostream &stream, const Color &color)
{
    return stream << "Color(" << color.r * 255 << ", " << color.g * 255 << ", " << color.b * 255 << ", "
                  << color.a * 255 << ")";
}
