#include "weather_station_base_class.h"

#include "wifi_ap.h"
#include "wifi_station.h"

#include "ws_common.h"
#include "config_file.h"

#include <Timezone.h>
#include "DS1307RTC.h"  // a basic DS1307 library that returns time as a time_t

#include <RtcDS3231.h>

#define CONFIG_FILE_NAME    "/ws_config.json"


#define DISPLAY_UPDATE_INTERVAL_MS      333               // 0.333 seconds
#define LOCAL_SENSOR_UPDATE_INTERVAL_MS ( 1 * 60 * 1000)  // 1 minute
#define UPDATE_TIME_UPDATE_INTERVAL_MS  (15 * 60 * 1000)  // 15 minutes
#define UDP_LISTEN_PORT                 8888


const char* k_default_host_ssid = "ACESS_POINT";
const char* k_default_host_password = "Password123";


#define TFT_DC 16
#define TFT_CS 0


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
WeatherStationBase::WeatherStationBase(uint8_t dht_pin, uint8_t dht_type) :
	OOWebServer<WeatherStationBase>("/web_templates", TEMP_REPORT_SERVER_LISTEN_PORT),
	m_wifi_connected(false),
	m_dht(dht_pin, dht_type, 11), // 11 works fine for ESP8266
	m_ntp_client(),
	m_last_time_update(millis()),
	m_last_local_sensor_update(millis()),
	m_host_ssid(k_default_host_ssid),
	m_host_password(k_default_host_password),
	m_std_string(k_default_tz_std_name),
	m_dst_string(k_default_tz_dst_name),
	m_std_offset(k_default_std_tz_offset),
	m_dst_offset(k_default_dst_tz_offset),
	m_tz(NULL),
	m_display(TFT_CS, TFT_DC),
	m_rtc(NULL),
	m_backlight_level(511),
	m_last_light_level(0),
	m_boost_backlight(false)
{
}

WeatherStationBase::~WeatherStationBase()
{
	delete m_rtc;
	delete m_tz;
}


