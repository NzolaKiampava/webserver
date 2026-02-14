/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:56:59 by nkiampav          #+#    #+#             */
/*   Updated: 2026/02/14 15:02:20 by nkiampav         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

// Estrutura para configuração de Location (rotas específicas)
struct LocationConfig
{
	std::string path;                          // Ex: "/uploads", "/cgi-bin"
	std::vector<std::string> allowed_methods;  // GET, POST, DELETE
	std::string root;                          // Document root específico
	bool autoindex;                            // Directory listing
	std::string redirect;                      // URL para redirect
	std::vector<std::string> index_files;      // index.html, index.php
	bool cgi_enabled;                          // CGI habilitado
	std::string cgi_path;                      // /usr/bin/python3, /usr/bin/php
	std::string cgi_extension;                 // .py, .php
	std::string upload_dir;                    // Diretório para uploads
	
	LocationConfig() : autoindex(false), cgi_enabled(false) {}
};

// Estrutura para configuração de Server (virtual server)
struct ServerConfig
{
	int port;                                  // Porta (8080, 80, etc.)
	std::string host;                          // localhost, 0.0.0.0
	std::string server_name;                   // Nome do servidor
	std::string root;                          // Document root
	std::vector<std::string> index_files;      // Arquivos índice padrão
	std::map<int, std::string> error_pages;    // Páginas de erro customizadas
	bool autoindex;                            // Directory listing global
	size_t client_max_body_size;               // Tamanho máximo do body (bytes)
	int timeout;                               // Timeout de requisições (segundos)
	int cgi_timeout;                           // Timeout de CGI (segundos)
	std::vector<LocationConfig> locations;     // Configurações de rotas
	
	ServerConfig() : port(8080), host("localhost"), autoindex(false), 
	                 client_max_body_size(100 * 1024 * 1024), // 100MB
	                 timeout(30), cgi_timeout(5) {}
};

class Config
{
	private:
		std::vector<ServerConfig> _servers;
		std::string _config_file;
		
		// Métodos privados de parsing
		void _parse_server_block(const std::string& block);
		void _parse_location_block(const std::string& block, ServerConfig& server);
		std::string _trim(const std::string& str) const;
		std::vector<std::string> _split(const std::string& str, char delimiter) const;
		std::string _extract_block(const std::string& content, size_t& pos) const;
		size_t _parse_size(const std::string& size_str) const;

	public:
		Config();
		Config(const std::string& configFile);
		~Config();
		
		void parse_config(const std::string& configFile);
		void validate_config() const;
		
		// Getters para múltiplos servers
		std::vector<ServerConfig> get_servers() const;
		ServerConfig get_server(size_t index) const;
		size_t get_server_count() const;
		
		// Getters para configurações específicas (usa primeiro servidor por padrão)
		int get_port() const;
		std::string get_host() const;
		std::string get_root() const;
		std::vector<std::string> get_index_files() const;
		std::string get_error_page(int code) const;
		bool is_autoindex_enabled() const;
		size_t get_max_body_size() const;
		int get_timeout() const;
		int get_cgi_timeout() const;
		
		// Busca de location mais específico para uma URI
		LocationConfig* find_location(const std::string& uri, size_t server_index = 0);
		const LocationConfig* find_location(const std::string& uri, size_t server_index = 0) const;
		
		// Verificação de métodos permitidos
		bool is_method_allowed(const std::string& method, const std::string& uri, size_t server_index = 0) const;
};

#endif
