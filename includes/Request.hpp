#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <map>

class Request
{
	private:
		std::string _method;
		std::string _uri;
		std::string _version;
		std::map<std::string, std::string> _headers;
		std::string _body;

	public:
		Request();
		~Request();
		
		void parse(const std::string& raw_request);
		
		std::string get_method() const;
		std::string get_uri() const;
		std::string get_version() const;
		std::string get_header(const std::string& key) const;
		std::string get_body() const;
		
		bool is_complete() const;
};

#endif
