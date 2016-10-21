#pragma once

#include <WString.h>


class Adafruit_ILI9341;


class Display
{
public:
    Display(int8_t _CS, int8_t _DC, int8_t _RST = -1);

    void update_time_string(const String & time_string);        // 5:09
    void update_time_meridian(const String & time_meridian);    // AM/PM
    void update_date_string(const String & date_string);        // Wed, Oct 21, 2016
    void update_inside_temp_string(const String & temp_string); // 72\xA7 F 42% RH
    void update_outside_temp_string(const String & temp_string);// 79\xA7 F 48% RH
};
