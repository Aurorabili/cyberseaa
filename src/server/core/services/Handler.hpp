#ifndef HANDLER_HPP_
#define HANDLER_HPP_

#include <asio/awaitable.hpp>
namespace Core {
class InternalService;
class Handler {
public:
    Handler(Core::InternalService* service)
    {
        m_service = service;
    }
    virtual ~Handler() { }

    Handler(const Handler&) = delete;
    Handler& operator=(const Handler&) = delete;
    Handler(const Handler&&) = delete;
    Handler& operator=(const Handler&&) = delete;

public:
    virtual asio::awaitable<void> start() = 0;

protected:
    Core::InternalService* m_service;
};
}

#endif // HANDLER_HPP_