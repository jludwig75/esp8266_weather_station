#pragma once

#include <ESP8266WiFi.h>


bool connect_wifi(const char *ap_ssid, const char *ap_password, int retries);

bool disconnect_wifi();

bool is_wifi_connected();

IPAddress get_local_ip();
