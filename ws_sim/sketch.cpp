#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "wifi_station.h"


ESP8266WebServer server(8080);


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

void handleRoot()
{
	String title;
	String body;
	String bgcolor;

	bgcolor = "#ffffff";
	title = "Web Server Example";
	body = "<h1>Web Server Example</h1>"
		"I wonder what you're going to click";

	server.send(200, "text/html", generate_page(title, bgcolor, body));
}

void handleRed()
{
	String title;
	String body;
	String bgcolor;

	bgcolor = "#ff4444";
	title = "You chose red";
	body = "<h1>Red</h1>";

	server.send(200, "text/html", generate_page(title, bgcolor, body));
}

void handleBlue()
{
	String title;
	String body;
	String bgcolor;

	bgcolor = "#4444ff";
	title = "You chose blue";
	body = "<h1>Blue</h1>";

	server.send(200, "text/html", generate_page(title, bgcolor, body));
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


	for (size_t i = 0; i < server.args(); i++)
	{
		body += "<br>" + server.argName(i) + " = " + server.arg(i);
	}


	body += "<hr>";

	server.send(200, "text/html", generate_page(title, bgcolor, body));
}

void setup()
{
	Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable

						   // Connect to WiFi network
	connect_wifi("Caradhras", "Speak friend.", 120);

	server.on("/", handleRoot);
	server.on("/red", handleRed);
	server.on("/blue", handleBlue);
	server.on("/form", handleForm);

	server.begin();
}

void loop()
{
	server.handleClient();
}
