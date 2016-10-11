#pragma once
#include <Time.h>


class WiFiUDP;

class NtpClient
{
public:
	NtpClient(int tz_adjust_hours = 0);	// You can refactor this if you live in a TZ with a half hour difference.
	~NtpClient();

	void begin(uint16_t udp_listen_port = 8888);

	time_t get_time() const;
private:
	WiFiUDP *m_udp;
	int _tz_adjust_hours;
};
