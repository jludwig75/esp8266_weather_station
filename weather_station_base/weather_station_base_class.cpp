#include "weather_station_base_class.h"

#include "wifi_ap.h"
#include "wifi_station.h"

#include "ws_common.h"


#define DISPLAY_UPDATE_INTERVAL_MS      1000              // 1 second
#define LOCAL_SENSOR_UPDATE_INTERVAL_MS ( 1 * 60 * 1000)  // 1 minute
#define UPDATE_TIME_UPDATE_INTERVAL_MS  (15 * 60 * 1000)  // 15 minutes
#define UDP_LISTEN_PORT                 8888



weather_station_base *weather_station_base::_this = NULL;

void reset()
{
	// TODO: send reset in GPIO 16
}


// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
weather_station_base::weather_station_base(const char *host_ssid, const char *host_password, uint8_t dht_pin, uint8_t dht_type) : m_wifi_connected(false), m_dht(dht_pin, dht_type, 11), m_server(TEMP_REPORT_SERVER_LISTEN_PORT), // 11 works fine for ESP8266
	m_last_time_update(millis()),
	m_last_local_sensor_update(millis()),
	m_host_ssid(host_ssid),
	m_host_password(host_password)
{
	_this = this;
}
// setup() methods
void weather_station_base::init()
{
	m_dht.begin();
	if (!start_access_point(ap_ssid, ap_password))
	{
		Serial.println("Failed to start access point! Resetting processor...");
		reset();
	}

	m_wifi_connected = connect_wifi(m_host_ssid.c_str(), m_host_password.c_str(), 60); // About 30 seconds
	if (!m_wifi_connected)
	{
		Serial.println("Failed to start WiFi! Continuing without WiFi");
	}

	Serial.println("Starting web server...\n");
	m_server.on(report_url, HTTP_POST, sensor_data_post_handler);
	m_server.on("/", root_handler);
	m_server.begin();
	Serial.println("Web server started.\n");

	Serial.println("Starting NTP client...");
	m_ntp_client.begin();
	Serial.println("Started NTP client\n");

	// ???: Sleep for DHT? probably not needed, because there will be some delay connecting to the access point.
	delay(1000);

	// Get the initial data.
	update_time(true);
	update_local_sensor_data(true);
	update_display(true);

	// Start the display update timer
	m_display_timer.attach_ms(DISPLAY_UPDATE_INTERVAL_MS, update_display_timer_func, this);
}

void weather_station_base::handle_root()
{
	m_server.send(200, "text/plain", "Hello");
}

void weather_station_base::handle_sensor_data_post()
{
	/*
	String message = "Received sensor data post\n\n";
	message += "URI: ";
	message += m_server.uri();
	message += "\nArguments: ";
	message += m_server.args();
	message += "\n";
	for (uint8_t i = 0; i < m_server.args(); i++)
	{
	message += " " + m_server.argName(i) + ": " + m_server.arg(i) + "\n";
	}
	Serial.println(message);

	Serial.printf("temp = \"%s\", humidity = \"%s\"\n", m_server.arg(temp_var_name).c_str(), m_server.arg(humidity_var_name).c_str());
	*/

	// Update the remote sensor data.
	m_current_remote_sensor_data.temperature = m_server.arg(temp_var_name).toInt();
	m_current_remote_sensor_data.humidity = m_server.arg(humidity_var_name).toInt();

	m_server.send(200, "text/plain", "OK");
}

void weather_station_base::on_loop()
{
	update_local_sensor_data();
	update_time();
	m_server.handleClient();
}

void weather_station_base::update_display_timer_func(weather_station_base *ws_base)
{
	ws_base->update_display();
}
void weather_station_base::update_display(bool update_now)
{
	// Get a snapshot of the time and sensor data.
	time_t current_time = now();
	sensor_data current_local_sensor_data = m_current_local_sensor_data;
	sensor_data current_remote_sensor_data = m_current_remote_sensor_data;

	// See if it's different than what we last displayed.
	if (update_now || m_last_display_data.changed(current_local_sensor_data, current_remote_sensor_data, current_time))
	{
		m_last_display_data.update(current_local_sensor_data, current_remote_sensor_data, current_time);  // Use same timestamp we checked against.
																											// So that in case the time just advanced,
																											// we'll know to update the display next time.
																											// Update the display if anything has changed.
		draw_display();
	}
}

void weather_station_base::update_local_sensor_data(bool update_now)
{
	time_t current_time = millis();
	if (!update_now && current_time - m_last_local_sensor_update < LOCAL_SENSOR_UPDATE_INTERVAL_MS)
	{
		return;
	}
	m_last_local_sensor_update = current_time;

	float humidity = NAN;
	float temperature = NAN;

	for (int i = 0; i < 5 && (isnan(humidity) || isnan(temperature)); i++)
	{
		humidity = m_dht.readHumidity();
		temperature = m_dht.readTemperature(true);
		delay(1000);
	}

	m_current_local_sensor_data.humidity = humidity;
	m_current_local_sensor_data.temperature = temperature;

	if (isnan(m_current_local_sensor_data.humidity))
	{
		Serial.println("Read bad humidity");
	}

	if (isnan(m_current_local_sensor_data.temperature))
	{
		Serial.println("Read bad temperature");
	}
}

void weather_station_base::update_time(bool update_now)
{
	time_t current_time = millis();
	if (!update_now && current_time - m_last_time_update < UPDATE_TIME_UPDATE_INTERVAL_MS)
	{
		return;
	}
	m_last_time_update = current_time;

	if (!m_wifi_connected)
	{
		m_wifi_connected = connect_wifi(m_host_ssid.c_str(), m_host_password.c_str(), 60); // About 30 seconds
		if (!m_wifi_connected)
		{
			Serial.println("Failed to start WiFi! Skipping time update.");
			return;
		}
	}
	// Get the time via NTP
	time_t new_time = m_ntp_client.get_time();

	// Update the time
	if (new_time == 0)
	{
		return;
	}

	setTime(new_time);
}


void weather_station_base::draw_display()
{
	tmElements_t tm;
	breakTime(now(), tm);

	// TODO: Go to LCD.
	String display = m_last_display_data.get_date_string() + " " + m_last_display_data.get_time_string() + " In: " + m_last_display_data.get_local_sensor_string() + " Out: " + m_last_display_data.get_remote_sensor_string();
	Serial.println(display);
}

void weather_station_base::sensor_data_post_handler()
{
	_this->handle_sensor_data_post();
}


void weather_station_base::root_handler()
{
	_this->handle_root();
}

