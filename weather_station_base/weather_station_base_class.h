#pragma once

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Ticker.h>
#include "ntp_client.h"

#include "OOWebServer.h"

#include "sensor_data.h"
#include "display_data.h"
#include "display.h"

class Timezone;
class DS1307RTC;



class WeatherStationBase : public OOWebServer<WeatherStationBase>
{
public:
	// Initialize DHT sensor 
	// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
	// you need to increase the threshold for cycle counts considered a 1 or 0.
	// You can do this by passing a 3rd parameter for this threshold.  It's a bit
	// of fiddling to find the right value, but in general the faster the CPU the
	// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
	// Arduino Due that runs at 84mhz a value of 30 works.
	// This is for the ESP8266 processor on ESP-01 
	WeatherStationBase(uint8_t dht_pin, uint8_t dht_type);
	~WeatherStationBase();

	void server_begin();

	void handle_root();

	void handle_sensor_data_post();

	void handleTime();
	void setTime();
	void setTimeZone();

	void handleWiFiConfig();
    void setWiFiConfig();

	void on_loop();

protected:
	static void update_display_timer_func(WeatherStationBase *ws_base);
	void update_display(bool update_now = false);

	void update_local_sensor_data(bool update_now = false);
	void update_time(bool update_now = false);

    bool load_config();
    bool save_wifi_config(const String & ap, const String & ap_passwd);
	bool save_tz_config(const String & std_string, int std_offset, const String & dst_string, int dst_offset);
	bool save_config(const String & ap, const String & ap_passwd, const String & std_string, int std_offset, const String & dst_string, int dst_offset);

private:
	void draw_display();
	void set_backlight_level(int level);
	void adjust_back_light();

	bool m_wifi_connected;
	time_t m_last_time_update;
	time_t m_last_local_sensor_update;
	String m_host_ssid;
	String m_host_password;
    DHT m_dht;
	NtpClient m_ntp_client;
	Ticker m_display_timer;
	Ticker m_adjust_backlight_timer;
	sensor_data m_current_local_sensor_data;
	sensor_data m_current_remote_sensor_data;
	display_data m_last_display_data;
	String m_std_string;
	String m_dst_string;
	int m_std_offset;
	int m_dst_offset;
	Timezone *m_tz;
    Display m_display;
	DS1307RTC *m_rtc;
	int m_backlight_level;
	int m_last_light_level;
};
