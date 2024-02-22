#ifndef CLIENTINFO_HPP_
#define CLIENTINFO_HPP_

#include <string>

#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>

#include "common/utils/IntTypes.hpp"

class ClientInfo {
public:
    virtual ~ClientInfo();

    virtual void send(const std::string& msg) = 0;
    virtual void disconnect() = 0;

    s64 getId() const;
    void setId(const s64 _id) { this->m_clientId = _id; }
    asio::ip::address ipAddress() const;

private:
    s64 m_clientId;
};

#endif /* CLIENTINFO_HPP_ */
