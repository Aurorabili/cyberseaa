#ifndef CONSTANT_HPP_
#define CONSTANT_HPP_

#include "common/utils/IntTypes.hpp"

#define PLATFORM_UNKNOWN 0
#define PLATFORM_WINDOWS 1
#define PLATFORM_LINUX 2
#define PLATFORM_MAC 3

#define TARGET_PLATFORM PLATFORM_UNKNOWN

// mac
#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#undef TARGET_PLATFORM
#define TARGET_PLATFORM PLATFORM_MAC
#endif

// win32
#if !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#if defined(WINCE) || defined(_WIN32_WCE)
// win ce
#else
#undef TARGET_PLATFORM
#define TARGET_PLATFORM PLATFORM_WINDOWS
#endif
#endif

#if !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#undef TARGET_PLATFORM
#define TARGET_PLATFORM PLATFORM_WINDOWS
#endif

// linux
#if defined(__linux__) || defined(__linux)
#undef TARGET_PLATFORM
#define TARGET_PLATFORM PLATFORM_LINUX
#endif

//////////////////////////////////////////////////////////////////////////
// post configure
//////////////////////////////////////////////////////////////////////////

// check user set platform
#if !TARGET_PLATFORM
#error "Cannot recognize the target platform; are you targeting an unsupported platform?"
#endif

#ifndef __has_feature // Clang - feature checking macros.
#define __has_feature(x) 0 // Compatibility with non-clang compilers.
#endif

#undef min
#undef max

#ifdef __linux__
#include <sys/syscall.h>
#include <unistd.h>
#endif

#define VA_ARGS_NUM(...) std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value

#define thread_sleep(x) std::this_thread::sleep_for(std::chrono::milliseconds(x));

namespace Common {

inline std::size_t _thread_id()
{
#ifdef _WIN32
    return static_cast<std::size_t>(GetCurrentThreadId());
#elif __linux__
    return static_cast<std::size_t>(syscall(SYS_gettid));
#elif __FreeBSD__
    long tid;
    thr_self(&tid);
    return static_cast<std::size_t>(tid);
#else
    return static_cast<std::size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

inline std::size_t threadId()
{
#if defined(_MSC_VER) && (_MSC_VER < 1900) || defined(__clang__) && !__has_feature(cxx_thread_local)
    return _thread_id();
#else
    static thread_local const size_t tid = _thread_id();
    return tid;
#endif
}

constexpr u32 WORKER_ID_SHIFT = 24;
constexpr u32 WORKER_MAX_SERVICE = (1 << 24) - 1;

constexpr u8 PTYPE_UNKNOWN = 0;
constexpr u8 PTYPE_SYSTEM = 1;
constexpr u8 PTYPE_TEXT = 2;
constexpr u8 PTYPE_LUA = 3;
constexpr u8 PTYPE_ERROR = 4;
constexpr u8 PTYPE_DEBUG = 5;
constexpr u8 PTYPE_SHUTDOWN = 6;
constexpr u8 PTYPE_TIMER = 7;
constexpr u8 PTYPE_SOCKET_TCP = 8;
constexpr u8 PTYPE_SOCKET_UDP = 9;
constexpr u8 PTYPE_SOCKET_WS = 10;
constexpr uint8_t PTYPE_INTEGER = 12;

constexpr u32 BOOTSTRAP_ADDR = 0x01000001; // The first service's id
}

#endif // CONSTANT_HPP_