#ifndef LOGGERHANDLER_HPP_
#define LOGGERHANDLER_HPP_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "common/logger/LogStream.hpp"
#include "common/logger/Logger.hpp"

class LoggerHandler {

public:
    ~LoggerHandler()
    {
        m_isRunning = false;
        m_cv.notify_one();
        m_thread.join();
    }

    Logger print(LogLevel level, const char* file, int line);

    void post(const std::string& stream);

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
        m_stream.openFile(filename);
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
    LogStream m_stream;

private:
    void run();

    bool m_isRunning;
    std::mutex m_mutex;
    std::thread m_thread;
    std::queue<std::string> m_queue;
    std::condition_variable m_cv;
};

#endif // LOGGERHANDLER_HPP_
