#include "display.h"


Display::Display(int8_t _CS, int8_t _DC, int8_t _RST)
{

}

void Display::update_time_string(const String & time_string)
{
    Serial.print(time_string);
}

void Display::update_time_meridian(const String & time_meridian)
{
    Serial.print(" ");
    Serial.println(time_meridian);
}

void Display::update_date_string(const String & date_string)
{
    Serial.println(date_string);
}

void Display::update_inside_temp_string(const String & temp_string)
{
    Serial.print("In: ");
    Serial.println(temp_string);
}

void Display::update_outside_temp_string(const String & temp_string)
{
    Serial.print("Out: ");
    Serial.println(temp_string);
}

