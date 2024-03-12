#ifndef LOG_HPP_
#define LOG_HPP_

#include <atomic>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <system_error>
#include <thread>

#include "common/Buffer.hpp"
#include "common/ConcurrentQueue.hpp"
#include "common/Constant.hpp"
#include "common/Exception.hpp"
#include "common/TermColor.hpp"
#include "common/Time.hpp"
#include "common/Types.hpp"
#include "common/utils/IntTypes.hpp"
#include "common/utils/String.hpp"

namespace Common {

namespace fs = std::filesystem;

enum class LogLevel : u8 {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3,
    None = 4
};

class Log final {
public:
    static constexpr bool ENABLE_STDOUT = true;
    static constexpr bool ENABLE_CONSOLE_COLOR = true;
    static constexpr bool PRINT_FILE_AND_LINE = true;

private:
    Log()
        : m_state(State::Init)
        , m_logLevel(LogLevel::Debug)
        , m_thread(&Log::write, this)
    {
    }

public:
    static Log& instance()
    {
        static Log inst;
        return inst;
    }

    ~Log()
    {
        wait();
    }

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
    Log(Log&&) = delete;
    Log& operator=(Log&&) = delete;

public:
    void init(const std::string& path, LogLevel logLevel = LogLevel::Debug)
    {
        std::error_code ec;
        auto parent_path = fs::path(path).parent_path();
        if (!parent_path.empty() && !fs::exists(parent_path, ec))
            fs::create_directories(parent_path, ec);

        int err = 0;
#if TARGET_PLATFORM == PLATFORM_WINDOWS
        FILE* fp
            = _fsopen(logfile.data(), "w", _SH_DENYWR);
        if (nullptr == fp)
            err = errno;
#else
        FILE* fp
            = std::fopen(path.data(), "w");
        if (nullptr == fp)
            err = errno;
#endif
        DEBUG_CHECK(!err, utils::format("open log file %s failed. errno %d.", path.data(), err));
        m_fp.reset(fp);
        m_state.store(State::Running, std::memory_order_release);
    }

    template <typename... Args>
    void logfmt(bool console, LogLevel level, const char* fmt, Args&&... args)
    {
        if (m_logLevel > level) {
            return;
        }

        if (nullptr == fmt)
            return;

        std::string str = utils::format(fmt, std::forward<Args>(args)...);
        logstring(console, level, std::string_view { str });
    }

    Buffer make_line(bool enable_stdout, LogLevel level, uint64_t serviceid = 0, size_t datasize = 0)
    {
        if (level == LogLevel::Error)
            ++m_errorCount;

        enable_stdout = ENABLE_STDOUT ? enable_stdout : ENABLE_STDOUT;

        auto line = Buffer { (datasize > 0) ? (64 + datasize) : 256, 0 };
        auto it = line.begin();
        *(it++) = static_cast<char>(enable_stdout);
        *(it++) = static_cast<char>(level);
        size_t offset = format_header(std::addressof(*it), level, serviceid);
        line.commit(2 + offset);
        return line;
    }

    void push_line(Buffer line)
    {
        m_queue.push_back(std::move(line));
        ++m_size;
    }

    void logstring(bool console, LogLevel level, std::string_view s, uint64_t serviceid = 0)
    {
        if (m_logLevel > level)
            return;

        Buffer line = make_line(console, level, serviceid, s.size());
        line.write_back(s.data(), s.size());
        m_queue.push_back(std::move(line));
        ++m_size;
    }

    void set_level(LogLevel level)
    {
        m_logLevel = level;
    }

    LogLevel get_level()
    {
        return m_logLevel;
    }

    void set_level(std::string_view s)
    {
        if (utils::iequal_string(s, std::string_view { "DEBUG" })) {
            set_level(LogLevel::Debug);
        } else if (utils::iequal_string(s, std::string_view { "INFO" })) {
            set_level(LogLevel::Info);
        } else if (utils::iequal_string(s, std::string_view { "WARN" })) {
            set_level(LogLevel::Warn);
        } else if (utils::iequal_string(s, std::string_view { "ERROR" })) {
            set_level(LogLevel::Error);
        } else {
            set_level(LogLevel::Debug);
        }
    }

private:
    void wait()
    {
        if (m_state.exchange(State::Stopped, std::memory_order_acq_rel) == State::Stopped)
            return;

        if (m_thread.joinable())
            m_thread.join();

        m_fp.reset(nullptr);
    }

