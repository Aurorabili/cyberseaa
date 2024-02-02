#include "server/network/ClientInfo.hpp"

ClientInfo::~ClientInfo() { }

s64 ClientInfo::getId() const { return m_id; }

std::string ClientInfo::getName() const { return m_name; }
