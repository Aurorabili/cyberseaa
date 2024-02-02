#ifndef LOGGERHANDLER_HPP_
#define LOGGERHANDLER_HPP_

#include <thread>
#include <unordered_map>

#include "common/logger/LogStream.hpp"
#include "common/logger/Logger.hpp"

class LoggerHandler
{
    using InstanceMap = std::unordered_map<std::thread::id, LoggerHandler *>;

  public:
    LoggerHandler() = default;

    Logger print(LogLevel level, const char *file, int line);

    LogLevel maxLevel() const
    {
        return m_maxLevel;
    }
    void setMaxLevel(LogLevel maxLevel)
    {
        m_maxLevel = maxLevel;
    }

    void setName(const std::string &name)
    {
        m_name = name;
    }

    void openLogFile(const std::string &filename)
    {
        m_stream.openFile(filename);
    }

    static LoggerHandler &getInstance();
    static void setInstance(LoggerHandler &instance);

  private:
    static InstanceMap s_instanceMap;

    std::string m_name;

    LogLevel m_maxLevel = LogLevel::Debug;

    LogStream m_stream;
};

#endif // LOGGERHANDLER_HPP_
