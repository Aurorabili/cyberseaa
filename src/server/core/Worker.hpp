#ifndef WORKER_HPP_
#define WORKER_HPP_

#include <atomic>

#include <asio.hpp>

#include "common/ConcurrentQueue.hpp"
#include "server/core/Message.hpp"
#include "server/core/Service.hpp"
#include "server/core/network/SocketServer.hpp"

namespace Core {
class Server;

class Worker {
    using ConcurrentQueue = concurrent_queue<Message, std::mutex, std::vector>;

    using AsioWorkType = asio::executor_work_guard<asio::io_context::executor_type>;

public:
    friend class SocketServer;

    explicit Worker(Server* srv, u32 id);

    ~Worker();

    Worker(const Worker&) = delete;

    Worker& operator=(const Worker&) = delete;

    asio::io_context& io_context();

    u32 id() const;

    void new_service(std::unique_ptr<ServiceConfig> conf);

    void remove_service(u32 serviceid, u32 sender, u32 sessionid);

    void scan(u32 sender, s32 sessionid);

    void send(Message&& msg);

    void shared(bool v);

    bool shared() const;

    SocketServer& socket_server() { return *m_socketServer; }

    size_t mq_size() { return m_mqsize.load(std::memory_order_acquire); }

    u32 alive();

    u32 count() { return m_count.load(std::memory_order_acquire); }

    void run();

    void stop();

    void wait();

private:
    Service* handle_one(Service* service, Message&& msg);

    Service* find_service(u32 serviceid) const;

private:
    std::atomic_bool m_shared = true;
    std::atomic_uint32_t m_count = 0;
    std::atomic_size_t m_mqsize = 0;
    u32 m_nextid = 0;
    u32 m_workerid = 0;
    u32 m_version = 0;
    Server* m_server;
    asio::io_context m_ioc;
    AsioWorkType m_work;
    std::thread m_thread;
    ConcurrentQueue m_queue;
    std::unique_ptr<SocketServer> m_socketServer;
    std::unordered_map<u32, ServicePtr> m_services;
};
};

#endif /* WORKER_HPP_ */