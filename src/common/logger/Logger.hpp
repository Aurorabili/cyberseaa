#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <sstream>
#include <string>

#include "common/logger/LoggerUtils.hpp"
#include "common/utils/IntTypes.hpp"

enum LogLevel : u8 {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,

    None = 4
};

class LogStream;

class Logger {

public:
    Logger(LogLevel level = LogLevel::Debug, const char* file = nullptr, int line = -1,
        const std::string& sourceName = "")
        : m_level(level)
        , m_file(file)
        , m_line(line)
        , m_sourceName(sourceName)
        , m_color([&level]() -> LoggerColor {
            if (level == LogLevel::Debug)
                return LoggerColor::Cyan;
            else if (level == LogLevel::Warning)
                return LoggerColor::Yellow;
            else if (level == LogLevel::Error)
                return LoggerColor::Red;
            else
                return LoggerColor::White;
        }())
    {
    }

    ~Logger()
    {
        print();
    }

    void setColor(LoggerColor color)
    {
        m_color = color;
    }
    void setBold(bool isBold)
    {
        m_isBold = isBold;
    }

    void addSpace()
    {
        if (!m_stream.str().empty())
            m_stream << " ";
    }

    template <typename T>
    Logger& operator<<(const T& object)
    {
        addSpace();
        m_stream << object;
        return *this;
    }
    Logger& operator<<(const char* str)
    {
        addSpace();
        m_stream << str;
        return *this;
    }
    Logger& operator<<(const std::string& str)
    {
        addSpace();
        m_stream << "\"" << str << "\"";
        return *this;
    }

    static std::string textColor(LoggerColor color = LoggerColor::White, bool bold = false);

    static bool isEnabled;
    static bool printFileAndLine;
    static bool printWithColor;

private:
    void print();

    LogLevel m_level;

    const char* m_file = nullptr;
    int m_line = -1;

    std::string m_sourceName;

    LoggerColor m_color = LoggerColor::White;
    bool m_isBold = false;

    std::stringstream m_stream;
};

#endif /* LOGGER_HPP_ */