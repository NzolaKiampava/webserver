#include "../includes/Server.hpp"
#include "../includes/Config.hpp"
#include <iostream>

Server::Server(const std::string& configFile) : _socket(-1), _port(8080), _host("localhost")
{
	_config.parse_config(configFile);
	_port = _config.get_port();
	_host = _config.get_host();
}

Server::~Server()
{
	close_server();
}

int Server::get_socket() const
{
	return _socket;
}

int Server::get_port() const
{
	return _port;
}

void Server::run()
{
	std::cout << "Server running on " << _host << ":" << _port << std::endl;
	// TODO: Implement server loop with select/poll/epoll
}

void Server::accept_connection()
{
	// TODO: Accept incoming connections
}

void Server::handle_client(Client& client)
{
	// TODO: Handle client requests
}

void Server::close_server()
{
	if (_socket != -1)
	{
		close(_socket);
		_socket = -1;
	}
}
