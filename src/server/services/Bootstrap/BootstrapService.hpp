#ifndef BOOTSTRAPSERVICE_HPP_
#define BOOTSTRAPSERVICE_HPP_

#include "common/Constant.hpp"
#include "common/Debug.hpp"
#include "server/core/Message.hpp"
#include "server/core/Service.hpp"
#include <memory>

class BootstrapService final : public Core::Service {
public:
    BootstrapService();

    bool init(const Core::ServiceConfig& conf);
    void dispatch(Core::Message* msg);

    asio::awaitable<void> run();

private:
    template <typename R, u8 PTYPE>
    R MessageDispatcher(std::shared_ptr<Core::Message> msg)
    {
        throw std::runtime_error("Not implemented");
    }

    template <>
    std::string MessageDispatcher<std::string, PTYPE_TEXT>(std::shared_ptr<Core::Message> msg)
    {
        try {
            return msg->data();
        } catch (std::exception& e) {
            logError("Could not dispatch message[type: %d]: %s", msg->type(), e.what());
            return "";
        }
    }

    template <>
    u32 MessageDispatcher<u32, PTYPE_INTEGER>(std::shared_ptr<Core::Message> msg)
    {
        try {
            return std::stoul(msg->data());
        } catch (std::exception& e) {
            logError("Could not dispatch message[type: %d]: %s", msg->type(), e.what());
            return 0;
        }
    }

    template <>
    void MessageDispatcher<void, PTYPE_ERROR>(std::shared_ptr<Core::Message> msg)
    {
        logError("Error message[type: %d] received: %s", msg->type(), msg->data());
    }
};

#endif /* BOOTSTRAPSERVICE_HPP_ */