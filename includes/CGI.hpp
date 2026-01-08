/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:57:22 by nkiampav          #+#    #+#             */
/*   Updated: 2026/01/08 19:57:22 by nkiampav         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <string>
#include <map>
#include "Request.hpp"

class CGI
{
	private:
		std::string _script_path;
		std::map<std::string, std::string> _env;

	public:
		CGI(const std::string& script_path, const Request& request);
		~CGI();
		
		void setup_environment(const Request& request);
		std::string execute();
		
		bool is_valid_script(const std::string& path) const;
};

#endif
