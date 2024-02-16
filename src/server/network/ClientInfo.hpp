#ifndef CLIENTINFO_HPP_
#define CLIENTINFO_HPP_

#include <asio/ip/tcp.hpp>
#include <string>

#include "common/utils/IntTypes.hpp"

class ClientInfo {
public:
    virtual ~ClientInfo();

    virtual void send(const std::string& msg) = 0;

    s64 getId() const;
    void setId(const s64 _id) { this->m_id = _id; }
    std::string getName() const;
    void setName(const std::string _name) { this->m_name = _name; }

private:
    s64 m_id;
    std::string m_name;
};

#endif /* CLIENTINFO_HPP_ */
