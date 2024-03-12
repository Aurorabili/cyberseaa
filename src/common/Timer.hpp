#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>

namespace Common {
template <typename ExpirePolicy>
class BaseTimer {
    using ExpirePolicyType = ExpirePolicy;

public:
    BaseTimer() = default;

    BaseTimer(const BaseTimer&) = delete;
    BaseTimer& operator=(const BaseTimer&) = delete;

    void update(int64_t now)
    {
        if (m_stop) {
            return;
        }

        do {
            std::unique_lock lock { m_lock };
            if (auto iter = m_timers.begin(); iter == m_timers.end() || iter->first > now) {
                break;
            } else {
                auto nh = m_timers.extract(iter);
                lock.unlock();
                nh.mapped()();
            }
        } while (true);
        return;
    }

    void pause()
    {
        m_stop = true;
    }

    void resume()
    {
        m_stop = false;
    }

    template <typename... Args>
    void add(time_t expiretime, Args&&... args)
    {
        std::lock_guard lock { m_lock };
        m_timers.emplace(expiretime, ExpirePolicyType { std::forward<Args>(args)... });
    }

    size_t size() const
    {
        return m_timers.size();
    }

private:
    bool m_stop = false;
    std::mutex m_lock;
    std::multimap<int64_t, ExpirePolicyType> m_timers;
};

class DefaultExpirePolicy {
public:
    using HandlerType = std::function<void()>;

    DefaultExpirePolicy(uint32_t, HandlerType handler)
        : m_handler(std::move(handler))
    {
    }

    void operator()()
    {
        m_handler();
    }

private:
    HandlerType m_handler;
};

using Timer = BaseTimer<DefaultExpirePolicy>;
}

#endif // TIMER_HPP_