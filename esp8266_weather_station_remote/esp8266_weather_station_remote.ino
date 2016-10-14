/* DHTServer - ESP8266 Webserver with a DHT sensor as an input

   Based on ESP8266Webserver, DHTexample, and BlinkWithoutDelay (thank you)

   Version 1.0  5/3/2014  Version 1.0   Mike Barela for Adafruit Industries
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>
#include "wifi_station.h"
#include "ws_common.h"

#define DHTTYPE DHT22
#define DHTPIN  2

// Data shared between client and server:

//const char *report_server = "192.168.4.1";
const char *report_server = "127.0.0.1";


// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
 
const unsigned int report_interval_minutes = 5;
const unsigned long report_interval_ms = report_interval_minutes * 60 * 1000;              // interval at which to read sensor in ms
 
void sendSensorData(const char *server, float temperature, float humidity)
{
  WiFiClient client;

  String postData = String("station_id=2&") + temp_var_name + "=" + String((int)temperature) + "&" + humidity_var_name + "=" + String((int)humidity);
  
  // If there's a successful connection, send the HTTP POST request
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.print(" on port ");
  Serial.print(TEMP_REPORT_SERVER_LISTEN_PORT);
  Serial.println("...");
  if (client.connect(server, TEMP_REPORT_SERVER_LISTEN_PORT))
  {
    Serial.println("connected to server");

    client.println(String("POST ") + report_url + " HTTP/1.1");

    client.println("Host: " + String(server));
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);
    Serial.println("Successfully reported temperature data");
  } 
  else
  {
    // If you couldn't make a connection:
    Serial.println("Connection failed");
  }
  client.stop();
}

void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
  dht.begin();           // initialize temperature sensor

  Serial.println("\r\nWaiting for DHT...");
  delay(1000);

  float humidity = dht.readHumidity();          // Read humidity (percent)
  float temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit

  Serial.println("Temperature: " + String((int)temp_f) + ", Humidity: " + String((int)humidity));

  // Connect to WiFi network
  if (connect_wifi(ap_ssid, ap_password, 40))
  {
	  Serial.println("Sending sensor data to server...");
	  sendSensorData(report_server, temp_f, humidity);
  }
  else
  {
	  Serial.print("Failed to connect to AP '");
      Serial.print(ap_ssid);
      Serial.println("'");
  }
  Serial.println("Sleeping...");
  ESP.deepSleep(report_interval_ms * 1000);
}
 
void loop(void)
{
} 

