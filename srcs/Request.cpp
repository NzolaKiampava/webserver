#include "../includes/Request.hpp"
#include "../includes/Utils.hpp"

// Construtor
Request::Request()
{
	// TODO: Inicializar estrutura de requisição
}

// Destrutor
Request::~Request()
{
	// TODO: Limpar recursos
}

// Parse completo de uma requisição HTTP bruta
// - Extrai método (GET, POST, etc.), URI e versão HTTP da linha inicial
// - Faz parse dos headers (Content-Type, Content-Length, etc.)
// - Separa body se existir
// - Trata diferentes formatos (chunked encoding, multipart, etc.)
void Request::parse(const std::string& raw_request)
{
	// TODO: Implementar parser HTTP completo
}

// Getter: Retorna método HTTP (GET, POST, DELETE, etc.)
std::string Request::get_method() const
{
	// TODO: Retornar método
}

// Getter: Retorna a URI/caminho solicitado
std::string Request::get_uri() const
{
	// TODO: Retornar URI
}

// Getter: Retorna versão HTTP (1.0 ou 1.1)
std::string Request::get_version() const
{
	// TODO: Retornar versão
}

// Getter: Retorna valor de um header específico (case-insensitive)
std::string Request::get_header(const std::string& key) const
{
	// TODO: Retornar header se existir
}

// Getter: Retorna body da requisição
std::string Request::get_body() const
{
	// TODO: Retornar body
}

// Valida se a requisição está completa e válida
// - Verifica presença de método e URI
// - Verifica content-length vs tamanho real do body
// - Retorna true se pronta para processamento
bool Request::is_complete() const
{
	// TODO: Verificar se requisição está completa
}
