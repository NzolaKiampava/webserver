#include "Config.hpp"
#include "Utils.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

// Construtor padrão: Inicializa com valores padrão
Config::Config() : _config_file("")
{
	// Criar servidor padrão
	ServerConfig default_server;
	_servers.push_back(default_server);
}

// Construtor com arquivo de configuração
Config::Config(const std::string& configFile) : _config_file(configFile)
{
	parse_config(configFile);
}

// Destrutor
Config::~Config()
{
}

// Parse do arquivo de configuração
// - Abre e lê arquivo .conf
// - Identifica blocos server
// - Para cada server, faz parse de diretivas e locations
// - Popula as variáveis membro
// - Trata erros de parsing
void Config::parse_config(const std::string& configFile)
{
	std::ifstream file(configFile.c_str());
	if (!file.is_open())
		throw std::runtime_error("Failed to open config file: " + configFile);
	
	_config_file = configFile;
	_servers.clear();
	
	// Ler todo o arquivo
	std::ostringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	file.close();
	
	// Encontrar e processar blocos server
	size_t pos = 0;
	while (pos < content.length())
	{
		// Procurar "server {"
		size_t server_start = content.find("server", pos);
		if (server_start == std::string::npos)
			break;
		
		pos = server_start + 6;  // Pular "server"
		
		// Encontrar abertura do bloco
		size_t brace_start = content.find("{", pos);
		if (brace_start == std::string::npos)
			break;
		
		pos = brace_start + 1;
		
		// Extrair bloco server
		std::string server_block = _extract_block(content, pos);
		
		// Parse do bloco
		_parse_server_block(server_block);
	}
	
	// Se nenhum servidor foi encontrado, criar um padrão
	if (_servers.empty())
	{
		ServerConfig default_server;
		_servers.push_back(default_server);
	}
	
	validate_config();
}

// Extrai um bloco delimitado por chaves
std::string Config::_extract_block(const std::string& content, size_t& pos) const
{
	int brace_count = 1;
	size_t start = pos;
	
	while (pos < content.length() && brace_count > 0)
	{
		if (content[pos] == '{')
			brace_count++;
		else if (content[pos] == '}')
			brace_count--;
		pos++;
	}
	
	return content.substr(start, pos - start - 1);
}

// Parse de um bloco server
void Config::_parse_server_block(const std::string& block)
{
	ServerConfig server;
	
	std::istringstream stream(block);
	std::string line;
	
	while (std::getline(stream, line))
	{
		line = _trim(line);
		
		if (line.empty() || line[0] == '#')
			continue;
		
		// Verificar se é um bloco location
		if (line.find("location") == 0)
		{
			// Extrair bloco location
			size_t brace_start = line.find("{");
			if (brace_start == std::string::npos)
			{
				// Location em múltiplas linhas
				std::string location_block;
				std::string loc_line;
				int braces = 0;
				
				do {
					if (!std::getline(stream, loc_line))
						break;
					location_block += loc_line + "\n";
					for (size_t i = 0; i < loc_line.length(); i++)
					{
						if (loc_line[i] == '{') braces++;
						if (loc_line[i] == '}') braces--;
					}
				} while (braces > 0);
				
				_parse_location_block(line + "\n" + location_block, server);
			}
			else
			{
				_parse_location_block(line, server);
			}
			continue;
		}
		
		// Parse de diretivas simples
		std::vector<std::string> tokens = _split(line, ' ');
		if (tokens.empty())
			continue;
		
		std::string directive = tokens[0];
		
		if (directive == "listen" && tokens.size() >= 2)
		{
			server.port = atoi(tokens[1].c_str());
		}
		else if (directive == "server_name" && tokens.size() >= 2)
		{
			server.server_name = tokens[1];
			if (server.server_name[server.server_name.length() - 1] == ';')
				server.server_name.erase(server.server_name.length() - 1);
		}
		else if (directive == "host" && tokens.size() >= 2)
		{
			server.host = tokens[1];
			if (server.host[server.host.length() - 1] == ';')
				server.host.erase(server.host.length() - 1);
		}
		else if (directive == "root" && tokens.size() >= 2)
		{
			server.root = tokens[1];
			if (server.root[server.root.length() - 1] == ';')
				server.root.erase(server.root.length() - 1);
		}
		else if (directive == "index" && tokens.size() >= 2)
		{
			for (size_t i = 1; i < tokens.size(); i++)
			{
				std::string idx = tokens[i];
				if (idx[idx.length() - 1] == ';')
					idx.erase(idx.length() - 1);
				server.index_files.push_back(idx);
			}
		}
		else if (directive == "autoindex" && tokens.size() >= 2)
		{
			server.autoindex = (tokens[1] == "on" || tokens[1] == "on;");
		}
		else if (directive == "error_page" && tokens.size() >= 3)
		{
			int code = atoi(tokens[1].c_str());
			std::string path = tokens[2];
			if (path[path.length() - 1] == ';')
				path.erase(path.length() - 1);
			server.error_pages[code] = path;
		}
		else if (directive == "client_max_body_size" && tokens.size() >= 2)
		{
			server.client_max_body_size = _parse_size(tokens[1]);
		}
		else if (directive == "timeout" && tokens.size() >= 2)
		{
			server.timeout = atoi(tokens[1].c_str());
		}
		else if (directive == "cgi_timeout" && tokens.size() >= 2)
		{
			server.cgi_timeout = atoi(tokens[1].c_str());
		}
	}
	
	_servers.push_back(server);
}

