#ifndef INTERNALSERVICE_HPP
#define INTERNALSERVICE_HPP

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "common/Debug.hpp"
#include "common/utils/IntTypes.hpp"
#include "server/core/Worker.hpp"
#include "server/core/services/Handler.hpp"
#include "server/core/services/Service.hpp"

namespace Core {
class InternalService : public Core::Service {
public:
    InternalService() = default;

    InternalService(const InternalService&) = delete;
    InternalService& operator=(const InternalService&) = delete;
    InternalService(const InternalService&&) = delete;
    InternalService& operator=(const InternalService&&) = delete;

public:
    bool init(const ServiceConfig& conf) override;
    void dispatch(Message* msg) override;

private:
    std::unique_ptr<Handler> m_handler;
};

}
#endif // INTERNALSERVICE_HPP