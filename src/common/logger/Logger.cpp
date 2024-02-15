#include "common/logger/Logger.hpp"
#include "common/logger/LoggerHandler.hpp"
#include "common/utils/Utils.hpp"
#include <string>

bool Logger::isEnabled = true;
bool Logger::printFileAndLine = false;
bool Logger::printWithColor = true;

std::string Logger::textColor(LoggerColor color, bool bold)
{
    return (!printWithColor) ? ""
                             : std::string("\33[") + ((bold) ? "1" : "0") + ";" + std::to_string(u8(color)) + "m";
}

void Logger::print()
{
    std::string _outstr;

    if (!isEnabled || m_level == LogLevel::None)
        return;

    _outstr += textColor(m_color, m_isBold);

    char levels[4] = { 'D', 'I', 'W', 'E' };
    _outstr += "[" + utils::getCurrentTime("%H:%M:%S") + "] [" + levels[m_level] + "] ";

    if (printFileAndLine)
        _outstr += std::string(m_file) + ":" + std::to_string(m_line) + ": ";

    if (!m_sourceName.empty())
        _outstr += "[" + m_sourceName + "] ";

    _outstr += m_stream.str();

    LoggerHandler::getInstance().post(_outstr);
}
