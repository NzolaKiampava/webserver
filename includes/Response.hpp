#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include "Request.hpp"

class Response
{
	private:
		int _status_code;
		std::string _status_message;
		std::map<std::string, std::string> _headers;
		std::string _body;

	public:
		Response();
		~Response();
		
		void generate(const Request& request, const std::string& root);
		void set_status(int code);
		void set_header(const std::string& key, const std::string& value);
		void set_body(const std::string& body);
		
		std::string get_response() const;
		std::string get_status_message(int code) const;
};

#endif