// Parse de um bloco location
void Config::_parse_location_block(const std::string& block, ServerConfig& server)
{
	LocationConfig location;
	
	// Extrair o caminho da location
	size_t path_start = block.find("location");
	if (path_start == std::string::npos)
		return;
	
	path_start += 8;  // Pular "location"
	size_t path_end = block.find("{", path_start);
	if (path_end == std::string::npos)
		return;
	
	std::string path_line = block.substr(path_start, path_end - path_start);
	path_line = _trim(path_line);
	
	// Remover ~ se for regex
	if (path_line[0] == '~')
		path_line = _trim(path_line.substr(1));
	
	location.path = path_line;
	
	// Parse do conteúdo do bloco
	std::string content = block.substr(path_end + 1);
	std::istringstream stream(content);
	std::string line;
	
	while (std::getline(stream, line))
	{
		line = _trim(line);
		
		if (line.empty() || line[0] == '#' || line == "}")
			continue;
		
		std::vector<std::string> tokens = _split(line, ' ');
		if (tokens.empty())
			continue;
		
		std::string directive = tokens[0];
		
		if (directive == "allow" && tokens.size() >= 2)
		{
			std::string method = tokens[1];
			if (method[method.length() - 1] == ';')
				method.erase(method.length() - 1);
			
			if (method == "all")
			{
				location.allowed_methods.push_back("GET");
				location.allowed_methods.push_back("POST");
				location.allowed_methods.push_back("DELETE");
			}
			else
			{
				location.allowed_methods.push_back(method);
			}
		}
		else if (directive == "root" && tokens.size() >= 2)
		{
			location.root = tokens[1];
			if (location.root[location.root.length() - 1] == ';')
				location.root.erase(location.root.length() - 1);
		}
		else if (directive == "autoindex" && tokens.size() >= 2)
		{
			location.autoindex = (tokens[1] == "on" || tokens[1] == "on;");
		}
		else if (directive == "return" && tokens.size() >= 2)
		{
			location.redirect = tokens[1];
			if (location.redirect[location.redirect.length() - 1] == ';')
				location.redirect.erase(location.redirect.length() - 1);
		}
		else if (directive == "cgi" && tokens.size() >= 2)
		{
			location.cgi_enabled = (tokens[1] == "on" || tokens[1] == "on;");
		}
		else if (directive == "cgi_path" && tokens.size() >= 2)
		{
			location.cgi_path = tokens[1];
			if (location.cgi_path[location.cgi_path.length() - 1] == ';')
				location.cgi_path.erase(location.cgi_path.length() - 1);
		}
		else if (directive == "upload_dir" && tokens.size() >= 2)
		{
			location.upload_dir = tokens[1];
			if (location.upload_dir[location.upload_dir.length() - 1] == ';')
				location.upload_dir.erase(location.upload_dir.length() - 1);
		}
	}
	
	server.locations.push_back(location);
}

// Helper: Remove espaços em branco
std::string Config::_trim(const std::string& str) const
{
	const char* whitespace = " \t\n\r\f\v";
	size_t start = str.find_first_not_of(whitespace);
	
	if (start == std::string::npos)
		return "";
	
	size_t end = str.find_last_not_of(whitespace);
	return str.substr(start, end - start + 1);
}

// Helper: Divide string por delimitador
std::vector<std::string> Config::_split(const std::string& str, char delimiter) const
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream stream(str);
	
	while (std::getline(stream, token, delimiter))
	{
		token = _trim(token);
		if (!token.empty())
			tokens.push_back(token);
	}
	
	return tokens;
}

// Helper: Parse de tamanho (100M, 1G, 50K)
size_t Config::_parse_size(const std::string& size_str) const
{
	std::string str = size_str;
	if (str[str.length() - 1] == ';')
		str.erase(str.length() - 1);
	
	size_t multiplier = 1;
	char unit = str[str.length() - 1];
	
	if (!isdigit(unit))
	{
		if (unit == 'K' || unit == 'k')
			multiplier = 1024;
		else if (unit == 'M' || unit == 'm')
			multiplier = 1024 * 1024;
		else if (unit == 'G' || unit == 'g')
			multiplier = 1024 * 1024 * 1024;
		
		str.erase(str.length() - 1);
	}
	
	return atoi(str.c_str()) * multiplier;
}

