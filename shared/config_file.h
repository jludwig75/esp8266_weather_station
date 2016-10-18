#pragma once


#include <WString.h>


class ConfigFile
{
public:
	ConfigFile(const String & file_name);

	bool Load();
	bool Save();

	String get_host_ap() const
	{
		return _host_ap;
	}
	void set_host_ap(const ::String & host_ap)
	{
		_host_ap = host_ap;
	}

	String get_host_ap_passwd() const
	{
		return _host_ap_passwd;
	}
	void set_host_ap_passwd(const String & host_ap_passwd)
	{
		_host_ap_passwd = host_ap_passwd;
	}

	String get_std_string() const
	{
		return _std_string;
	}
	void set_std_string(const String & value)
	{
		_std_string = value;
	}

	String get_dst_string() const
	{
		return _dst_string;
	}
	void set_dst_string(const String & value)
	{
		_dst_string = value;
	}

	int get_std_offset() const
	{
		return _std_offset;
	}
	void set_std_offset(int value)
	{
		_std_offset = value;
	}

	int get_dst_offset() const
	{
		return _dst_offset;
	}
	void set_dst_offset(int value)
	{
		_dst_offset = value;
	}

private:
	String _file_name;
	String _host_ap;
	String _host_ap_passwd;
	String _std_string;
	String _dst_string;
	int _std_offset;
	int _dst_offset;
};
