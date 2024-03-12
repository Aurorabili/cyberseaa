#ifndef TIME_HPP_
#define TIME_HPP_

#include <cassert>
#include <chrono>
#include <ctime>

#include "common/utils/String.hpp"

namespace Common {
class Time {
    using time_point = std::chrono::time_point<std::chrono::steady_clock>;
    inline static const time_point start_time_point_ = std::chrono::steady_clock::now();
    inline static std::time_t start_millsecond = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    inline static std::time_t offset_ = 0;

public:
    static double clock()
    {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time_point_).count();
    }

    static bool offset(std::time_t v)
    {
        if (v <= 0) {
            return false;
        }
        offset_ += v;
        return true;
    }

    static std::time_t now()
    {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time_point_);
        return start_millsecond + diff.count() + offset_;
    }

    // e. 2017-11-11 16:03:11.635
    static size_t milltimestamp(std::time_t t, char* buf, size_t len)
    {
        if (len < 23) {
            return 0;
        }

        auto mill = t;
        std::time_t now = mill / 1000;
        std::tm m;
        Time::localtime(&now, &m);
        uint64_t ymd = (m.tm_year + 1900ULL) * utils::pow10(15)
            + (m.tm_mon + 1ULL) * utils::pow10(12)
            + m.tm_mday * utils::pow10(9)
            + m.tm_hour * utils::pow10(6)
            + m.tm_min * utils::pow10(3)
            + m.tm_sec;

        size_t n = utils::uint64_to_str(ymd, buf);
        buf[4] = '-';
        buf[7] = '-';
        buf[10] = ' ';
        buf[13] = ':';
        buf[16] = ':';
        n += utils::uint64_to_str(1000 + mill % 1000, buf + n);
        buf[n - 4] = '.';
        return n;
    }

    static std::time_t make_time(int year, int month, int day, int hour, int min, int sec)
    {
        assert(year >= 1900);
        assert(month > 0 && month <= 12);
        assert(day > 0 && day <= 31);
        assert(hour >= 0 && hour < 24);
        assert(min >= 0 && min < 60);
        assert(sec >= 0 && sec < 60);

        std::tm _tm;
        _tm.tm_year = (year - 1900);
        _tm.tm_mon = (month - 1);
        _tm.tm_mday = day;
        _tm.tm_hour = hour;
        _tm.tm_min = min;
        _tm.tm_sec = sec;
        _tm.tm_isdst = 0;

        return mktime(&_tm);
    }

    static std::tm* localtime(std::time_t* t, std::tm* result)
    {
        localtime_r(t, result);
        return result;
    }

    static std::tm gmtime(const std::time_t& time_tt)
    {
        std::tm tm;
        gmtime_r(&time_tt, &tm);
        return tm;
    }

    static int timezone()
    {
        static int tz = 0;
        if (tz == 0) {
            auto t = std::time(nullptr);
            auto gm_tm = Time::gmtime(t);
            std::tm local_tm;
            Time::localtime(&t, &local_tm);
            auto diff = local_tm.tm_hour - gm_tm.tm_hour;
            if (diff < -12) {
                diff += 24;
            } else if (diff > 12) {
                diff -= 24;
            }
            tz = diff;
        }
        return tz;
    }
};

inline bool operator==(const std::tm& tm1, const std::tm& tm2)
{
    return (tm1.tm_sec == tm2.tm_sec && tm1.tm_min == tm2.tm_min && tm1.tm_hour == tm2.tm_hour && tm1.tm_mday == tm2.tm_mday && tm1.tm_mon == tm2.tm_mon && tm1.tm_year == tm2.tm_year && tm1.tm_isdst == tm2.tm_isdst);
}

inline bool operator!=(const std::tm& tm1, const std::tm& tm2)
{
    return !(tm1 == tm2);
}

};

#endif // TIME_HPP_