#include "display_data.h"

#include <Time.h>
#include <TimeLib.h>


String pad_int(int val, int width, char fill);

String time_string(const tmElements_t & tm)
{
	return pad_int(tm.Hour, 2, ' ') + ":" + pad_int(tm.Minute, 2, '0');
}

String date_string(const tmElements_t & tm)
{
	return pad_int(tm.Month, 2, ' ') + "/" + pad_int(tm.Day, 2, ' ') + "/" + (tm.Year + 1970);
}

bool display_data::changed(const sensor_data & new_local_sensor_data, const sensor_data & new_remote_sensor_data, time_t new_time) const
{
	return !time_is_same(new_time) || new_local_sensor_data != local_data || new_remote_sensor_data != remote_data;
}

void display_data::update(const sensor_data & new_local_sensor_data, const sensor_data & new_remote_sensor_data, time_t new_time)
{
	time = new_time;
	local_data = new_local_sensor_data;
	remote_data = new_remote_sensor_data;
}

String display_data::get_local_sensor_string() const
{
	return local_data.to_string();
}

String display_data::get_remote_sensor_string() const
{
	return remote_data.to_string();
}

String display_data::get_time_string() const
{
	tmElements_t tm;
	breakTime(time, tm);

	return time_string(tm);
}

String display_data::get_date_string() const
{
	tmElements_t tm;
	breakTime(time, tm);

	return date_string(tm);
}

bool display_data::time_is_same(time_t new_time) const
{
	// Ignore seconds;
	return minute(time) == minute(new_time) &&
		hour(time) == hour(new_time) &&
		day(time) == day(new_time) &&
		month(time) == month(new_time) &&
		year(time) == year(new_time);
}

String pad_int(int val, int width, char fill)
{
	String s(val);
	if (s.length() < width)
	{
		String fill_str;
		for (int i = 0; i < width - s.length(); i++)
		{
			fill_str += fill;
		}

		return fill_str + s;
	}

	return s;
}
