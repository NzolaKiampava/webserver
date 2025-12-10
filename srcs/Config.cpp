#include "../includes/Config.hpp"
#include <fstream>
#include <sstream>

// Construtor padrão: Inicializa com valores padrão
Config::Config() : _port(8080), _host("localhost"), _autoindex(false)
{
	// TODO: Valores padrão inicializados
}

// Construtor com arquivo de configuração
Config::Config(const std::string& configFile) : _port(8080), _host("localhost"), _autoindex(false)
{
	// TODO: Chamar parse_config
}

// Destrutor
Config::~Config()
{
	// TODO: Liberar recursos se necessário
}

// Parse do arquivo de configuração
// - Abre e lê arquivo .conf
// - Identifica diretivas (listen, server_name, root, etc.)
// - Popula as variáveis membro
// - Trata erros de parsing
void Config::parse_config(const std::string& configFile)
{
	// TODO: Implementar parser de configuração (estilo nginx)
}

// Getter: Retorna a porta
int Config::get_port() const
{
	// TODO: Retornar porta
}

// Getter: Retorna o hostname/IP
std::string Config::get_host() const
{
	// TODO: Retornar host
}

// Getter: Retorna o diretório raiz (document root)
std::string Config::get_root() const
{
	// TODO: Retornar raiz do servidor
}

// Getter: Retorna lista de arquivos índice (ex: index.html, index.php)
std::vector<std::string> Config::get_index_files() const
{
	// TODO: Retornar arquivos de índice padrão
}

// Getter: Retorna caminho de página de erro para um código HTTP específico
std::string Config::get_error_page(int code) const
{
	// TODO: Retornar página de erro para o código
}

// Getter: Retorna se autoindex está habilitado
bool Config::is_autoindex_enabled() const
{
	// TODO: Retornar se autoindex está ativo
}
