#pragma once

// Data shared between client and server:
const char *ap_ssid = "JRL_WS_0";
const char *ap_password = "MyWeather1234";

#define TEMP_REPORT_SERVER_LISTEN_PORT  8080
const char *report_url = "/report_sensor_data";
const char *temp_var_name = "temp";
const char *humidity_var_name = "humidity";

const char *k_default_tz_std_name = "MST";
const char *k_default_tz_dst_name = "MDT";
const int k_default_std_tz_offset = -7 * 60;
const int k_default_dst_tz_offset = -6 * 60;