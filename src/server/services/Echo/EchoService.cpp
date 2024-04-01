#include "server/services/Echo/EchoService.hpp"

#include <asio/co_spawn.hpp>
#include <memory>
#include <string_view>

#include "common/Constant.hpp"
#include "common/Debug.hpp"
#include "server/core/Message.hpp"
#include "server/interface/Core.hpp"

using namespace std::literals::string_view_literals;

EchoService::EchoService()
{
}

bool EchoService::init(const Core::ServiceConfig& conf)
{
    m_commands["set_fd"] = std::bind(&EchoService::cSetFd, this, std::placeholders::_1);
    return true;
}

void EchoService::dispatch(Core::Message* msg)
{
    auto messageCopy = std::make_shared<Core::Message>(msg->clone());
    auto sessionId = msg->sessionId();
    if (sessionId != 0) {
        this->resumeSession(sessionId, messageCopy);
        return;
    }

    if (msg->type() == PTYPE_COMMAND) {
        std::string cmd = msg->data();
        std::string_view sv(cmd);
        std::string_view cmdName = sv.substr(0, sv.find('|'));

        auto it = m_commands.find(std::string(cmdName));
        if (it != m_commands.end()) {
            it->second(cmd);
        } else {
            logError("Command not found: %s", cmdName.data());
        }
    }

    if (msg->type() == PTYPE_SHUTDOWN) {
        this->reset();
    }
}

void EchoService::cSetFd(const std::string& cmd)
{
    auto args = utils::split<std::string>(cmd, "|"sv);
    m_fd = std::stoul(args[1]);
    m_fdOwner = std::stoul(args[2]);
    logInfo("Set fd: %d, owner: %d", m_fd, m_fdOwner);
    this->setReady();
    asio::co_spawn(this->worker()->io_context(), this->run(), asio::detached);
}

asio::awaitable<void> EchoService::run()
{
    logInfo("EchoService is running");

    while (this->ready()) {
        auto msg = co_await asioRecv(this, m_fdOwner, m_fd, 0, "\n"sv);
        if (msg == nullptr) {
            logError("Failed to receive message");
            continue;
        }

        logInfo("Received socket[%d] message: %s", m_fd, msg->data());

        auto buf = std::make_unique<Buffer>(msg->size());
        buf->write_back(msg->data(), msg->size());
        asioSend(this, m_fd, std::move(buf));
    }

    logInfo("EchoService is stopped");
    co_return;
}