// Valida a configuração carregada
void Config::validate_config() const
{
	if (_servers.empty())
		throw std::runtime_error("No server configuration found");
	
	// Verificar portas duplicadas no mesmo arquivo de configuração
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (_servers[i].port <= 0 || _servers[i].port > 65535)
			throw std::runtime_error("Invalid port number");
		
		if (_servers[i].root.empty())
			throw std::runtime_error("Server root directory not specified");
		
		// Verificar se há outra configuração com a mesma porta
		for (size_t j = i + 1; j < _servers.size(); j++)
		{
			if (_servers[i].port == _servers[j].port)
			{
				std::ostringstream oss;
				oss << "Error: Duplicate port " << _servers[i].port 
				    << " found in configuration file. Each server must have a unique port.";
				throw std::runtime_error(oss.str());
			}
		}
	}
}

// Getters
std::vector<ServerConfig> Config::get_servers() const
{
	return _servers;
}

ServerConfig Config::get_server(size_t index) const
{
	if (index >= _servers.size())
		throw std::out_of_range("Server index out of range");
	return _servers[index];
}

size_t Config::get_server_count() const
{
	return _servers.size();
}

int Config::get_port() const
{
	return _servers.empty() ? 8080 : _servers[0].port;
}

std::string Config::get_host() const
{
	return _servers.empty() ? "localhost" : _servers[0].host;
}

std::string Config::get_root() const
{
	return _servers.empty() ? "./www" : _servers[0].root;
}

std::vector<std::string> Config::get_index_files() const
{
	return _servers.empty() ? std::vector<std::string>() : _servers[0].index_files;
}

std::string Config::get_error_page(int code) const
{
	if (_servers.empty())
		return "";
	
	std::map<int, std::string>::const_iterator it = _servers[0].error_pages.find(code);
	return (it != _servers[0].error_pages.end()) ? it->second : "";
}

bool Config::is_autoindex_enabled() const
{
	return _servers.empty() ? false : _servers[0].autoindex;
}

size_t Config::get_max_body_size() const
{
	return _servers.empty() ? (100 * 1024 * 1024) : _servers[0].client_max_body_size;
}

int Config::get_timeout() const
{
	return _servers.empty() ? 30 : _servers[0].timeout;
}

int Config::get_cgi_timeout() const
{
	return _servers.empty() ? 5 : _servers[0].cgi_timeout;
}

// Busca location mais específico para uma URI
LocationConfig* Config::find_location(const std::string& uri, size_t server_index)
{
	if (server_index >= _servers.size())
		return NULL;
	
	LocationConfig* best_match = NULL;
	LocationConfig* regex_match = NULL;
	size_t best_match_length = 0;
	
	for (size_t i = 0; i < _servers[server_index].locations.size(); i++)
	{
		const std::string& loc_path = _servers[server_index].locations[i].path;
		
		// Verificar regex patterns (começam com ~)
		// Simplificado: apenas verifica extensão
		if (loc_path.find("\\.") != std::string::npos)
		{
			// Ex: ~ \.py$ -> verifica se URI termina com .py
			size_t ext_pos = loc_path.find("\\.");
			if (ext_pos != std::string::npos)
			{
				std::string ext = loc_path.substr(ext_pos + 2);
				if (ext[ext.length() - 1] == '$')
					ext.erase(ext.length() - 1);
				
				if (uri.length() >= ext.length() &&
				    uri.substr(uri.length() - ext.length()) == ext)
				{
					// Guardar regex match, mas não sobrescrever best_match ainda
					regex_match = &_servers[server_index].locations[i];
				}
			}
		}
		// Verificar se URI começa com o path da location (prefix match)
		else if (uri.find(loc_path) == 0)
		{
			// Encontrar match mais específico (path mais longo)
			if (loc_path.length() > best_match_length)
			{
				best_match = &_servers[server_index].locations[i];
				best_match_length = loc_path.length();
			}
		}
	}
	
	// Regex match tem prioridade sobre prefix match (comportamento similar ao nginx)
	if (regex_match)
		return regex_match;
	
	return best_match;
}

const LocationConfig* Config::find_location(const std::string& uri, size_t server_index) const
{
	return const_cast<Config*>(this)->find_location(uri, server_index);
}

// Verificação de método permitido
bool Config::is_method_allowed(const std::string& method, const std::string& uri, size_t server_index) const
{
	const LocationConfig* location = find_location(uri, server_index);
	
	if (!location || location->allowed_methods.empty())
		return false;  // Se não há métodos definidos, bloquear
	
	for (size_t i = 0; i < location->allowed_methods.size(); i++)
	{
		if (location->allowed_methods[i] == method)
			return true;
	}
	
	return false;
}
