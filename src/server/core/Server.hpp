#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <atomic>
#include <ctime>
#include <memory>
#include <string>
#include <unordered_set>

#include "common/Buffer.hpp"
#include "common/ConcurrentMap.hpp"
#include "common/Constant.hpp"
#include "common/RWSpinLock.hpp"
#include "common/Timer.hpp"
#include "common/Types.hpp"
#include "common/utils/IntTypes.hpp"
#include "server/core/services/Service.hpp"

namespace Core {

class Server final {
private:
    class TimerExpirePolicy {
    public:
        TimerExpirePolicy() = default;

        TimerExpirePolicy(u32 serviceid, u32 timerid, Server* srv)
            : m_serviceId(serviceid)
            , m_timerId(timerid)
            , m_server(srv)
        {
        }

        void operator()()
        {
            m_server->onTimer(m_serviceId, m_timerId);
        }

        u32 id() const
        {
            return m_timerId;
        }

    private:
        u32 m_serviceId = 0;
        u32 m_timerId = 0;
        Server* m_server = nullptr;
    };

public:
    using ServiceRegisterFunction = ServicePtr (*)();

    using TimerType = BaseTimer<TimerExpirePolicy>;

public:
    Server() = default;

    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(const Server&&) = delete;
    Server& operator=(const Server&&) = delete;

public:
    void init(u32 worker_num);

    int run();

    void stop(int ec);

    State state() const;

    void setState(enum State state);

    std::time_t now(bool sync = false);

    u32 serviceCount() const;

    Worker* nextWorker();

    Worker* getWorker(u32 workerId, u32 serviceId = 0) const;

    void timeout(s64 interval, u32 serviceId, u32 timerId);

    void newService(std::unique_ptr<ServiceConfig> conf);

    void removeService(u32 serviceId, u32 sender, s32 sessionId);

    void scanServices(u32 sender, u32 workerId, s32 sessionId) const;

    bool sendMessage(Message&& msg) const;

    bool send(u32 sender, u32 receiver, BufferPtr& buf, s32 sessionId, u8 type) const;

    void broadcast(u32 sender, const BufferPtr& buf, u8 type) const;

    bool registerService(const std::string& type, ServiceRegisterFunction func);

    ServicePtr makeService(const std::string& type);

    std::shared_ptr<const std::string> env(const std::string& name) const;

    void setEnv(std::string name, std::string value);

    u32 uniqueService(const std::string& name) const;

    bool setUniqueService(std::string name, u32 v);

    // Used to respond to calls from the user layer to the framework layer
    void response(u32 to, std::string_view content, int32_t sessionid, uint8_t mtype = PTYPE_TEXT) const;

    std::string info() const;

    u32 nextFd();

    bool tryLockFd(u32 fd);

    void unlockFd(u32 fd);

    size_t socketNum() const;

private:
    void onTimer(u32 serviceid, u32 timerid);

    void wait();

private:
    volatile int m_ec = std::numeric_limits<int>::max();
    std::atomic<State> m_state = State::Unkown;
    std::atomic<u32> m_fdSeq = 1;
    std::time_t m_now = 0;
    mutable std::mutex m_fdLock;
    std::unordered_map<std::string, ServiceRegisterFunction> m_registeredServices;
    concurrent_map<std::string, std::shared_ptr<const std::string>, RWSpinLock> m_envs;
    concurrent_map<std::string, u32, RWSpinLock> m_uniqueServices;
    std::unordered_set<u32> m_fdWatcher;
    std::vector<std::unique_ptr<TimerType>> m_timers;
    std::vector<std::unique_ptr<Worker>> m_workers;
};
}; // namespace Cyberseea

#endif /* SERVER_HPP_ */