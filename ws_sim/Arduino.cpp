#include <Windows.h>

extern "C" void yield()
{
    Sleep(10);
}

extern "C" unsigned long millis()
{
    return GetTickCount();
}

bool connect_wifi(const char *ap_ssid, const char *ap_password, int retries)
{
	return true;
}
