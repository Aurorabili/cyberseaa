#ifndef DEBUG_HPP_
#define DEBUG_HPP_

#include <cstring>

#include "common/logger/LoggerHandler.hpp"

#define _FILE                                                       \
    (strrchr(__FILE__, '/')           ? strrchr(__FILE__, '/') + 1  \
            : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 \
                                      : __FILE__)

#define logDebug() (LoggerHandler::getInstance().print(LogLevel::Debug, _FILE, __LINE__))
#define logInfo() (LoggerHandler::getInstance().print(LogLevel::Info, _FILE, __LINE__))
#define logWarning() (LoggerHandler::getInstance().print(LogLevel::Warning, _FILE, __LINE__))
#define logError() (LoggerHandler::getInstance().print(LogLevel::Error, _FILE, __LINE__))

#define logTrace(s)                          \
    do {                                     \
        logInfo() << "Function called: " #s; \
        s                                    \
    } while (false);

#endif // DEBUG_HPP_
