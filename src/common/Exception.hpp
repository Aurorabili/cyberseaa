#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <exception>
#include <string>

#include "common/Types.hpp"
#include "common/utils/IntTypes.hpp"

#define _FILE                                                       \
    (strrchr(__FILE__, '/')           ? strrchr(__FILE__, '/') + 1  \
            : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 \
                                      : __FILE__)

#define EXCEPTION(...) (Exception(__LINE__, _FILE, __VA_ARGS__))

namespace Common {
class Exception : public std::exception {
public:
    template <typename... Args>
    Exception(u16 line, const std::string& filename, Args... args) noexcept
    {
        m_errorMsg += "at " + filename + ":" + std::to_string(line) + ": ";
        m_errorMsg += Common::makeString(std::forward<Args>(args)...);
    }

    virtual ~Exception() = default;

    virtual const char* what() const noexcept
    {
        return m_errorMsg.c_str();
    }

private:
    std::string m_errorMsg;
};

#ifndef DEBUG
#define DEBUG_ASSERT(cnd, msg) assert(cnd&& msg)
#else
#define DEBUG_ASSERT(cnd, msg)
#endif

#define DEBUG_CHECK(cnd, msg)                                   \
    {                                                           \
        if (!(cnd))                                             \
            throw Common::Exception { __LINE__, _FILE, (msg) }; \
    }
}

#endif // EXCEPTION_HPP_
