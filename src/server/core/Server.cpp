#include "server/core/Server.hpp"

#include <ctime>
#include <limits>

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>

#include "common/Buffer.hpp"
#include "common/Constant.hpp"
#include "common/Debug.hpp"
#include "common/Time.hpp"
#include "common/utils/String.hpp"
#include "server/core/Message.hpp"
#include "server/core/Worker.hpp"
#include "server/core/services/Service.hpp"

namespace Core {

static inline u32 getWorkerId(u32 serviceid)
{
    return ((serviceid >> WORKER_ID_SHIFT) & 0xFF);
}

Server::~Server()
{
    wait();
}

void Server::init(u32 worker_num)
{
    m_now = Time::now();

    worker_num = (worker_num == 0) ? 1 : worker_num;

    logInfo("INIT with %d workers.", worker_num);

    for (u32 i = 0; i != worker_num; i++) {
        m_workers.emplace_back(std::make_unique<Worker>(this, i + 1));
        m_timers.emplace_back(std::make_unique<TimerType>());
    }

    for (auto& w : m_workers) {
        w->run();
    }

    m_state.store(State::Init, std::memory_order_release);
}

int Server::run()
{
    asio::io_context io_context;
    asio::steady_timer timer(io_context);
    asio::error_code _ec;
    bool stop_once = false;

    m_state.store(State::Running, std::memory_order_release);

    float startupTime = static_cast<float>(Time::now() - m_now) / 1000.0f;
    logInfo("Startup Done. (%.2f seconds)", startupTime);

    // mainloop
    while (true) {
        m_now = Time::now();

        if (m_ec < 0) {
            break;
        }

        if (m_ec != std::numeric_limits<int>::max() && !stop_once) {
            stop_once = true;
            logWarning("Received signal code %d", m_ec);
            for (auto w = m_workers.rbegin(); w != m_workers.rend(); w++) {
                (*w)->stop();
            }
        }

        if (m_state.load(std::memory_order_acquire) == State::Stopped) {
            size_t alive = m_workers.size();
            for (const auto& w : m_workers) {
                if (w->count() == 0) {
                    --alive;
                }
            }

            if (alive == 0) {
                break;
            }
        }

        for (auto& t : m_timers) {
            t->update(m_now);
        }
        timer.expires_after(std::chrono::milliseconds(1));
        timer.wait(_ec);
    }
    wait();
    if (m_ec == std::numeric_limits<int>::max()) {
        m_ec = 0;
    }
    return m_ec;
};

void Server::stop(int ec)
{
    m_ec = ec;
}

void Server::wait()
{
    for (auto iter = m_workers.rbegin(); iter != m_workers.rend(); iter++) {
        (*iter)->wait();
    }
    m_state.store(State::Stopped, std::memory_order_release);
}

State Server::state() const
{
    return m_state.load(std::memory_order_acquire);
}

void Server::setState(State st)
{
    m_state.store(st, std::memory_order_release);
}

std::time_t Server::now(bool sync)
{
    if (sync) {
        m_now = Time::now();
    }

    if (m_now == 0) {
        return Time::now();
    }
    return m_now;
}

u32 Server::serviceCount() const
{
    u32 res = 0;
    for (const auto& w : m_workers) {
        res += w->count();
    }
    return res;
}

Worker* Server::nextWorker()
{
    assert(m_workers.size() > 0);
    u32 min_count = std::numeric_limits<u32>::max();
    u32 min_count_workerid = 0;
    for (const auto& w : m_workers) {
        auto n = w->count();
        if (w->shared() && n < min_count) {
            min_count = n;
            min_count_workerid = w->id();
        }
    }

    if (min_count_workerid == 0) {
        min_count = std::numeric_limits<u32>::max();
        for (const auto& w : m_workers) {
            auto n = w->count();
            if (n < min_count) {
                min_count = n;
                min_count_workerid = w->id();
            }
        }
    }
    return m_workers[min_count_workerid - 1].get();
}

Worker* Server::getWorker(u32 wid, u32 sid) const
{
    wid = wid ? wid : getWorkerId(sid);
    if ((wid == 0 || wid > static_cast<u32>(m_workers.size()))) {
        return nullptr;
    }
    return m_workers[wid - 1].get();
}

void Server::timeout(int64_t interval, u32 serviceid, u32 timerid)
{
    auto workerid = getWorkerId(serviceid);
    assert(workerid > 0);
    if (interval <= 0) {
        onTimer(serviceid, timerid);
        return;
    }
    m_timers[workerid - 1]->add(m_now + interval, serviceid, timerid, this);
}

void Server::onTimer(u32 serviceid, u32 timerid)
{
    auto msg = Message::EmptyMessage();
    msg.setType(PTYPE_TIMER);
    msg.setSender(timerid);
    msg.setReceiver(serviceid);
    sendMessage(std::move(msg));
}

void Server::newService(std::unique_ptr<ServiceConfig> conf)
{
    Worker* w = getWorker(conf->threadid);
    if (nullptr != w) {
        w->shared(false);
    } else {
        w = nextWorker();
    }
    w->new_service(std::move(conf));
}

void Server::removeService(u32 serviceid, u32 sender, s32 sessionid)
{
    Worker* w = getWorker(0, serviceid);
    if (nullptr != w) {
        w->remove_service(serviceid, sender, sessionid);
    } else {
        auto content = utils::format("remove_service: invalid service id %u.", serviceid);
        response(sender, content, sessionid, PTYPE_ERROR);
    }
}

void Server::scanServices(u32 sender, u32 workerid, s32 sessionid) const
{
    auto* w = getWorker(workerid);
    if (nullptr == w) {
        return;
    }
    w->scan(sender, sessionid);
}

bool Server::sendMessage(Message&& m) const
{
    Worker* w = getWorker(0, m.receiver());
    if (nullptr == w) {
        logError("invalid message receiver id: %X", m.receiver());
        return false;
    }
    w->send(std::move(m));
    return true;
}

bool Server::send(u32 sender, u32 receiver, BufferPtr& data, s32 sessionid, uint8_t type) const
{
    sessionid = -sessionid;
    Message m = Message { std::move(data) };
    m.setSender(sender);
    m.setReceiver(receiver);
    m.setType(type);
    m.setSessionId(sessionid);
    return sendMessage(std::move(m));
}

void Server::broadcast(u32 sender, const BufferPtr& buf, u8 type) const
{
    for (auto& w : m_workers) {
        auto m = Message { std::make_unique<Buffer>(buf->clone()) };
        m.setBroadcast(true);
        m.setSender(sender);
        m.setType(type);
        w->send(std::move(m));
    }
}

bool Server::registerService(const std::string& type, ServiceRegisterFunction f)
{
    auto ret = m_registeredServices.emplace(type, f);
    DEBUG_ASSERT(ret.second, utils::format("already registed service type[%s].", type.data()).data());
    return ret.second;
}

ServicePtr Server::makeService(const std::string& type)
{
    auto iter = m_registeredServices.find(type);
    if (iter != m_registeredServices.end()) {
        return iter->second();
    }
    return nullptr;
}

std::shared_ptr<const std::string> Server::env(const std::string& name) const
{
    std::shared_ptr<const std::string> value;
    if (m_envs.try_get_value(name, value)) {
        return value;
    }
    return nullptr;
}

void Server::setEnv(std::string name, std::string value)
{
    m_envs.set(std::move(name), std::make_shared<const std::string>(std::move(value)));
}

u32 Server::uniqueService(const std::string& name) const
{
    if (name.empty()) {
        return 0;
    }
    u32 id = 0;
    m_uniqueServices.try_get_value(name, id);
    return id;
}

bool Server::setUniqueService(std::string name, u32 v)
{
    if (name.empty()) {
        return false;
    }
    return m_uniqueServices.try_set(std::move(name), v);
}

void Server::response(u32 to, std::string_view content, s32 sessionid, uint8_t mtype) const
{
    if (to == 0 || sessionid == 0) {
        if (state() == State::Running && mtype == PTYPE_ERROR && !content.empty()) {
            logDebug("server::response %s", std::string(content).data());
        }
        return;
    }

    auto m = Message { content.size() };
    m.setReceiver(to);
    m.setType(mtype);
    m.setSessionId(sessionid);
    m.writer(content);
    sendMessage(std::move(m));
}

std::string Server::info() const
{
    size_t timer_size = 0;
    for (const auto& timer : m_timers) {
        timer_size += timer->size();
    }

    std::string req;
    req.append("[\n");
    req.append(utils::format(R"({"id":0, "socket":%zu, "timer":%zu, "service":%u })",
        socketNum(),
        timer_size,
        serviceCount()));
    for (const auto& w : m_workers) {
        req.append(",\n");
        auto v = utils::format(R"({"id":%u, "mqsize":%u, "service":%u, "timer":%zu, "alive":%u})",
            w->id(),
            w->mq_size(),
            w->count(),
            m_timers[w->id() - 1]->size(),
            w->alive());
        req.append(v);
    }
    req.append("]");
    return req;
}

u32 Server::nextFd()
{
    u32 fd = 0;
    do {
        fd = m_fdSeq.fetch_add(1);
    } while (fd == 0 || !tryLockFd(fd));
    return fd;
}

bool Server::tryLockFd(u32 fd)
{
    std::unique_lock lck(m_fdLock);
    return m_fdWatcher.emplace(fd).second;
}

void Server::unlockFd(u32 fd)
{
    std::unique_lock lck(m_fdLock);
    size_t count = m_fdWatcher.erase(fd);
}

size_t Server::socketNum() const
{
    std::unique_lock lck(m_fdLock);
    return m_fdWatcher.size();
}
};