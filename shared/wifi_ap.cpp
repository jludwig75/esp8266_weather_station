#include "wifi_ap.h"
#include <ESP8266WiFi.h>

bool start_access_point(const char *ap_ssid, const char *ap_password)
{
	Serial.println("\r\nStarting WiFi Access Point");
	if (!WiFi.mode(WIFI_AP_STA))
	{
		Serial.println("Failed to set WiFi mode to access point + station");
		return false;
	}

	// Start the AP
	if (!WiFi.softAP(ap_ssid, ap_password))
	{
		Serial.println("Failed to start access point");
		return false;
	}
	Serial.print("Access point started with SSID ");
	Serial.println(ap_ssid);
	Serial.print("Access point IP address = ");
	Serial.println(WiFi.softAPIP());
	return true;
}

