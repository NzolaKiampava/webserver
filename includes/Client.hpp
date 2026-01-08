/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:57:13 by nkiampav          #+#    #+#             */
/*   Updated: 2026/01/08 19:57:13 by nkiampav         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include "Request.hpp"
#include "Response.hpp"

class Client
{
	private:
		int _socket;
		std::string _ip_address;
		Request _request;
		Response _response;
		std::string _buffer;

	public:
		Client(int socket, const std::string& ip);
		~Client();
		
		void receive_data();
		void send_response();
		
		int get_socket() const;
		std::string get_ip() const;
		Request& get_request();
		Response& get_response();
		std::string& get_buffer();
		
		bool is_request_complete() const;
};

#endif
