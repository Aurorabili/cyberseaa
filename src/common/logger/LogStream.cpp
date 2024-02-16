#include "common/logger/LogStream.hpp"

#include "common/logger/Log.hpp"
#include "common/logger/LoggerHandler.hpp"

bool LogStream::isEnabled = true;

LogStream::~LogStream()
{
    if (!isEnabled || m_level == LogLevel::None)
        return;

    LoggerHandler::getInstance().post(Log(m_level, m_file, m_line, m_sourceName, m_stream.str()));
}
