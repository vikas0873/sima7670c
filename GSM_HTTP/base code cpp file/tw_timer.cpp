#include "flageManager.cpp"
#include "globleVariables.cpp"
#include <ESP32Time.h>
#include "time.h"

class TwTimer
{
    FlagManager flagManager;
    ESP32Time rtc;

public:
    bool twTimeFromInternet(String time_zone = "UTC-5:30")
    {
        flagManager.setFlagStatus(block_status_bit_series, TIMER_FUNCTION_ACCESSED, ACCESSED, 0);
        configTime(0, 0, "pool.ntp.org");
        setenv("TZ", time_zone.c_str(), 1);
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            flagManager.setFlagStatus(block_status_bit_series, INTERNAL_TIMER, RESET, 0);
            return false;
        }
        flagManager.setFlagStatus(block_status_bit_series, INTERNAL_TIMER, SET, 0);
        unsigned long epoch_time = static_cast<unsigned long>(mktime(&timeinfo));
        rtc.setTime(epoch_time);
        return true;
    };
    bool TwTimerMain()
    {
        bool got_time = twTimeFromInternet(); // input paramter is time zone which is kept blank because default time zone is set
        return got_time;
    };
    String getFormattedTimeString()
    {
        return rtc.getTime("%d-%m-%Y %H:%M");
    };
};