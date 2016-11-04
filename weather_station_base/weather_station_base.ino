#include <RtcDateTime.h>
#include <RtcDS1307.h>
#include <RtcDS3231.h>
#include <RtcTemperature.h>
#include <RtcUtility.h>


#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Ticker.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include "weather_station_base_class.h"


#define DHTTYPE DHT22
#define DHTPIN  2


WeatherStationBase g_weather_station_base(DHTPIN, DHTTYPE);

void setup()
{
	Serial.begin(115200);
	Serial.println("Starting weather station sketch...");
	g_weather_station_base.begin();
}

void loop()
{
	g_weather_station_base.on_loop();
}
