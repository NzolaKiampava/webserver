#include "../includes/Request.hpp"
#include "../includes/Utils.hpp"

Request::Request()
{
}

Request::~Request()
{
}

void Request::parse(const std::string& raw_request)
{
	std::istringstream iss(raw_request);
	std::string line;
	
	// Parse request line
	if (std::getline(iss, line))
	{
		std::istringstream req_line(line);
		req_line >> _method >> _uri >> _version;
	}
	
	// Parse headers
	while (std::getline(iss, line))
	{
		if (line == "\r" || line.empty())
			break;
		
		size_t pos = line.find(':');
		if (pos != std::string::npos)
		{
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			_headers[key] = Utils::trim(value);
		}
	}
	
	// Parse body
	std::string body_line;
	while (std::getline(iss, body_line))
		_body += body_line + "\n";
}

std::string Request::get_method() const
{
	return _method;
}

std::string Request::get_uri() const
{
	return _uri;
}

std::string Request::get_version() const
{
	return _version;
}

std::string Request::get_header(const std::string& key) const
{
	auto it = _headers.find(key);
	if (it != _headers.end())
		return it->second;
	return "";
}

std::string Request::get_body() const
{
	return _body;
}

bool Request::is_complete() const
{
	return !_method.empty() && !_uri.empty();
}
