#include "common/logger/Logger.hpp"
#include "common/logger/LogStream.hpp"
#include "common/utils/Utils.hpp"

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
    if (!isEnabled || m_level == LogLevel::None)
        return;

    m_outStream << textColor(m_color, m_isBold);

    char levels[4] = { 'D', 'I', 'W', 'E' };
    m_outStream << "[" + utils::getCurrentTime("%H:%M:%S") + "] [" << levels[m_level] << "] ";

    if (printFileAndLine)
        m_outStream << m_file << ":" << m_line << ": ";

    if (!m_sourceName.empty())
        m_outStream << "[" + m_sourceName + "] ";

    m_outStream << m_stream.str() << std::endl;
}
