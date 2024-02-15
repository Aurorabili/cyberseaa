#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <string>

#include "common/logger/Log.hpp"

class Logger {
public:
    virtual void print(const Log& log) = 0;

public:
    virtual Logger& operator<<(const std::string& str) = 0;
    virtual Logger& operator<<(int i) = 0;
    virtual Logger& operator<<(char c) = 0;
    virtual Logger& operator<<(std::ostream& (*f)(std::ostream&)) = 0;
};

#endif /* LOGGER_HPP_ */
