#include <ESP8266WiFi.h>
#include <Windows.h>


extern "C" void yield()
{
    Sleep(10);
}

extern "C" void delay(unsigned long miliseconds)
{
	Sleep(miliseconds);
}

extern "C" unsigned long millis()
{
    return GetTickCount();
}

static bool g_wifi_connected = false;

bool connect_wifi(const char *ap_ssid, const char *ap_password, int retries)
{
    g_wifi_connected = true;
	return g_wifi_connected;
}

bool disconnect_wifi()
{
    g_wifi_connected = false;
    return true;
}

bool is_wifi_connected()
{
    return g_wifi_connected;
}

IPAddress get_local_ip()
{
    return IPAddress(127, 0, 0, 1);
}

extern "C" {

	void pinMode(uint8_t, uint8_t)
	{

	}

	void analogWrite(uint8_t, int)
	{

	}

	int analogRead(uint8_t)
	{
		return 511;
	}

}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
