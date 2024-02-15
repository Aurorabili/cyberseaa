#include "common/logger/LoggerHandler.hpp"
#include "common/utils/Exception.hpp"

void LoggerHandler::run()
{
    while (m_isRunning) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this]() { return !m_queue.empty() || !m_isRunning; });

        while (!m_queue.empty()) {
            m_stream << m_queue.front() << std::endl;
            m_queue.pop();
        }
    }
}

Logger LoggerHandler::print(LogLevel level, const char* file, int line)
{
    return { level >= m_maxLevel ? level : LogLevel::None, file, line, m_name };
}

void LoggerHandler::post(const std::string& stream)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(stream);
    m_cv.notify_one();
}