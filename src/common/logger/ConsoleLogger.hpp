#ifndef CONSOLELOGGER_HPP_
#define CONSOLELOGGER_HPP_

#include <iostream>
#include <memory>

#include "common/logger/Log.hpp"
#include "common/logger/Logger.hpp"
#include "common/logger/LoggerUtils.hpp"
#include "common/utils/Utils.hpp"

class ConsoleLogger : public Logger, public std::enable_shared_from_this<ConsoleLogger> {
public:
    ConsoleLogger() = default;

    ConsoleLogger& operator<<(const std::string& str)
    {
        std::cout << str;
        return *this;
    }

    ConsoleLogger& operator<<(int i)
    {
        std::cout << i;
        return *this;
    }

    ConsoleLogger& operator<<(char c)
    {
        std::cout << c;
        return *this;
    }

    ConsoleLogger& operator<<(std::ostream& (*f)(std::ostream&))
    {
        f(std::cout);
        return *this;
    }

    void print(const Log& log)
    {
        auto _color = [&]() -> LoggerColor {
            switch (log.level()) {
            case LogLevel::Debug:
                return LoggerColor::Green;
            case LogLevel::Info:
                return LoggerColor::White;
            case LogLevel::Warning:
                return LoggerColor::Yellow;
            case LogLevel::Error:
                return LoggerColor::Red;
            default:
                return LoggerColor::White;
            }
        }();

        if (m_printWithColor)
            *this << LoggerUtils::textColor(LoggerColor::White, _color, false);

        char levels[4] = { 'D', 'I', 'W', 'E' };
        *this << "[" << utils::getCurrentTime("%Y-%m-%d %H:%M:%S") << "] [" << levels[log.level()] << "] ";

        if (m_printFileAndLine)
            *this << log.file() << ":" << log.line() << ": ";

        if (!log.sourceName().empty())
            *this << "[" << log.sourceName() << "]";

        if (m_printWithColor)
            *this << LoggerUtils::textColorReset();

        *this << " " << log.message() << std::endl;
    }

private:
    bool m_printFileAndLine = false;
    bool m_printWithColor = true;
};

#endif /* CONSOLELOGGER_HPP_ */