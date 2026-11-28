#ifndef SUN_TIMES_H
#define SUN_TIMES_H

#include <time.h>
#include <stddef.h>

class SunTimes {
public:
    enum class Status {
        Ready,
        TimeUnavailable,
        PositionUnavailable,
        PolarDay,
        PolarNight
    };

    struct Result {
        Status status = Status::TimeUnavailable;
        time_t sunrise = 0;
        time_t sunset = 0;
    };

    static bool setTimezone(const char* timezone = "UTC0");
    static Result calculate(time_t systemTime, double latitude, double longitude);
    static bool formatTime(time_t timestamp, char* buffer, size_t size);
};

#endif
