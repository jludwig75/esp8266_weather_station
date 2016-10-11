#include "ntp_client.h"

#include <ESP8266WiFi.h>

#include <time.h>


NtpClient::NtpClient() : m_udp(NULL)
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
	return time(NULL);
}
