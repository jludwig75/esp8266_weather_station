#pragma once

#include <WString.h>


struct sensor_data
{
	sensor_data();
	int temperature;
	int humidity;
	bool operator==(const sensor_data & other) const;
	bool operator!=(const sensor_data & other) const;
	String to_string() const;
};
