#include "config_file.h"

#include "FS.h"


#include <ArduinoJson.h>

#ifdef WIN32
#define	JSON_NS	ArduinoJson
#else
#define	JSON_NS
#endif // WIN32

ConfigFile::ConfigFile(const String & file_name) : _file_name(file_name)
{
}

bool ConfigFile::Load()
{
	File config_file = SPIFFS.open(_file_name, "r");
	if (!config_file)
	{
		Serial.println("Failed to open config file");
		return false;
	}

	size_t size = config_file.size();
	if (size > 1024) {
		Serial.println("Config file size is too large");
		return false;
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	config_file.readBytes(buf.get(), size);

	ArduinoJson::StaticJsonBuffer<400> jsonBuffer;
	ArduinoJson::JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
		Serial.println("Failed to parse config file");
		return false;
	}

	_host_ap = json["host_ap"].asString();
	_host_ap_passwd = json["host_ap_passwd"].asString();
	_std_string = json["std_string"].asString();
	_dst_string = json["dst_string"].asString();
	_std_offset = String(json["std_offset"].asString()).toInt();
	_dst_offset = String(json["dst_offset"].asString()).toInt();

	return true;
}

bool ConfigFile::Save()
{
	ArduinoJson::StaticJsonBuffer<400> jsonBuffer;
	ArduinoJson::JsonObject& json = jsonBuffer.createObject();
	json["host_ap"] = _host_ap.c_str();
	json["host_ap_passwd"] = _host_ap_passwd.c_str();
	json["std_string"] = _std_string.c_str();
	json["dst_string"] = _dst_string.c_str();
	String std_offset = String(_std_offset);
	String dst_offset = String(_dst_offset);
	json["std_offset"] = std_offset.c_str();
	json["dst_offset"] = dst_offset.c_str();

	File config_file = SPIFFS.open(_file_name, "w");
	if (!config_file) {
		Serial.println("Failed to open config file for writing");
		return false;
	}

	JSON_NS::String str;

	json.printTo(str);

	return config_file.write((const uint8_t *)str.c_str(), str.length()) == str.length();
}
