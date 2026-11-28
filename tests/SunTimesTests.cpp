#include "../src/SunTimes.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <initializer_list>
#include <limits>

time_t utc(int year, int month, int day, int hour = 0, int minute = 0) {
    struct tm value_ = {};
    value_.tm_year = year - 1900;
    value_.tm_mon = month - 1;
    value_.tm_mday = day;
    value_.tm_hour = hour;
    value_.tm_min = minute;
#if defined(_WIN32)
    return _mkgmtime(&value_);
#else
    return timegm(&value_);
#endif
}

void checkTimes(time_t date, double latitude, double longitude, time_t rise, time_t set) {
    SunTimes::Result result_ = SunTimes::calculate(date, latitude, longitude);
    assert(result_.status == SunTimes::Status::Ready);
    std::printf("Sunrise difference: %.0fs; sunset difference: %.0fs\n",
        std::difftime(result_.sunrise, rise), std::difftime(result_.sunset, set));
    assert(std::abs(std::difftime(result_.sunrise, rise)) <= 120);
    assert(std::abs(std::difftime(result_.sunset, set)) <= 120);
}

int main() {
    assert(SunTimes::setTimezone());
    // Published minute-resolution values, converted from local time to UTC:
    // https://www.timeanddate.com/sun/switzerland/zurich?month=6&year=2026
    // https://www.timeanddate.com/sun/uk/london?month=3&year=2026
    // https://www.timeanddate.com/sun/australia/sydney?month=12&year=2026
    checkTimes(utc(2026, 6, 1), 47.3769, 8.5417,
        utc(2026, 6, 1, 3, 33), utc(2026, 6, 1, 19, 14));
    checkTimes(utc(2026, 3, 1), 51.5074, -0.1278,
        utc(2026, 3, 1, 6, 45), utc(2026, 3, 1, 17, 40));
    checkTimes(utc(2026, 12, 1), -33.8688, 151.2093,
        utc(2026, 11, 30, 18, 37), utc(2026, 12, 1, 8, 50));

    assert(SunTimes::calculate(utc(2026, 6, 21), 69.6492, 18.9553).status
        == SunTimes::Status::PolarDay);
    assert(SunTimes::calculate(utc(2026, 12, 21), 69.6492, 18.9553).status
        == SunTimes::Status::PolarNight);
    assert(SunTimes::calculate(utc(2026, 6, 21), -90, 0).status
        == SunTimes::Status::PolarNight);
    assert(SunTimes::calculate(utc(2026, 12, 21), -90, 0).status
        == SunTimes::Status::PolarDay);

    time_t date_ = utc(2024, 2, 29);
    auto morning_ = SunTimes::calculate(date_, 47.3769, 8.5417);
    auto evening_ = SunTimes::calculate(date_ + 86399, 47.3769, 8.5417);
    auto tomorrow_ = SunTimes::calculate(date_ + 86400, 47.3769, 8.5417);
    assert(morning_.status == SunTimes::Status::Ready);
    assert(morning_.sunrise == evening_.sunrise && morning_.sunset == evening_.sunset);
    assert(tomorrow_.sunrise > morning_.sunrise && tomorrow_.sunset > morning_.sunset);

    for (double longitude_ : {-180.0, 180.0}) {
        auto result_ = SunTimes::calculate(utc(2026, 3, 20), 0, longitude_);
        assert(result_.status == SunTimes::Status::Ready);
        assert(result_.sunset > result_.sunrise);
        assert(std::abs(std::difftime(result_.sunset, result_.sunrise) - 43200) < 600);
    }
    assert(SunTimes::calculate(date_, 91, 0).status == SunTimes::Status::PositionUnavailable);
    assert(SunTimes::calculate(date_, 0, 181).status == SunTimes::Status::PositionUnavailable);
    assert(SunTimes::calculate(date_, std::numeric_limits<double>::quiet_NaN(), 0).status
        == SunTimes::Status::PositionUnavailable);
    assert(SunTimes::calculate(-1, 0, 0).status == SunTimes::Status::TimeUnavailable);

    char formatted_[48];
    assert(SunTimes::formatTime(utc(2026, 6, 1, 3, 33), formatted_, sizeof(formatted_)));
    assert(std::strcmp(formatted_, "2026-06-01 03:33 UTC") == 0);
    assert(SunTimes::setTimezone("CET-1"));
    assert(SunTimes::formatTime(utc(2026, 6, 1, 3, 33), formatted_, sizeof(formatted_)));
    assert(std::strcmp(formatted_, "2026-06-01 04:33 CET") == 0);
    auto afterLocalMidnight_ = SunTimes::calculate(utc(2026, 6, 1, 23, 30), 47.3769, 8.5417);
    auto nextDay_ = SunTimes::calculate(utc(2026, 6, 2, 12), 47.3769, 8.5417);
    assert(afterLocalMidnight_.sunrise == nextDay_.sunrise);
    assert(afterLocalMidnight_.sunset == nextDay_.sunset);
    assert(!SunTimes::setTimezone(nullptr));
    assert(!SunTimes::setTimezone(""));
    assert(!SunTimes::formatTime(date_, nullptr, 0));
    assert(!SunTimes::formatTime(date_, formatted_, 1));

#if !defined(_WIN32)
    // Windows CRT uses its own DST rules, rather than POSIX M rules.
    assert(SunTimes::setTimezone("CET-1CEST,M3.5.0,M10.5.0/3"));
    assert(SunTimes::formatTime(utc(2026, 3, 29, 0, 59), formatted_, sizeof(formatted_)));
    assert(std::strcmp(formatted_, "2026-03-29 01:59 CET") == 0);
    assert(SunTimes::formatTime(utc(2026, 3, 29, 1), formatted_, sizeof(formatted_)));
    assert(std::strcmp(formatted_, "2026-03-29 03:00 CEST") == 0);
    assert(SunTimes::formatTime(utc(2026, 10, 25, 0, 59), formatted_, sizeof(formatted_)));
    assert(std::strcmp(formatted_, "2026-10-25 02:59 CEST") == 0);
    assert(SunTimes::formatTime(utc(2026, 10, 25, 1), formatted_, sizeof(formatted_)));
    assert(std::strcmp(formatted_, "2026-10-25 02:00 CET") == 0);
#endif
    assert(SunTimes::setTimezone());
    std::puts("SunTimes tests passed");
}
