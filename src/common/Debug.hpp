#ifndef DEBUG_HPP_
#define DEBUG_HPP_

#include <cstring>

#include "common/Log.hpp"

#define logInfo(fmt, ...) Common::Log::instance().logfmt(true, Common::LogLevel::Info, fmt, ##__VA_ARGS__);
#define logWarning(fmt, ...) Common::Log::instance().logfmt(true, Common::LogLevel::Warn, fmt " (%s:%d)", ##__VA_ARGS__, _FILE, __LINE__);
#define logError(fmt, ...) Common::Log::instance().logfmt(true, Common::LogLevel::Error, fmt " (%s:%d)", ##__VA_ARGS__, _FILE, __LINE__);
#define logDebug(fmt, ...) Common::Log::instance().logfmt(true, Common::LogLevel::Debug, fmt " (%s:%d)", ##__VA_ARGS__, _FILE, __LINE__);

#endif // DEBUG_HPP_