// setup() methods
void WeatherStationBase::server_begin()
{
	SPIFFS.begin();

	m_display.begin();

	pinMode(12, OUTPUT);
	pinMode(10, INPUT);
	set_backlight_level(m_backlight_level);

	m_rtc = new RtcDS3231;

	// never assume the Rtc was last configured by you, so
	// just clear them to your needed state
	m_rtc->Enable32kHzPin(false);
	m_rtc->SetSquareWavePin(DS3231SquareWavePin_ModeNone);

	m_rtc->Begin();
	if (!m_rtc->GetIsRunning())
	{
		Serial.println("RTC was not actively running, starting now");
		m_rtc->SetIsRunning(true);
		if (m_rtc->GetIsRunning())
		{
			Serial.println("RTC is running");
		}
		else
		{
			Serial.println("RTC is still not running");
		}
	}
	else
	{
		Serial.println("RTC is running");
	}

	if (!m_rtc->IsDateTimeValid())
	{
		Serial.println("RTC time not valid");
	}
	else
	{
		Serial.println("RTC time is valid");
		RtcDateTime now = m_rtc->GetDateTime();
		Serial.printf("RTC time = %lu\n", now.Epoch32Time());
	}

	if (m_rtc->IsDateTimeValid())
	{
		::setTime(m_rtc->GetDateTime().Epoch32Time());
	}

	load_config();

	Serial.print("TZ Info: ");
	Serial.print(m_std_string);
	Serial.print(" ");
	Serial.print(m_std_offset);
	Serial.print(", ");
	Serial.print(m_dst_string);
	Serial.print(" ");
	Serial.println(m_dst_offset);

	TimeChangeRule myDST = { "", Second, Sun, Mar, 2, m_dst_offset };    //Daylight time = UTC - 6 hours
	TimeChangeRule mySTD = { "", First, Sun, Nov, 2, m_std_offset };     //Standard time = UTC - 7 hours
	strncpy(myDST.abbrev, m_dst_string.c_str(), sizeof(myDST.abbrev));
	strncpy(mySTD.abbrev, m_std_string.c_str(), sizeof(mySTD.abbrev));
	m_tz = new Timezone(myDST, mySTD);

	m_dht.begin();
	if (!start_access_point(ap_ssid, ap_password))
	{
		Serial.println("Failed to start access point! Resetting processor...");
		reset();
	}

	m_wifi_connected = connect_wifi(m_host_ssid.c_str(), m_host_password.c_str(), 20); // About 10 seconds
	if (!m_wifi_connected)
	{
		Serial.println("Failed to start WiFi! Continuing without WiFi");
	}

	Serial.println("Starting web server...\n");
	on(report_url, HTTP_POST, &WeatherStationBase::handle_sensor_data_post);
	on("/", &WeatherStationBase::handle_root);
	on("/time", HTTP_GET, &WeatherStationBase::handleTime);
	on("/time", HTTP_POST, &WeatherStationBase::setTime);
	on("/time_zone", HTTP_POST, &WeatherStationBase::setTimeZone);
	on("/wifi_config", HTTP_GET, &WeatherStationBase::handleWiFiConfig);
    on("/wifi_config", HTTP_POST, &WeatherStationBase::setWiFiConfig);

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


bool WeatherStationBase::load_config()
{
    ConfigFile config_file(CONFIG_FILE_NAME);

    if (!config_file.Load()) {
        return false;
    }

    m_host_ssid = config_file.get_host_ap();
    m_host_password = config_file.get_host_ap_passwd();

	if (config_file.get_std_string().length() > 0 && config_file.get_dst_string().length() > 0)
	{
		m_dst_string = config_file.get_dst_string();
		m_dst_offset = config_file.get_dst_offset();
		m_std_string = config_file.get_std_string();
		m_std_offset = config_file.get_std_offset();
	}

	return true;
}

bool WeatherStationBase::save_wifi_config(const String & ap, const String & ap_passwd)
{
	return save_config(ap, ap_passwd, m_std_string, m_std_offset, m_dst_string, m_dst_offset);
}

bool WeatherStationBase::save_tz_config(const String & std_string, int std_offset, const String & dst_string, int dst_offset)
{
	return save_config(m_host_ssid, m_host_password, std_string, std_offset, dst_string, dst_offset);
}

bool WeatherStationBase::save_config(const String & ap, const String & ap_passwd, const String & std_string, int std_offset, const String & dst_string, int dst_offset)
{
    ConfigFile config_file(CONFIG_FILE_NAME);

    config_file.set_host_ap(ap);
    config_file.set_host_ap_passwd(ap_passwd);
	config_file.set_std_string(std_string);
	config_file.set_std_offset(std_offset);
	config_file.set_dst_string(dst_string);
	config_file.set_dst_offset(dst_offset);

    if (!config_file.Save())
    {
        return false;
    }

    m_host_ssid = ap;
    m_host_password = ap_passwd;
	m_std_string = std_string;
	m_std_offset = std_offset;
	m_dst_string = dst_string;
	m_dst_offset = dst_offset;
	return true;
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

String IPAddressToString(const IPAddress & ip_address)
{
    String str;

    str = String(ip_address[0]) + "." + ip_address[1] + "." + ip_address[2] + "." + ip_address[3];

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

	page_template.replace("<%backlight_level%>", String(m_backlight_level));
	page_template.replace("<%light_sensor%>", String(analogRead(A0)));

    String local_ip_string = "Not connected";
    if (is_wifi_connected())
    {
        local_ip_string = IPAddressToString(get_local_ip());
    }
    page_template.replace("<%host_ip%>", local_ip_string);

    render_page(page_template);
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
	time_t tNow = m_tz->toLocal(now());
	breakTime(tNow, tm);

	String page_template = get_current_page_template();

	page_template.replace("<%hour%>", String(tm.Hour));
	page_template.replace("<%minute%>", String(tm.Minute));

	page_template.replace("<%day%>", String(tm.Day));
	page_template.replace("<%month%>", String(tm.Month));
	page_template.replace("<%year%>", String(tmYearToCalendar(tm.Year)));

    render_page(page_template);
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
        time_t utc_time = m_tz->toUTC(new_time);

		RtcDateTime dt_new;
		dt_new.InitWithEpoch32Time(utc_time);
		m_rtc->SetDateTime(dt_new);

        ::setTime(utc_time);
		m_last_display_data.time = m_tz->toLocal(new_time);
	}
	else
	{
		Serial.println("setTime: arguments missing");
	}

    redirect_to("/");
}

void WeatherStationBase::setTimeZone()
{
	if (hasArg("time_zone"))
	{
		String tz_string = arg("time_zone");
		bool use_dst = tz_string.endsWith("d");
		if (use_dst)
		{
			tz_string = tz_string.substring(0, tz_string.length() - 1);
		}

		float offset = tz_string.toFloat();
		offset *= SECS_PER_MIN;

		int std_offset = (int)offset;
		int dst_offset = (int)offset;

		String std_string = "?ST";
		String dst_string = "?ST";

		if (use_dst)
		{
			dst_offset += SECS_PER_MIN;
			dst_string[1] = 'D';
		}

		// Save the TZ information:
		if (!save_tz_config(std_string, std_offset, dst_string, dst_offset))
		{
			// TODO: error
		}

		delete m_tz;
		TimeChangeRule myDST = { "", Second, Sun, Mar, 2, m_dst_offset };    //Daylight time = UTC - 6 hours
		TimeChangeRule mySTD = { "", First, Sun, Nov, 2, m_std_offset };     //Standard time = UTC - 7 hours
		strncpy(myDST.abbrev, m_dst_string.c_str(), sizeof(myDST.abbrev));
		strncpy(mySTD.abbrev, m_std_string.c_str(), sizeof(mySTD.abbrev));
		m_tz = new Timezone(myDST, mySTD);

		m_last_display_data.time = m_tz->toLocal(now());
	}
	else
	{
		Serial.println("setTimeZone: arguments missing");
	}

	redirect_to("/");
}

void WeatherStationBase::handleWiFiConfig()
{
	String page_template = get_current_page_template();

    page_template.replace("<%ap%>", m_host_ssid);
    page_template.replace("<%ap_passwd%>", m_host_password);

    render_page(page_template);
}


void WeatherStationBase::setWiFiConfig()
{
    if (!hasArg("ap") || !hasArg("ap_passwd")) {
        Serial.println("setTime: arguments missing");
    }
    else
    {
        if (!save_wifi_config(arg("ap"), arg("ap_passwd")))
        {
            // TODO: Error
        }

        if (m_wifi_connected)
        {
            disconnect_wifi();
            m_wifi_connected = false;
        }

        if (!connect_wifi(m_host_ssid.c_str(), m_host_password.c_str(), 20))
        {
            // TODO: Error
        }

		update_time(true);
    }

    redirect_to("/");
}


void WeatherStationBase::on_loop()
{
	update_local_sensor_data();
	update_time();
	handleClient();
	handle_backlight_button();
}

void WeatherStationBase::update_display_timer_func(WeatherStationBase *ws_base)
{
	ws_base->update_display();
}

void WeatherStationBase::update_display(bool update_now)
{
	adjust_back_light();

	// Get a snapshot of the time and sensor data.
	time_t current_time = m_tz->toLocal(now());
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
		m_wifi_connected = connect_wifi(m_host_ssid.c_str(), m_host_password.c_str(), 20); // About 10 seconds
		if (!m_wifi_connected)
		{
			Serial.println("Failed to start WiFi! Skipping time update.");
			return;
		}
	}
	// Get the time via NTP
	time_t new_time = m_ntp_client.get_time();
	if (new_time == 0)
	{
		return;
	}

	// Update the time
	RtcDateTime now_dt;
	now_dt.InitWithEpoch32Time(new_time);
	Serial.printf("Setting RTC time to %lu\n", new_time);
	m_rtc->SetDateTime(now_dt);

	::setTime(new_time);
}


