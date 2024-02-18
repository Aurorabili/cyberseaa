#ifndef COREMESSAGE_HPP_
#define COREMESSAGE_HPP_

#include <string>

class CoreMessage {
public:
    CoreMessage() = default;
    CoreMessage(std::string sender, std::string receiver)
        : sender(sender)
        , receiver(receiver) {};
    virtual ~CoreMessage() { }
    std::string sender;
    std::string receiver;
};

#endif /* COREMESSAGE_HPP_ */