#include <ESP8266WiFi.h>
#include <FS.h>
#include <TimeLib.h>

#include "OOWebServer.h"
#include "wifi_station.h"


class TestWebServer : public OOWebServer<TestWebServer>
{
public:
	TestWebServer(IPAddress addr, int port) : 
		OOWebServer("/web_templates", addr, port)
	{
	}

	TestWebServer(int port) :
		OOWebServer("/web_templates", port)
	{
	}

private:
    virtual void server_begin()
    {
		Serial.println("TestWebServer::impl_begin()");
		on("/", &TestWebServer::handleRoot);
		on("/time", HTTP_GET, &TestWebServer::handleTime);
        on("/time", HTTP_POST, &TestWebServer::setTime);
        on("/wifi_config", &TestWebServer::handleWiFiConfig);
	}

	String pad_string(const String & _str, size_t width, char fill_char)
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

	void handleRoot()
	{
		tmElements_t tm;
		time_t tNow = now();
		breakTime(tNow, tm);
    
		Serial.println("handleRoot");
		String page_template = get_current_page_template();

		page_template.replace("<%hour%>", String(hourFormat12(tNow)));
		page_template.replace("<%minute%>", pad_string(String(tm.Minute), 2, '0'));
		page_template.replace("<%meridian%>", isAM(tNow) ? "am" : "pm");

		page_template.replace("<%day_of_week%>", dayShortStr(tm.Wday));
		page_template.replace("<%day%>", String(tm.Day));
		page_template.replace("<%month%>", monthShortStr(tm.Month));
		page_template.replace("<%year%>", String(tmYearToCalendar(tm.Year)));

		page_template.replace("<%inside_temperature%>", "73");
		page_template.replace("<%inside_humidity%>", "43");

		page_template.replace("<%outside_temperature%>", "83");
		page_template.replace("<%outside_humidity%>", "53");

		send(200, "text/html", page_template);
	}

	void handleTime()
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

	void setTime()
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

			::setTime(makeTime(tm));
		}
		else
		{
			Serial.println("setTime: arguments missing");
		}

		sendHeader("Location", "/");
		send(302, "", "/");
	}

	void handleWiFiConfig()
	{
		String page_template = get_current_page_template();

		send(200, "text/html", page_template);
	}

private:
};

TestWebServer server(8080);



void setup()
{
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
	Serial.println("Sketch loaded...");

	// Connect to WiFi network
	connect_wifi("Caradhras", "Speak friend.", 120);

	SPIFFS.begin();
	server.begin();
}

void loop()
{
	server.handleClient();
}
