#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Ticker.h>
#include <TimeLib.h>

#include "weather_station_base_class.h"


#define DHTTYPE DHT22
#define DHTPIN  2

const char* host_ssid            = "Caradhras";
const char* host_password        = "Speak friend.";


weather_station_base g_weather_station_base(host_ssid, host_password, DHTPIN, DHTTYPE);


void setup()
{
  Serial.begin(115200);
  g_weather_station_base.init();
}

void loop()
{
  g_weather_station_base.on_loop();
}
