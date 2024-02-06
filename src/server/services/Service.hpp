#ifndef SERVICE_HPP_
#define SERVICE_HPP_

#include <asio/awaitable.hpp>
#include <asio/io_context.hpp>

using asio::awaitable;
using asio::io_context;

class Service {
public:
    Service(io_context& ioContext)
        : m_ioContext(ioContext) {};
    virtual ~Service() = default;
    virtual awaitable<void> start() = 0;
    virtual void stop() = 0;
    virtual bool isInitialized() = 0;
    io_context& m_ioContext;
};

#endif /* SERVICE_HPP_ */