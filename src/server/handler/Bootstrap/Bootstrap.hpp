#ifndef BOOTSTRAP_HPP_
#define BOOTSTRAP_HPP_

#include "server/core/services/Handler.hpp"
class BootStrap : public Core::Handler {
public:
    BootStrap(Core::InternalService* s)
        : Core::Handler(s)
    {
    }
};

#endif // BOOTSTRAP_HPP_