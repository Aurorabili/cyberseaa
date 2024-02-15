#ifndef LOGGERHANDLER_HPP_
#define LOGGERHANDLER_HPP_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "common/logger/ConsoleLogger.hpp"
#include "common/logger/FileLogger.hpp"
#include "common/logger/Log.hpp"
#include "common/logger/LogStream.hpp"
#include "common/logger/Logger.hpp"

class LogStream;

class LoggerHandler {
public:
    ~LoggerHandler()
    {
        m_isRunning = false;
        m_cv.notify_one();
        m_thread.join();
    }

    LogStream print(LogLevel level, const char* file, int line);

    void post(Log&& log);

    LogLevel maxLevel() const
    {
        return m_maxLevel;
    }

    static LoggerHandler& getInstance()
    {
        static LoggerHandler instance;
        return instance;
    }

    void init(LogLevel maxLevel, const std::string& name, const std::string& filename)
    {
        m_maxLevel = maxLevel;
        m_name = name;

        m_loggers.push_back(std::make_unique<FileLogger>(filename));
        m_loggers.push_back(std::make_unique<ConsoleLogger>());

        m_isRunning = true;
        m_thread = std::thread(&LoggerHandler::run, this);
    }

private:
    LoggerHandler() = default;

    LoggerHandler(const LoggerHandler&) = delete;
    LoggerHandler& operator=(const LoggerHandler&) = delete;

private:
    std::string m_name;
    LogLevel m_maxLevel;
    std::vector<std::unique_ptr<Logger>> m_loggers;

private:
    void run();

    bool m_isRunning;
    std::mutex m_mutex;
    std::thread m_thread;
    std::queue<Log> m_queue;
    std::condition_variable m_cv;
};

#endif // LOGGERHANDLER_HPP_
