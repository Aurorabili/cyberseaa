#ifndef FILELOGGER_HPP_
#define FILELOGGER_HPP_

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "common/logger/Logger.hpp"
#include "common/utils/Utils.hpp"

class FileLogger : public Logger, public std::enable_shared_from_this<FileLogger> {
public:
    FileLogger(const std::string& filename)
    {
        openFile(filename);
    }

    void openFile(const std::string& filename)
    {
        m_file.open(filename, std::ofstream::out | std::ofstream::trunc);
        if (!m_file.is_open())
            std::cerr << "Can't open log file: '" << filename << "'" << std::endl;
    }

    FileLogger& operator<<(const std::string& str)
    {
        if (m_file.is_open())
            m_file << str;
        return *this;
    }

    FileLogger& operator<<(int i)
    {
        if (m_file.is_open())
            m_file << i;
        return *this;
    }

    FileLogger& operator<<(char c)
    {
        if (m_file.is_open())
            m_file << c;
        return *this;
    }

    FileLogger& operator<<(std::ostream& (*f)(std::ostream&))
    {
        if (m_file.is_open())
            f(m_file);
        return *this;
    }

    void print(const Log& log)
    {
        char levels[4] = { 'D', 'I', 'W', 'E' };
        *this << "[" << utils::getCurrentTime("%Y-%m-%d %H:%M:%S") << "] [" << levels[log.level()] << "] ";

        if (m_printFileAndLine)
            *this << log.file() << ":" << log.line() << ": ";

        if (!log.sourceName().empty())
            *this << "[" << log.sourceName() << "] ";

        *this << log.message() << std::endl;
    }

private:
    std::ofstream m_file;

    bool m_printFileAndLine = false;
};

#endif /* FILELOGGER_HPP_ */