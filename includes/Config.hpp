/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:56:59 by nkiampav          #+#    #+#             */
/*   Updated: 2026/01/08 19:56:59 by nkiampav         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

class Config
{
	private:
		int _port;
		std::string _host;
		std::string _root;
		std::vector<std::string> _index_files;
		std::map<int, std::string> _error_pages;
		bool _autoindex;

	public:
		Config();
		Config(const std::string& configFile);
		~Config();
		
		void parse_config(const std::string& configFile);
		
		int get_port() const;
		std::string get_host() const;
		std::string get_root() const;
		std::vector<std::string> get_index_files() const;
		std::string get_error_page(int code) const;
		bool is_autoindex_enabled() const;
};

#endif
