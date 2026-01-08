/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:54:53 by nkiampav          #+#    #+#             */
/*   Updated: 2026/01/08 19:54:53 by nkiampav         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Client.hpp"
#include "Config.hpp"

class Server
{
	private:
		int _socket;
		int _port;
		std::string _host;
		std::vector<Client> _clients;
		Config _config;
		
		void _setup_socket();

	public:
		Server(const std::string& configFile);
		~Server();
		
		void run();
		void accept_connection();
		void handle_client(Client& client);
		void close_server();
		
		int get_socket() const;
		int get_port() const;
};

#endif
