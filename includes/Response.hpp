/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:55:13 by nkiampav          #+#    #+#             */
/*   Updated: 2026/02/14 16:20:50 by nkiampav         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "Request.hpp"

class Response
{
	private:
		int _status_code;
		std::string _status_message;
		std::map<std::string, std::string> _headers;
		std::string _body;
		size_t _max_body_size;
		std::map<int, std::string> _error_pages;  // Páginas de erro customizadas
		std::string _root;  // Root directory para resolver caminhos
		std::vector<std::string> _index_files;  // Arquivos index (index.html, index.php, etc.)
		
		// Métodos privados auxiliares
		void _handle_get(const Request& request, const std::string& root);
		void _handle_post(const Request& request, const std::string& root);
		void _handle_delete(const Request& request, const std::string& root);
		void _serve_file(const std::string& filepath);
		void _serve_directory_listing(const std::string& dirpath, const std::string& uri);
		void _serve_error_page(int code);
		std::string _generate_directory_html(const std::string& dirpath, const std::string& uri);
		
		// Upload de arquivos
		bool _handle_file_upload(const Request& request, const std::string& upload_dir);
		std::string _parse_multipart_filename(const std::string& content_disposition);
		std::string _extract_file_content(const std::string& body, const std::string& boundary);

	public:
		Response();
		~Response();
		
		void generate(const Request& request, const std::string& root);
		void generate_with_config(const Request& request, const std::string& root, size_t max_body_size);
		void set_status(int code);
		void set_header(const std::string& key, const std::string& value);
		void set_body(const std::string& body);
		void set_max_body_size(size_t size);
		void set_error_pages(const std::map<int, std::string>& error_pages);
		void set_root(const std::string& root);
		void set_index_files(const std::vector<std::string>& index_files);
		
		std::string get_response() const;
		std::string get_status_message(int code) const;
		int get_status_code() const;

		bool _process_multipart_upload(const std::string& body, const std::string& content_type, const std::string& upload_dir);
};

#endif
