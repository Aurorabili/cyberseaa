#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <exception>
#include <string>

#include "common/utils/Debug.hpp"
#include "common/utils/Utils.hpp"

#define EXCEPTION(...) (Exception(__LINE__, _FILE, __VA_ARGS__))

class Exception {
public:
    template <typename... Args>
    Exception(u16 line, const std::string& filename, Args... args) noexcept
    {
        m_errorMsg = Logger::textColor(LoggerColor::Red, true);
        m_errorMsg += "at " + filename + ":" + std::to_string(line) + ": ";
        m_errorMsg += utils::makeString(std::forward<Args>(args)...);
        m_errorMsg += Logger::textColor();
    }

    virtual ~Exception() = default;

    virtual const char* what() const noexcept
    {
        return m_errorMsg.c_str();
    }

private:
    std::string m_errorMsg;
};

#endif // EXCEPTION_HPP_
