#include "server/core/Worker.hpp"

#include <string_view>

#include "common/Debug.hpp"
#include "common/Time.hpp"
#include "common/utils/String.hpp"
#include "server/core/Message.hpp"
#include "server/core/Server.hpp"
#include "server/core/Service.hpp"

using namespace std::literals::string_view_literals;

namespace Core {
Worker::Worker(Server* srv, u32 id)
    : m_workerid(id)
    , m_server(srv)
    , m_ioc(1)
    , m_work(asio::make_work_guard(m_ioc))
{
}

Worker::~Worker()
{
    wait();
}

u32 Worker::alive()
{
    auto n = m_version;
    asio::post(m_ioc, [this]() {
        ++m_version;
    });
    return n;
}

void Worker::run()
{
    m_socketServer = std::make_unique<SocketServer>(m_server, this, m_ioc);

    m_thread = std::thread([this]() {
        logInfo("WORKER-%u START", m_workerid);
        m_ioc.run();
        m_socketServer->closeAllConnections();
        m_services.clear();
        logInfo("WORKER-%u STOP", m_workerid);
    });
}

void Worker::stop()
{
    asio::post(m_ioc, [this] {
        Message msg = Message::EmptyMessage();
        msg.setType(PTYPE_SHUTDOWN);
        for (auto& it : m_services) {
            it.second->dispatch(&msg);
        }
    });
}

void Worker::wait()
{
    m_ioc.stop();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Worker::new_service(std::unique_ptr<ServiceConfig> conf)
{
    m_count.fetch_add(1, std::memory_order_release);
    asio::post(m_ioc, [this, conf = std::move(conf)]() {
        do {
            size_t counter = 0;
            u32 serviceid = 0;
            do {
                if (counter >= WORKER_MAX_SERVICE) {
                    serviceid = 0;
                    logError("new service failed: can not get more service id. Worker[%u] service num[%zu].", id(), m_services.size());
                    break;
                }

                ++m_nextid;
                if (m_nextid == WORKER_MAX_SERVICE) {
                    m_nextid = 1;
                }
                serviceid = m_nextid | (id() << WORKER_ID_SHIFT);
                ++counter;
            } while (m_services.find(serviceid) != m_services.end());

            if (serviceid == 0) {
                break;
            }

            auto s = m_server->makeService(conf->type);
            DEBUG_ASSERT(s, utils::format("new service failed:service type[%s] was not registered", conf->type.data()).data());
            s->setId(serviceid);
            s->setUnique(conf->unique);
            s->setContext(m_server, this);

            if (!s->init(*conf)) {
                if (serviceid == BOOTSTRAP_ADDR) {
                    m_server->stop(-1);
                }
                break;
            }
            s->setReady();
            m_services.emplace(serviceid, std::move(s));

            logDebug("new service[%08X] type[%s] name[%s] threadid[%u] ready.", serviceid, conf->type.data(), conf->name.data(), conf->threadid);

            if (0 != conf->session) {
                m_server->response(conf->creator, std::to_string(serviceid), conf->session, PTYPE_INTEGER);
            }
            return;
        } while (false);

        m_count.fetch_sub(1, std::memory_order_release);
        if (m_services.empty()) {
            shared(true);
        }

        if (0 != conf->session) {
            m_server->response(conf->creator, "0"sv, conf->session, PTYPE_INTEGER);
        }
    });
}

void Worker::remove_service(u32 serviceid, u32 sender, u32 sessionid)
{
    asio::post(m_ioc, [this, serviceid, sender, sessionid]() {
        if (auto s = find_service(serviceid); nullptr != s) {
            auto name = s->name();
            auto id = s->id();
            m_count.fetch_sub(1, std::memory_order_release);
            m_server->response(sender, "service destroy"sv, sessionid);
            m_services.erase(serviceid);
            if (m_services.empty())
                shared(true);

            if (m_server->state() == State::Running) {
                auto content = utils::format("_service_exit,name:%s serviceid:%08X", name.data(), id);
                auto buf = Buffer::make_unique();
                buf->write_back(content.data(), content.size());
                m_server->broadcast(serviceid, buf, PTYPE_SYSTEM);
            }

            if (serviceid == BOOTSTRAP_ADDR) {
                m_server->setState(State::Stopped);
            }
        } else {
            m_server->response(sender, utils::format("service [%08X] not found", serviceid), sessionid, PTYPE_ERROR);
        }
    });
}

void Worker::scan(u32 sender, int32_t sessionid)
{
    asio::post(m_ioc, [this, sender, sessionid] {
        std::string content;
        for (auto& it : m_services) {
            if (content.empty())
                content.append("[");

            content.append(utils::format(
                R"({"name":"%s","serviceid":"%X"},)",
                it.second->name().data(),
                it.second->id()));
        }

        if (!content.empty())
            content.back() = ']';
        m_server->response(sender, content, sessionid);
    });
}

asio::io_context& Worker::io_context()
{
    return m_ioc;
}

void Worker::send(Message&& msg)
{
    m_mqsize.fetch_add(1, std::memory_order_relaxed);
    if (m_queue.push_back(std::move(msg)) == 1) {
        asio::post(m_ioc, [this]() {
            if (auto& read_queue = m_queue.swap_on_read(); !read_queue.empty()) {
                Service* s = nullptr;
                for (auto& m : read_queue) {
                    handle_one(s, std::move(m));
                    --m_mqsize;
                }
                read_queue.clear();
            }
        });
    }
}

u32 Worker::id() const
{
    return m_workerid;
}

Service* Worker::find_service(u32 serviceid) const
{
    auto iter = m_services.find(serviceid);
    if (m_services.end() != iter) {
        return iter->second.get();
    }
    return nullptr;
}

void Worker::shared(bool v)
{
    m_shared = v;
}

bool Worker::shared() const
{
    return m_shared.load();
}

Service* Worker::handle_one(Service* s, Message&& msg)
{
    u32 sender = msg.sender();
    u32 receiver = msg.receiver();
    uint8_t type = msg.type();

    if (msg.broadcast()) {
        for (auto& it : m_services) {
            if (!it.second->unique() && type == PTYPE_SYSTEM) {
                continue;
            }

            if (it.second->ready() && it.second->id() != sender) {
                handle_message(it.second, msg);
            }
        }
        return nullptr;
    }

    if (nullptr == s || s->id() != receiver) {
        s = find_service(receiver);
        if (nullptr == s || !s->ready()) {
            if (sender != 0 && msg.type() != PTYPE_TIMER) {
                std::string hexdata = utils::hex_string({ msg.data(), msg.size() });
                std::string str = utils::format("[%08X] attempt send to dead service [%08X]: %s.", sender, receiver, hexdata.data());

                msg.setSessionId(-msg.sessionId());
                m_server->response(sender, str, msg.sessionId(), PTYPE_ERROR);
            }
            return s;
        }
    }

    double start_time = Time::clock();
    handle_message(s, std::move(msg));
    double diff_time = Time::clock() - start_time;
    if (diff_time > 0.1) {
        logWarning("Worker %u handle one Message(%d) cost %f, from %08X to %08X", id(), type, diff_time, sender, receiver);
    }
    return s;
}
}
