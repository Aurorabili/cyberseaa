#ifndef SESSION_HPP_
#define SESSION_HPP_

#include <asio/awaitable.hpp>
#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>

#include "common/utils/IntTypes.hpp"

template <typename T>
class Session {
public:
    Session(s32 sessionId, asio::io_context& ctx)
        : sessionId(sessionId)
        , ctx(ctx)
        , timer(ctx)
    {
    }

    asio::awaitable<T> wait()
    {
        try {
            while (!result) {
                co_await timer.async_wait(asio::use_awaitable);
            }
        } catch (std::exception& e) {
            co_return result;
        }
        co_return result;
    }

    void resume(T result)
    {
        this->result = result;
        timer.cancel();
    }

    s32 id() const
    {
        return sessionId;
    }

private:
    s32 sessionId;
    T result;
    asio::io_context& ctx;
    asio::steady_timer timer;
};

#endif /* SESSION_HPP_ */