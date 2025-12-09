#include "../includes/Client.hpp"
#include <unistd.h>
#include <cstring>

Client::Client(int socket, const std::string& ip) : _socket(socket), _ip_address(ip)
{
}

Client::~Client()
{
	if (_socket != -1)
		close(_socket);
}

void Client::receive_data()
{
	char buffer[4096];
	ssize_t bytes = recv(_socket, buffer, sizeof(buffer), 0);
	
	if (bytes > 0)
		_buffer.append(buffer, bytes);
	
	// TODO: Parse request from buffer
}

void Client::send_response()
{
	std::string response = _response.get_response();
	send(_socket, response.c_str(), response.size(), 0);
}

int Client::get_socket() const
{
	return _socket;
}

std::string Client::get_ip() const
{
	return _ip_address;
}

Request& Client::get_request()
{
	return _request;
}

Response& Client::get_response()
{
	return _response;
}

std::string& Client::get_buffer()
{
	return _buffer;
}

bool Client::is_request_complete() const
{
	return _request.is_complete();
}
