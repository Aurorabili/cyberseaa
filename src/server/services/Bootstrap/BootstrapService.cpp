#include "server/services/Bootstrap/BootstrapService.hpp"
#include "common/Constant.hpp"
#include "common/Debug.hpp"
#include "common/Exception.hpp"
#include "server/core/Message.hpp"
#include "server/interface/Core.hpp"
#include <asio/co_spawn.hpp>
#include <memory>

BootstrapService::BootstrapService()
{
}

bool BootstrapService::init(const Core::ServiceConfig& conf)
{
    this->setReady();
    asio::co_spawn(this->worker()->io_context(), this->run(), asio::detached);
    return true;
}

void BootstrapService::dispatch(Core::Message* msg)
{
    /**
     * 这里必须创建消息拷贝，因为dispatch方法是被ctx.post调用的
     * 而消息实际处理在协程中，如果不拷贝消息，那么消息会在ctx.post调用结束后被销毁
     *
     * TODO: 优化消息拷贝，避免拷贝消息数据
     * !注意: 这里的clone()方法不能拷贝出相等的数据，会在DEBUG_CHECK中报错
     */
    auto messageCopy = std::make_shared<Core::Message>(msg->clone());
    // DEBUG_CHECK(std::string(msg->data()) == std::string(messageCopy->data()), "Message copy failed");

    auto sessionId = msg->sessionId();
    if (sessionId != 0) {
        this->resumeSession(sessionId, messageCopy);
        return;
    }
}

asio::awaitable<void> BootstrapService::run()
{
    logInfo("BootstrapService is running");

    auto listen = asioListen(this, "127.0.0.1", 28221, PTYPE_SOCKET_TCP);

    while (this->ready()) {
        auto newServiceConf = std::make_unique<Core::ServiceConfig>();
        newServiceConf->name = "EchoService";
        newServiceConf->type = "EchoService";
        newServiceConf->unique = false;
        newServiceConf->threadid = this->worker()->id();

        auto newServiceId = co_await newService(this, std::move(newServiceConf));
        if (newServiceId == 0) {
            logError("Failed to create new service");
            continue;
        }

        auto fd = co_await asioAccept(this, listen, newServiceId);
        if (fd == 0) {
            logError("Failed to accept new connection");
            closeService(this, newServiceId);
            continue;
        }

        logInfo("New service created: %d", newServiceId);
        auto cmd = "set_fd|" + std::to_string(fd) + "|" + std::to_string(newServiceId);
        auto buf = std::make_unique<Buffer>(cmd.size());
        buf->write_back(cmd.data(), cmd.size());
        sendMessageToService(this, newServiceId, std::move(buf), 0, PTYPE_COMMAND);
    }
}