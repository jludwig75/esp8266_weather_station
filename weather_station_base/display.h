#pragma once

#include <WString.h>


class Adafruit_ILI9341;


class Display
{
public:
	struct Region
	{
		int16_t x;
		int16_t y;
		int16_t w;
		int16_t h;

	};
    Display(int8_t _CS, int8_t _DC, int8_t _RST = -1);

    void update_time_string(const String & time_string);        // 5:09
    void update_time_meridian(const String & time_meridian);    // AM/PM
    void update_date_string(const String & date_string);        // Wed, Oct 21, 2016
    void update_inside_temp_string(const String & temp_string); // 72\xA7 F 42% RH
    void update_outside_temp_string(const String & temp_string);// 79\xA7 F 48% RH
protected:
	void update_region(const Region &region, const String &str);
	void clear_region(const Region &region);
	void write_region(const Region &region, const String &str);
private:
	String m_last_time_string;
	String m_last_time_meridian;
	String m_last_date_string;
	String m_last_inside_temp_string;
	String m_last_outside_temp_string;
};
