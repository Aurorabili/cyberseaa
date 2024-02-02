#ifndef SERVERCONFIG_HPP_
#define SERVERCONFIG_HPP_

#include <string>

#include "common/utils/IntTypes.hpp"

namespace ServerConfig {

/* Base Server Config */
extern u16 server_port;
extern std::string server_name;

/* Logger Config */
extern std::string log_level;

/* UUID Provider Config */
extern s64 uuid_worker_id;
extern s64 uuid_datacenter_id;
extern s64 uuid_twepoch;

void loadConfigFromFile(const char* filepath);
void saveConfigToFile(const char* filepath);
}

#endif /* SERVERCONFIG_HPP_ */