# esp8266_weather_station
Weather station base and remote with ESP8266

This is the source for an ESP8266 based weather station base and remote.
All firmware is built to run on the ESP8266; no other MCU is required.
The weather station uses a local DHT22 to get indoor temperature and
humidity and an LCD display to output the date, time, indoor and outdoor
temperature and humidity. The weather station base uses NTP to
periodically get the time and update the local time. Optionally an RTC
can be attached to improve time accuracy or to keep the time up-to-date
while running. If RTC is used, the Time library by Michael Margolis
keeps the time.

The following hardware is used:

Remote:
- ESP8266 (ESP-12E)
- DHT22
- LiFePo4 battery

Base
- ESP8266 (ESP-12E)
- DHT22
- 6 AA batteries
- LDO 3.3v regulator
- 3.3v SPI LCD
- 3.3v I2C RTC (optional)
