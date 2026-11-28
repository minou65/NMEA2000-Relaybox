#include "SunTimes.h"

#include <cmath>
#include <cstdlib>

namespace {
constexpr double Pi = 3.14159265358979323846;
constexpr double Radians = Pi / 180.0;
constexpr double SecondsPerDay = 86400.0;

struct SolarPosition {
    double declination;
    double equationOfTime;
};

bool localTime(time_t timestamp, struct tm& result) {
#if defined(_WIN32)
    return localtime_s(&result, &timestamp) == 0;
#else
    return localtime_r(&timestamp, &result) != nullptr;
#endif
}

// NOAA solar equations; standard horizon includes refraction and the solar radius.
SolarPosition solarPosition(double julianDay) {
    double t_ = (julianDay - 2451545.0) / 36525.0;
    double longitude_ = std::fmod(280.46646 + t_ * (36000.76983 + t_ * 0.0003032), 360.0);
    double anomaly_ = (357.52911 + t_ * (35999.05029 - 0.0001537 * t_)) * Radians;
    double eccentricity_ = 0.016708634 - t_ * (0.000042037 + 0.0000001267 * t_);
    double center_ = std::sin(anomaly_) * (1.914602 - t_ * (0.004817 + 0.000014 * t_))
        + std::sin(2 * anomaly_) * (0.019993 - 0.000101 * t_)
        + std::sin(3 * anomaly_) * 0.000289;
    double omega_ = (125.04 - 1934.136 * t_) * Radians;
    double apparentLongitude_ = (longitude_ + center_ - 0.00569 - 0.00478 * std::sin(omega_)) * Radians;
    double obliquity_ = (23.0 + (26.0 + (21.448 - t_ * (46.815 + t_ * (0.00059 - t_ * 0.001813))) / 60.0) / 60.0
        + 0.00256 * std::cos(omega_)) * Radians;
    double y_ = std::tan(obliquity_ / 2.0);
    y_ *= y_;
    double l_ = longitude_ * Radians;
    double equation_ = y_ * std::sin(2 * l_) - 2 * eccentricity_ * std::sin(anomaly_)
        + 4 * eccentricity_ * y_ * std::sin(anomaly_) * std::cos(2 * l_)
        - 0.5 * y_ * y_ * std::sin(4 * l_)
        - 1.25 * eccentricity_ * eccentricity_ * std::sin(2 * anomaly_);
    return {std::asin(std::sin(obliquity_) * std::sin(apparentLongitude_)),
        4.0 * equation_ / Radians};
}

double hourAngleCosine(double latitude, double declination) {
    return (std::cos(90.833 * Radians) - std::sin(latitude) * std::sin(declination))
        / (std::cos(latitude) * std::cos(declination));
}
}

bool SunTimes::setTimezone(const char* timezone) {
    if (timezone == nullptr || timezone[0] == '\0') {
        return false;
    }
#if defined(_WIN32)
    if (_putenv_s("TZ", timezone) != 0) {
        return false;
    }
    _tzset();
#else
    if (setenv("TZ", timezone, 1) != 0) {
        return false;
    }
    tzset();
#endif
    return true;
}

bool SunTimes::formatTime(time_t timestamp, char* buffer, size_t size) {
    struct tm local_;
    return buffer != nullptr && size > 0 && localTime(timestamp, local_)
        && strftime(buffer, size, "%Y-%m-%d %H:%M %Z", &local_) != 0;
}

SunTimes::Result SunTimes::calculate(time_t systemTime, double latitude, double longitude) {
    Result result_;
    struct tm local_;
    if (systemTime < 0 || !localTime(systemTime, local_)) {
        return result_;
    }
    if (!std::isfinite(latitude) || !std::isfinite(longitude)
        || latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
        result_.status = Status::PositionUnavailable;
        return result_;
    }

    // Use the local calendar date, but keep event timestamps in UTC.
    int year_ = local_.tm_year + 1900;
    int month_ = local_.tm_mon + 1;
    if (month_ <= 2) {
        --year_;
        month_ += 12;
    }
    int century_ = year_ / 100;
    double julianDay_ = std::floor(365.25 * (year_ + 4716))
        + std::floor(30.6001 * (month_ + 1)) + local_.tm_mday
        + 2 - century_ + century_ / 4 - 1524.5;
    time_t midnight_ = static_cast<time_t>((julianDay_ - 2440587.5) * SecondsPerDay);
    double noon_ = 720.0 - 4.0 * longitude;
    for (int i_ = 0; i_ < 2; ++i_) {
        noon_ = 720.0 - 4.0 * longitude - solarPosition(julianDay_ + noon_ / 1440.0).equationOfTime;
    }
    double latitudeRadians_ = latitude * Radians;
    SolarPosition sun_ = solarPosition(julianDay_ + noon_ / 1440.0);
    double cosine_ = hourAngleCosine(latitudeRadians_, sun_.declination);
    if (cosine_ > 1.0 || cosine_ < -1.0) {
        result_.status = cosine_ > 1.0 ? Status::PolarNight : Status::PolarDay;
        return result_;
    }

    double events_[2];
    for (int event_ = 0; event_ < 2; ++event_) {
        double direction_ = event_ == 0 ? -1.0 : 1.0;
        double minutes_ = noon_ + direction_ * 4.0 * std::acos(cosine_) / Radians;
        for (int i_ = 0; i_ < 2; ++i_) {
            SolarPosition position_ = solarPosition(julianDay_ + minutes_ / 1440.0);
            double eventCosine_ = hourAngleCosine(latitudeRadians_, position_.declination);
            if (eventCosine_ > 1.0 || eventCosine_ < -1.0) {
                result_.status = eventCosine_ > 1.0 ? Status::PolarNight : Status::PolarDay;
                return result_;
            }
            minutes_ = 720.0 - 4.0 * longitude - position_.equationOfTime
                + direction_ * 4.0 * std::acos(eventCosine_) / Radians;
        }
        events_[event_] = minutes_;
    }
    result_.status = Status::Ready;
    result_.sunrise = midnight_ + static_cast<time_t>(std::round(events_[0] * 60.0));
    result_.sunset = midnight_ + static_cast<time_t>(std::round(events_[1] * 60.0));
    return result_;
}
