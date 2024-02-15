#ifndef LOG_HPP_
#define LOG_HPP_

#include <string>

#include "common/logger/LoggerUtils.hpp"

class Log {

public:
    Log(LogLevel level = LogLevel::Debug, const char* file = nullptr, int line = -1,
        const std::string& sourceName = "", const std::string& message = "")
        : m_level(level)
        , m_file(file)
        , m_line(line)
        , m_sourceName(sourceName)
        , m_message(message)
    {
    }

    Log(const Log& other)
    {
        m_level = other.m_level;
        m_file = other.m_file;
        m_line = other.m_line;
        m_sourceName = other.m_sourceName;
        m_message = other.m_message;
    }

    LogLevel level() const
    {
        return m_level;
    }
    std::string sourceName() const
    {
        return m_sourceName;
    }
    std::string file() const
    {
        return m_file;
    }
    int line() const
    {
        return m_line;
    }
    std::string message() const
    {
        return m_message;
    }

private:
    LogLevel m_level;
    const char* m_file = nullptr;
    int m_line = -1;
    std::string m_sourceName;
    std::string m_message;
};

#endif /* LOG_HPP_ */