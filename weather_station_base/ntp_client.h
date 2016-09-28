#pragma once
#include <Time.h>


class WiFiUDP;

class NtpClient
{
public:
	NtpClient();
	~NtpClient();

	void begin(uint16_t udp_listen_port = 8888);

	time_t get_time() const;
private:
	WiFiUDP *m_udp;
};
