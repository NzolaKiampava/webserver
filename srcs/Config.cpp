#include "../includes/Config.hpp"
#include <fstream>
#include <sstream>

Config::Config() : _port(8080), _host("localhost"), _autoindex(false)
{
}

Config::Config(const std::string& configFile) : _port(8080), _host("localhost"), _autoindex(false)
{
	parse_config(configFile);
}

Config::~Config()
{
}

void Config::parse_config(const std::string& configFile)
{
	std::ifstream file(configFile);
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open config file: " << configFile << std::endl;
		return;
	}
	
	std::string line;
	while (std::getline(file, line))
	{
		// TODO: Parse configuration file
	}
	
	file.close();
}

int Config::get_port() const
{
	return _port;
}

std::string Config::get_host() const
{
	return _host;
}

std::string Config::get_root() const
{
	return _root;
}

std::vector<std::string> Config::get_index_files() const
{
	return _index_files;
}

std::string Config::get_error_page(int code) const
{
	auto it = _error_pages.find(code);
	if (it != _error_pages.end())
		return it->second;
	return "";
}

bool Config::is_autoindex_enabled() const
{
	return _autoindex;
}
