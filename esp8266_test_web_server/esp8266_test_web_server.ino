#include <ESP8266WiFi.h>
#include <FS.h>
#include <TimeLib.h>

#include "OOWebServer.h"
#include "wifi_station.h"


class TestWebServer : public OOWebServer<TestWebServer>
{
	struct html_page_template
	{
		html_page_template(String uri, String template_data) :
			_next(NULL),
			_uri(uri),
			_template_data(template_data)
		{

		}
		html_page_template *_next;
		String _uri;
		String _template_data;
	};
public:
	TestWebServer(IPAddress addr, int port) : 
		OOWebServer(addr, port),
		_templates_base_dir("/web_templates"),
		_page_template_list(NULL)
	{
	}

	TestWebServer(int port) :
		OOWebServer(port),
		_templates_base_dir("/web_templates"),
		_page_template_list(NULL)
	{
	}

	void begin()
	{
		OOWebServer<TestWebServer>::begin();
		initHandlers();
	}

private:
	void initHandlers()
	{
		Serial.println("initHandlers");
		on("/", &TestWebServer::handleRoot);
		on("/time", HTTP_GET, &TestWebServer::handleTime);
		on("/time", HTTP_POST, &TestWebServer::setTime);
		on("/wifi_config", &TestWebServer::handleWiFiConfig);

		loadPageTemplates();
	}

	void loadPageTemplates()
	{
		Serial.println("Loading templates...");
		// Load HTML templates
		for (RequestHandler* p = _firstHandler; p != NULL; p = p->next())
		{
			String uri = p->uri();

			String templateName = "";
			if (uri == "/")
			{
				templateName = "/index";
			}
			else
			{
				templateName = uri;
			}

			templateName += ".html";

			LoadTemplate(uri, templateName);
		}
	}

	void LoadTemplate(const String & uri, const String & templateName)
	{
		String template_fil_name = _templates_base_dir + templateName;
		File template_file = SPIFFS.open(template_fil_name, "r");
		if (template_file)
		{
			Serial.print("Successfully opened file '");
			Serial.print(template_fil_name.c_str());
			Serial.println("'\n");
			size_t template_size = template_file.size();
			char *template_contents = new char[template_size + 1];
			if (template_contents)
			{
				template_file.readBytes(template_contents, template_size);
				template_contents[template_size] = 0;

				html_page_template *page_template = new html_page_template(uri, template_contents);
				if (page_template)
				{
					if (_page_template_list)
					{
						page_template->_next = _page_template_list;
					}

					_page_template_list = page_template;
				}
			}

			template_file.close();
		}
		else
		{
			Serial.print("Failed to open file '");
			Serial.print(template_fil_name.c_str());
			Serial.println("'\n");
		}
	}

	String get_current_page_template() const
	{
		return get_page_template(_currentUri);
	}

	String get_page_template(const String & uri) const
	{
		html_page_template *page_template = _page_template_list;

		for (html_page_template *page_template = _page_template_list; page_template != NULL; page_template = page_template->_next)
		{
			if (page_template->_uri == uri)
			{
				return page_template->_template_data;
			}
		}

		return "";
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

			tm.Hour = arg("hour").toInt();
			tm.Minute = arg("minute").toInt();
			tm.Second = 0;
			tm.Month = arg("month").toInt();
			tm.Day = arg("day").toInt();
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
	String _templates_base_dir;
	html_page_template *_page_template_list;
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
