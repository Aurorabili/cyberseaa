#include "common/logger/LoggerHandler.hpp"

void LoggerHandler::run()
{
    while (m_isRunning) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this]() { return !m_queue.empty() || !m_isRunning; });

        while (!m_queue.empty()) {
            auto log = m_queue.front();
            m_queue.pop();
            for (auto& stream : m_loggers) {
                stream->print(log);
            }
        }
    }
}

LogStream LoggerHandler::print(LogLevel level, const char* file, int line)
{
    return { level >= m_maxLevel ? level : LogLevel::None, file, line, m_name };
}

void LoggerHandler::post(Log&& log)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::move(log));
    m_cv.notify_one();
}