void WeatherStationBase::draw_display()
{
	tmElements_t tm;
	breakTime(m_last_display_data.time, tm);

    String time_string = String(hourFormat12(m_last_display_data.time)) + ":" + pad_string(String(tm.Minute), 2, '0');
    String time_meridian = isAM(m_last_display_data.time) ? "am" : "pm";

    String date_string = String(dayShortStr(tm.Wday)) + ", " + monthShortStr(tm.Month) + " " + String(tm.Day) + ", " + String(tmYearToCalendar(tm.Year));

	// Draw on LCD.
    m_display.update_time_string(time_string);
    m_display.update_time_meridian(time_meridian);
    m_display.update_date_string(date_string);
    m_display.update_inside_temp_string(m_last_display_data.get_local_sensor_string());
    m_display.update_outside_temp_string(m_last_display_data.get_remote_sensor_string());

	String display = m_last_display_data.get_date_string() + " " + m_last_display_data.get_time_string() + " In: " + m_last_display_data.get_local_sensor_string() + " Out: " + m_last_display_data.get_remote_sensor_string();
	Serial.println(display);
}

void WeatherStationBase::set_backlight_level(int level)
{
	if (level > 1023)
	{
		level = 1023;
	}
	if (level < 0)
	{
		level = 0;
	}

	m_backlight_level = level;

	// Using PNP resistor so logic will be inverted.
	analogWrite(12, 1023 - m_backlight_level);
}

void WeatherStationBase::adjust_back_light(bool force)
{
	int light_level = analogRead(A0);
	if (abs(light_level - m_last_light_level) > 10 || force)
	{
		int new_backlight_level = map(light_level, 30, 900, 4, 200);

		if (m_boost_backlight)
		{
			Serial.println ("Boosting backlight level");
			new_backlight_level += 128;
		}

		set_backlight_level(new_backlight_level);
		m_last_light_level = light_level;
	}
}

void WeatherStationBase::handle_backlight_button()
{
	if (digitalRead(10) == LOW)
	{
		if (!m_boost_backlight)
		{
			Serial.println("Button pressed.");
			m_boost_backlight = true;
			adjust_back_light(true);
		}
	}
	else
	{
		if (m_boost_backlight)
		{
			Serial.println("Button released.");
			m_boost_backlight = false;
			adjust_back_light(true);
		}
	}
}
