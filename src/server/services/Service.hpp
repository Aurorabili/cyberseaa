#ifndef SERVICE_HPP_
#define SERVICE_HPP_

#include <asio/awaitable.hpp>
#include <asio/io_context.hpp>

using asio::awaitable;
using asio::io_context;

class Service {
public:
    virtual awaitable<void> start() = 0;
    virtual void stop() = 0;
};

#endif /* SERVICE_HPP_ */