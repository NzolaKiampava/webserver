/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:54:53 by nkiampav          #+#    #+#             */
/*   Updated: 2026/01/24 11:35:19 by nkiampav         ###   ########.fr       */
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
		bool _running;
		
		void _setup_socket();
		void _set_nonblocking(int socket);


		void accept_connection();

		void handle_client_read(Client& client, size_t client_index);
		void handle_client_write(Client& client, size_t client_index);

		void _remove_client(size_t index);
		void _check_client_timeouts();

		// Deve ser implementado
		//Server(const Server& other);
		//Server& operator=(const Server& other);

	public:
		Server(const std::string& configFile);
		~Server();
		
		void run();
		void close_server();
		
		int get_socket() const;
		int get_port() const;
};

#endif
