# ESP8266 Weather Station
Weather station base and remote with ESP8266

This is the source for an ESP8266-based weather station base and remote.
All firmware is built to run on the ESP8266; no other MCU is required.
The weather station uses a local DHT22 to get indoor temperature and
humidity and an LCD display to output the date, time, indoor and outdoor
temperature and humidity. The weather station base uses NTP to
periodically get the time and update the local time. Optionally an RTC
can be attached to improve time accuracy or to keep the time up-to-date
while running. If RTC is used, the Time library by Michael Margolis
keeps the time.

The following hardware components are used:

Remote:
- ESP8266 (ESP-12E)
- DHT22
- LiFePo4 battery

Base
- ESP8266 (ESP-12E)
- DHT22
- 4 AA batteries
- LD111733 3.3v regulator
- 3.3v 2.4" 240x320 SD ILI9341 GM SPI LCD
- 3.3v DS3231 AT24C32 I2C RTC (optional)

Proposed Weather Station Display:

![Alt text](/Weather Station Display.png?raw=true "Proposed Weather Station Display")
