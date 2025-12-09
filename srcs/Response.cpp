#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include <fstream>
#include <sstream>

Response::Response() : _status_code(200)
{
	_status_message = "OK";
}

Response::~Response()
{
}

void Response::generate(const Request& request, const std::string& root)
{
	std::string uri = request.get_uri();
	std::string file_path = root + uri;
	
	// TODO: Generate appropriate response based on request
	set_status(200);
	set_header("Content-Type", "text/html");
	set_header("Content-Length", "0");
}

void Response::set_status(int code)
{
	_status_code = code;
	_status_message = get_status_message(code);
}

void Response::set_header(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void Response::set_body(const std::string& body)
{
	_body = body;
}

std::string Response::get_response() const
{
	std::ostringstream oss;
	
	oss << "HTTP/1.1 " << _status_code << " " << _status_message << "\r\n";
	
	for (const auto& header : _headers)
		oss << header.first << ": " << header.second << "\r\n";
	
	oss << "\r\n" << _body;
	
	return oss.str();
}

std::string Response::get_status_message(int code) const
{
	switch (code)
	{
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 500: return "Internal Server Error";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		default: return "Unknown";
	}
}
