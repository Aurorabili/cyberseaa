#include "server/network/ClientInfo.hpp"

ClientInfo::~ClientInfo() { }

s64 ClientInfo::getId() const { return m_clientId; }
