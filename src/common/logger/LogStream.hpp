#ifndef LOGSTREAM_HPP_
#define LOGSTREAM_HPP_

#include <sstream>
#include <string>

#include "common/logger/LoggerUtils.hpp"

class LogStream {
public:
    LogStream(LogLevel level = LogLevel::Debug, const char* file = nullptr, int line = -1,
        const std::string& sourceName = "")
        : m_level(level)
        , m_file(file)
        , m_line(line)
        , m_sourceName(sourceName)
    {
    }

    ~LogStream();

    void addSpace()
    {
        if (!m_stream.str().empty())
            m_stream << " ";
    }

    template <typename T>
    LogStream& operator<<(const T& object)
    {
        addSpace();
        m_stream << object;
        return *this;
    }
    LogStream& operator<<(const char* str)
    {
        addSpace();
        m_stream << str;
        return *this;
    }
    LogStream& operator<<(const std::string& str)
    {
        addSpace();
        m_stream << "\"" << str << "\"";
        return *this;
    }

    static bool isEnabled;

private:
    LogLevel m_level;

    const char* m_file = nullptr;
    int m_line = -1;

    std::string m_sourceName;

    std::stringstream m_stream;
};

#endif /* LOGSTREAM_HPP_ */