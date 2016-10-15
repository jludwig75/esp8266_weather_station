#include "weather_station_base_class.h"

#include "wifi_ap.h"
#include "wifi_station.h"

#include "ws_common.h"


#define DISPLAY_UPDATE_INTERVAL_MS      1000              // 1 second
#define LOCAL_SENSOR_UPDATE_INTERVAL_MS ( 1 * 60 * 1000)  // 1 minute
#define UPDATE_TIME_UPDATE_INTERVAL_MS  (15 * 60 * 1000)  // 15 minutes
#define UDP_LISTEN_PORT                 8888


const int timeZone = -6;     // Mountain Daylight Time
							 //const int timeZone = -5;  // Eastern Standard Time (USA)
							 //const int timeZone = -4;  // Eastern Daylight Time (USA)
							 //const int timeZone = -8;  // Pacific Standard Time (USA)
							 //const int timeZone = -7;  // Pacific Daylight Time (USA)

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
WeatherStationBase::WeatherStationBase(const char *host_ssid, const char *host_password, uint8_t dht_pin, uint8_t dht_type) :
	OOWebServer<WeatherStationBase>("/web_templates", TEMP_REPORT_SERVER_LISTEN_PORT),
	m_wifi_connected(false),
	m_dht(dht_pin, dht_type, 11), // 11 works fine for ESP8266
	m_ntp_client(timeZone),
	m_last_time_update(millis()),
	m_last_local_sensor_update(millis()),
	m_host_ssid(host_ssid),
	m_host_password(host_password)
{
}
// setup() methods
void WeatherStationBase::server_begin()
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
	on(report_url, HTTP_POST, &WeatherStationBase::handle_sensor_data_post);
	on("/", &WeatherStationBase::handle_root);
	on("/time", HTTP_GET, &WeatherStationBase::handleTime);
	on("/time", HTTP_POST, &WeatherStationBase::setTime);
	on("/wifi_config", &WeatherStationBase::handleWiFiConfig);

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

static String pad_string(const String & _str, size_t width, char fill_char)
{
	String str = _str;
	if (str.length() < width)
	{
		size_t fill_amount = width - str.length();
		for (size_t i = 0; i < fill_amount; i++)
		{
			str = fill_char + str;
		}
	}

	return str;
}

void WeatherStationBase::handle_root()
{
	String page_template = get_current_page_template();

	m_last_display_data.time;
	tmElements_t tm;
	breakTime(m_last_display_data.time, tm);

	page_template.replace("<%hour%>", String(hourFormat12(m_last_display_data.time)));
	page_template.replace("<%minute%>", pad_string(String(tm.Minute), 2, '0'));
	page_template.replace("<%meridian%>", isAM(m_last_display_data.time) ? "am" : "pm");

	page_template.replace("<%day_of_week%>", dayShortStr(tm.Wday));
	page_template.replace("<%day%>", String(tm.Day));
	page_template.replace("<%month%>", monthShortStr(tm.Month));
	page_template.replace("<%year%>", String(tmYearToCalendar(tm.Year)));

	page_template.replace("<%inside_temperature%>", String(m_last_display_data.local_data.temperature));
	page_template.replace("<%inside_humidity%>", String(m_last_display_data.local_data.humidity));

	page_template.replace("<%outside_temperature%>", 0xffff == m_last_display_data.remote_data.temperature ? "--" : String(m_last_display_data.remote_data.temperature));
	page_template.replace("<%outside_humidity%>", 0xffff == m_last_display_data.remote_data.humidity ? "--" : String(m_last_display_data.remote_data.humidity));

	send(200, "text/html", page_template);
}

void WeatherStationBase::handle_sensor_data_post()
{
	/*
	String message = "Received sensor data post\n\n";
	message += "URI: ";
	message += uri();
	message += "\nArguments: ";
	message += args();
	message += "\n";
	for (uint8_t i = 0; i < args(); i++)
	{
	message += " " + argName(i) + ": " + arg(i) + "\n";
	}
	Serial.println(message);

	Serial.printf("temp = \"%s\", humidity = \"%s\"\n", arg(temp_var_name).c_str(), arg(humidity_var_name).c_str());
	*/


	// Update the remote sensor data.
	m_current_remote_sensor_data.temperature = arg(temp_var_name).toInt();
	m_current_remote_sensor_data.humidity = arg(humidity_var_name).toInt();

	send(200, "text/plain", "OK");
}

void WeatherStationBase::handleTime()
{
	tmElements_t tm;
	time_t tNow = now();
	breakTime(tNow, tm);

	String page_template = get_current_page_template();

	page_template.replace("<%hour%>", String(tm.Hour));
	page_template.replace("<%minute%>", String(tm.Minute));

	page_template.replace("<%day%>", String(tm.Day));
	page_template.replace("<%month%>", String(tm.Month));
	page_template.replace("<%year%>", String(tmYearToCalendar(tm.Year)));

	send(200, "text/html", page_template);
}

void WeatherStationBase::setTime()
{
	if (hasArg("hour") && hasArg("minute") && hasArg("month") && hasArg("day") && hasArg("year"))
	{
		tmElements_t tm;

		//Serial.printf("setTime: Setting date/time to %s/%s/%s %s:%s\n", arg("month").c_str(), arg("day").c_str(), arg("year").c_str(), arg("hour").c_str(), arg("minute").c_str());

		tm.Hour = (uint8_t)arg("hour").toInt();
		tm.Minute = (uint8_t)arg("minute").toInt();
		tm.Second = 0;
		tm.Month = (uint8_t)arg("month").toInt();
		tm.Day = (uint8_t)arg("day").toInt();
		int year = arg("year").toInt();
		if (year < 100)
		{
			year += 2000;
		}
		tm.Year = year - 1970;

		time_t new_time = makeTime(tm);
		::setTime(new_time);
		m_last_display_data.time = new_time;
	}
	else
	{
		Serial.println("setTime: arguments missing");
	}

	sendHeader("Location", "/");
	send(302, "", "/");
}

void WeatherStationBase::handleWiFiConfig()
{
	String page_template = get_current_page_template();

	send(200, "text/html", page_template);
}


void WeatherStationBase::on_loop()
{
	update_local_sensor_data();
	update_time();
	handleClient();
}

void WeatherStationBase::update_display_timer_func(WeatherStationBase *ws_base)
{
	ws_base->update_display();
}
void WeatherStationBase::update_display(bool update_now)
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

void WeatherStationBase::update_local_sensor_data(bool update_now)
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

	if (m_current_local_sensor_data.humidity > 100)
	{
		Serial.println("Read bad humidity");
	}

	if (m_current_local_sensor_data.temperature > 1000)
	{
		Serial.println("Read bad temperature");
	}
}

void WeatherStationBase::update_time(bool update_now)
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

	::setTime(new_time);
}


void WeatherStationBase::draw_display()
{
	tmElements_t tm;
	breakTime(now(), tm);

	// TODO: Go to LCD.
	String display = m_last_display_data.get_date_string() + " " + m_last_display_data.get_time_string() + " In: " + m_last_display_data.get_local_sensor_string() + " Out: " + m_last_display_data.get_remote_sensor_string();
	Serial.println(display);
}
