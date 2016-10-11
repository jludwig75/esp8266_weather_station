#include "ntp_client.h"

#include <ESP8266WiFi.h>

#include <time.h>


NtpClient::NtpClient(int tz_adjust_hours) : m_udp(NULL), _tz_adjust_hours(tz_adjust_hours)
{
}

NtpClient::~NtpClient()
{
}

void NtpClient::begin(uint16_t udp_listen_port)
{
}

extern "C" time_t time(time_t *t);

time_t NtpClient::get_time() const
{
	return time(NULL) + _tz_adjust_hours * SECS_PER_HOUR;
}
