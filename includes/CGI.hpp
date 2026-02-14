/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:57:22 by nkiampav          #+#    #+#             */
/*   Updated: 2026/02/03 08:42:41 by nkiampav         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "Request.hpp"

class CGI
{
	private:
		std::string _script_path;
		std::string _interpreter_path;  // /usr/bin/python3, /usr/bin/php
		std::map<std::string, std::string> _env;
		std::string _query_string;
		std::string _request_body;
		int _timeout;  // Timeout em segundos
		
		// Métodos privados
		void _setup_standard_env();
		void _setup_request_env(const Request& request);
		char** _create_env_array() const;
		void _free_env_array(char** env) const;
		std::string _read_from_fd(int fd, int timeout) const;
		bool _wait_with_timeout(pid_t pid, int timeout, int& status) const;

	public:
		CGI(const std::string& script_path, const Request& request);
		CGI(const std::string& script_path, const std::string& interpreter, const Request& request);
		~CGI();
		
		void setup_environment(const Request& request);
		void set_interpreter(const std::string& interpreter_path);
		void set_timeout(int seconds);
		
		std::string execute();
		std::string execute_with_body(const std::string& body);
		
		bool is_valid_script(const std::string& path) const;
		std::string get_interpreter_for_extension(const std::string& extension) const;
		
		// Dechunking para CGI (se request veio chunked)
		static std::string dechunk_body(const std::string& chunked_body);
};

#endif
