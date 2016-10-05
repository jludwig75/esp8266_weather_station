#include <ESP8266WiFi.h>

#include "OOWebServer.h"
#include "wifi_station.h"



class TestWebServer : public OOWebServer<TestWebServer>
{
public:
	TestWebServer(IPAddress addr, int port) : OOWebServer(addr, port)
	{
		initHandlers();
	}

	TestWebServer(int port) : OOWebServer(port)
	{
		initHandlers();
	}

private:
	void initHandlers()
	{
		on("/", &TestWebServer::handleRoot);
		on("/red", &TestWebServer::handleRed);
		on("/blue", &TestWebServer::handleBlue);
		on("/form", &TestWebServer::handleForm);
	}

	void handleRoot()
	{
		String title;
		String body;
		String bgcolor;

		bgcolor = "#ffffff";
		title = "Web Server Example";
		body = "<h1>Web Server Example</h1>"
			"I wonder what you're going to click";

		send(200, "text/html", generate_page(title, bgcolor, body));
	}

	void handleRed()
	{
		String title;
		String body;
		String bgcolor;

		bgcolor = "#ff4444";
		title = "You chose red";
		body = "<h1>Red</h1>";

		send(200, "text/html", generate_page(title, bgcolor, body));
	}

	void handleBlue()
	{
		String title;
		String body;
		String bgcolor;

		bgcolor = "#4444ff";
		title = "You chose blue";
		body = "<h1>Blue</h1>";

		send(200, "text/html", generate_page(title, bgcolor, body));
	}

	void handleForm()
	{
		String title;
		String body;
		String bgcolor;

		title = "Fill a form";

		body = "<h1>Fill a form</h1>";
		body += "<form action='/form'>"
			"<table>"
			"<tr><td>Field 1</td><td><input name=field_1></td></tr>"
			"<tr><td>Field 2</td><td><input name=field_2></td></tr>"
			"<tr><td>Field 3</td><td><input name=field_3></td></tr>"
			"</table>"
			"<input type=submit></form>";


		for (size_t i = 0; i < args(); i++)
		{
			body += "<br>" + argName(i) + " = " + arg(i);
		}


		body += "<hr>";

		send(200, "text/html", generate_page(title, bgcolor, body));
	}

	String generate_page(String title, String bgcolor, String body)
	{
		String links =
			"<p><a href='/red'>red</a> "
			"<br><a href='/blue'>blue</a> "
			"<br><a href='/form'>form</a> "
			"<br><a href='/auth'>authentication example</a> [use <b>adp</b> as username and <b>gmbh</b> as password"
			"<br><a href='/header'>show some HTTP header details</a> "
			;

		String response;
		response = "<html><head><title>";
		response += title;
		response += "</title></head><body bgcolor='" + bgcolor + "'>";
		response += body;
		response += links;
		response += "</body></html>";

		return response;
	}
};

TestWebServer server(8080);



void setup()
{
	Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable

	// Connect to WiFi network
	connect_wifi("Caradhras", "Speak friend.", 120);

	server.begin();
}

void loop()
{
	server.handleClient();
}
