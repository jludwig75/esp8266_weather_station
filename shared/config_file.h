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

private:
	String _file_name;
	String _host_ap;
	String _host_ap_passwd;
};
