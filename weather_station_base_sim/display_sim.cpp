#include "display.h"

Display::Region time_region = { 1, 1, 5, 1 };
Display::Region time_meridian_region = { 6, 1, 2, 1 };
Display::Region date_region = { 1, 2, 20, 1 };
Display::Region inside_temp_region = { 1, 3, 20, 1 };
Display::Region outside_temp_region = { 1, 4, 20, 1 };


Display::Display(int8_t _CS, int8_t _DC, int8_t _RST) : m_lcd(_CS, _DC, _RST)
{

}

void Display::begin()
{

}

void Display::update_time_string(const String & time_string)
{
	if (time_string != m_last_time_string)
	{
		update_region(time_region, time_string);
		m_last_time_string = time_string;
	}
}

void Display::update_time_meridian(const String & time_meridian)
{
	if (time_meridian != m_last_time_meridian)
	{
		update_region(time_meridian_region, time_meridian);
		m_last_time_meridian = time_meridian;
	}
}

void Display::update_date_string(const String & date_string)
{
	if (date_string != m_last_date_string)
	{
		update_region(date_region, date_string);
		m_last_date_string = date_string;
	}
}

void Display::update_inside_temp_string(const String & temp_string)
{
	if (temp_string != m_last_inside_temp_string)
	{
		String inside_temp_string = String("In: ") + temp_string;
		update_region(inside_temp_region, inside_temp_string);
		m_last_inside_temp_string = temp_string;
	}
}

void Display::update_outside_temp_string(const String & temp_string)
{
	if (temp_string != m_last_outside_temp_string)
	{
		String outside_temp_string = String("Out: ") + temp_string;
		update_region(outside_temp_region, outside_temp_string);
		m_last_outside_temp_string = temp_string;
	}
}


void Display::update_region(const Region &region, const String &str)
{
	clear_region(region);
	write_region(region, str);
}

void Display::clear_region(const Region &region)
{
	Serial.print("Clearing region @ (");
	Serial.print(region.x);
	Serial.print(", ");
	Serial.print(region.y);
	Serial.println(")");
}

void Display::write_region(const Region &region, const String &str)
{
	Serial.print("Writing region @ (");
	Serial.print(region.x);
	Serial.print(", ");
	Serial.print(region.y);
	Serial.print("): ");
	Serial.println(str);
}
