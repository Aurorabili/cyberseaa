#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_

#include <mutex>
#include <thread>

#include <asio/io_context.hpp>

#include "common/utils/IntTypes.hpp"

class ServerApplication;

class ThreadPool final {
    friend class ServerApplication;

public:
    ThreadPool(u16 numThreads = std::thread::hardware_concurrency())
        : m_numThreads(numThreads)
    {
        m_threads.reserve(m_numThreads);
    }
    ~ThreadPool()
    {
        m_ioContext.stop();

        for (auto& thread : m_threads) {
            thread.join();
        }
    }

    void post(std::function<void()>&& task)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_ioContext.post(std::move(task));
    }

    asio::io_context& getIoContext() { return m_ioContext; }

private:
    void run()
    {
        for (u16 i = 0; i < m_numThreads; ++i) {
            m_threads.emplace_back([this]() {
                m_ioContext.run();
            });
        }

        m_isRunning = true;
    }

    void stop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_isRunning) {
            return;
        }

        m_ioContext.stop();
        m_work.~work();

        for (auto& thread : m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        m_isRunning = false;
        m_threads.clear();
        m_ioContext.reset();
    }

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

private:
    u16 m_numThreads = std::thread::hardware_concurrency();
    bool m_isRunning = false;
    asio::io_context m_ioContext;
    asio::io_context::work m_work { m_ioContext };
    std::vector<std::thread> m_threads;

private:
    std::mutex m_mutex;
};

#endif /* THREADPOOL_HPP_ */