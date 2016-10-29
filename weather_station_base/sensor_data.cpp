#include "sensor_data.h"


sensor_data::sensor_data() : temperature(0xffff), humidity(0xffff)
{

}

bool sensor_data::operator==(const sensor_data & other) const
{
	return temperature == other.temperature && humidity == other.humidity;
}

bool sensor_data::operator!=(const sensor_data & other) const
{
	return !operator==(other);
}

String sensor_data::to_string() const
{
	String temp_string = "--";
	String humidity_string = "--";
	if (temperature < 255)
	{
		temp_string = String(temperature);
	}
	if (humidity < 255)
	{
		humidity_string = String(humidity);
	}
	return temp_string + "\xA7" "F " + humidity_string + "%";
}
