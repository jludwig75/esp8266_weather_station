#include "display.h"

#include <HardwareSerial.h>

Display::Region time_region = { 20, 40, 120, 35, 4 };
Display::Region time_meridian_region = { 145, 50, 30, 25, 2 };
Display::Region date_region = { 20, 85, 210, 20, 2 };
Display::Region inside_temp_region = { 20, 160, 180, 20, 2 };
Display::Region outside_temp_region = { 20, 220, 180, 20, 2 };


Display::Display(int8_t _CS, int8_t _DC, int8_t _RST) : m_lcd(_CS, _DC, _RST)
{

}

void Display::begin()
{
	m_lcd.begin();
	m_lcd.setRotation(2);
	m_lcd.fillScreen(ILI9341_WHITE);
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

	m_lcd.fillRect(region.x, region.y, region.w, region.h, ILI9341_WHITE);
}

void Display::write_region(const Region &region, const String &str)
{
	Serial.print("Writing region @ (");
	Serial.print(region.x);
	Serial.print(", ");
	Serial.print(region.y);
	Serial.print("): ");
	Serial.println(str);
	m_lcd.setCursor(region.x + 2, region.y + 2);
	m_lcd.setTextColor(ILI9341_BLACK);
	m_lcd.setTextSize(region.font_size);
	m_lcd.print(str);
}
