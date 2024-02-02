#include "common/logger/LoggerHandler.hpp"
#include "common/utils/Exception.hpp"

LoggerHandler::InstanceMap LoggerHandler::s_instanceMap;

Logger LoggerHandler::print(LogLevel level, const char* file, int line)
{
    return { m_stream, level >= m_maxLevel ? level : LogLevel::None, file, line, m_name };
}

LoggerHandler& LoggerHandler::getInstance()
{
    if (s_instanceMap.empty())
        throw EXCEPTION("LoggerHandler is not initialized");

    return *s_instanceMap.at(std::this_thread::get_id());
}

void LoggerHandler::setInstance(LoggerHandler& instance)
{
    s_instanceMap.emplace(std::this_thread::get_id(), &instance);
}
