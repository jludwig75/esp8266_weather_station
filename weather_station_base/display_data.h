#pragma once

#include "sensor_data.h"


struct display_data
{
	sensor_data local_data;
	sensor_data remote_data;
	time_t time;
	bool changed(const sensor_data & new_local_sensor_data, const sensor_data & new_remote_sensor_data, time_t new_time) const;
	void update(const sensor_data & new_local_sensor_data, const sensor_data & new_remote_sensor_data, time_t new_time);
	String get_local_sensor_string() const;
	String get_remote_sensor_string() const;
	String get_time_string() const;
	String get_date_string() const;
private:
	bool time_is_same(time_t new_time) const;
};
