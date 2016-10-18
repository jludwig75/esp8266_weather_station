#include "wifi_station.h"


bool connect_wifi(const char *ap_ssid, const char *ap_password, int retries)
{
	// Connect to host WiFi network
	wl_status_t status = WiFi.begin(ap_ssid, ap_password);
	Serial.print("WiFi.begin status = ");
	Serial.println(status);
	Serial.print("\n\r \n\rWorking to connect");

	// Wait for connection
	int attempts = 0;
	while (WiFi.status() != WL_CONNECTED && attempts < retries) {
		delay(500);
		Serial.print(".");
		attempts++;
	}
	if (WiFi.status() != WL_CONNECTED)
	{
		Serial.println("");
		Serial.print("Failed tp connect to ");
		Serial.println(ap_ssid);
		WiFi.disconnect();
		return false;
	}

	Serial.println("");
	Serial.println("DHT Weather Reporting Client");
	Serial.print("Connected to ");
	Serial.println(ap_ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	return true;
}


bool disconnect_wifi()
{
    return WiFi.disconnect();
}

bool is_wifi_connected()
{
    return WiFi.isConnected();
}


IPAddress get_local_ip()
{
    return WiFi.localIP();
}