    size_t format_header(char* buf, LogLevel level, uint64_t serviceid) const
    {
        size_t offset = 0;
        offset += Common::Time::milltimestamp(Common::Time::now(), buf, 23);
        memcpy(buf + offset, " | ", 3);
        offset += 3;
        size_t len = 0;
        if (serviceid == 0) {
            len = utils::uint64_to_str(Common::threadId(), buf + offset);
        } else {
            buf[offset] = ':';
            len = utils::uint64_to_hexstr(serviceid, buf + offset + 1, 8) + 1;
        }
        offset += len;
        if (len < 9) {
            memcpy(buf + offset, "        ", 9 - len);
            offset += 9 - len;
        }
        std::string_view strlevel = to_string(level);
        memcpy(buf + offset, strlevel.data(), strlevel.size());
        offset += strlevel.size();
        return offset;
    }

    void do_write(const concurrent_queue<Buffer, std::mutex, std::vector>::container_type& lines)
    {
        for (auto& it : lines) {
            auto p = it.data();
            auto bconsole = static_cast<bool>(*(p++));
            auto level = static_cast<LogLevel>(*(p++));
            auto str = std::string_view { p, it.size() - 2 };
            if (bconsole) {
                switch (level) {
                case LogLevel::Error:
                    std::cerr << termcolor::red << str;
                    break;
                case LogLevel::Warn:
                    std::cout << termcolor::yellow << str;
                    break;
                case LogLevel::Info:
                    std::cout << termcolor::white << str;
                    break;
                case LogLevel::Debug:
                    std::cout << termcolor::green << str;
                    break;
                default:
                    break;
                }
                if (str.back() != '\n')
                    std::cout << termcolor::white << '\n';
            }

            if (m_fp) {
                std::fwrite(str.data(), str.size(), 1, m_fp.get());
                if (str.back() != '\n')
                    std::fputc('\n', m_fp.get());
                if (level <= LogLevel::Error) {
                    std::cout << std::endl;
                    std::fflush(m_fp.get());
                }
            }
            --m_size;
        }
        if (m_fp) {
            std::fflush(m_fp.get());
        }
        std::cout.flush();
    }

    void write()
    {
        while (m_state.load(std::memory_order_acquire) == State::Init)
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        time_t sleep_time = 1;
        while (m_state.load(std::memory_order_acquire) == State::Running) {
            if (auto& read_queue = m_queue.swap_on_read(); !read_queue.empty()) {
                sleep_time = 1;
                do_write(read_queue);
                read_queue.clear();
            } else {
                std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time));
                sleep_time = (sleep_time << 1);
                sleep_time = std::min(sleep_time, (time_t)1 << 24);
            }
        }

        if (auto& read_queue = m_queue.swap_on_read(); !read_queue.empty()) {
            do_write(read_queue);
            read_queue.clear();
        }
    }

    static constexpr std::string_view to_string(LogLevel lv)
    {
        switch (lv) {
        case LogLevel::Error:
            return " | ERROR | ";
        case LogLevel::Warn:
            return " | WARN  | ";
        case LogLevel::Info:
            return " | INFO  | ";
        case LogLevel::Debug:
            return " | DEBUG | ";
        default:
            return " | NULL  | ";
        }
    }

    struct file_deleter {
        void operator()(FILE* p) const
        {
            if (nullptr == p)
                return;
            std::fflush(p);
            std::fclose(p);
        }
    };

private:
    struct FileDeleter {
        void operator()(FILE* p) const
        {
            if (nullptr == p)
                return;
            std::fflush(p);
            std::fclose(p);
        }
    };

private:
    std::atomic<State> m_state = State::Unkown;
    std::atomic<LogLevel> m_logLevel = LogLevel::Debug;
    std::atomic_uint32_t m_size = 0;
    u32 m_errorCount = 0;
    std::unique_ptr<std::FILE, FileDeleter> m_fp;
    std::thread m_thread;
    concurrent_queue<Buffer, std::mutex, std::vector> m_queue;
};
}

#endif /* LOG_HPP_ */