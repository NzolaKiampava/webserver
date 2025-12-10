#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include <fstream>
#include <sstream>

// Construtor: Inicializa resposta com status 200 OK
Response::Response() : _status_code(200)
{
	// TODO: Inicializar response com defaults
}

// Destrutor
Response::~Response()
{
	// TODO: Limpar recursos
}

// Gera resposta apropriada baseado na requisição
// - Verifica método HTTP (GET, POST, DELETE, etc.)
// - Para GET: localiza arquivo, lê conteúdo, define content-type
// - Para POST: salva arquivo/dados no servidor
// - Para DELETE: remove arquivo
// - Trata erros (404, 403, 500) com páginas de erro
// - Define headers apropriados (Content-Length, Content-Type, etc.)
void Response::generate(const Request& request, const std::string& root)
{
	// TODO: Implementar geração de resposta HTTP
}

// Define o código de status HTTP
void Response::set_status(int code)
{
	// TODO: Setar código e mensagem de status
}

// Adiciona um header à resposta
void Response::set_header(const std::string& key, const std::string& value)
{
	// TODO: Adicionar header à resposta
}

// Define o body da resposta
void Response::set_body(const std::string& body)
{
	// TODO: Setar body
}

// Monta a resposta HTTP completa formatada
// - Primeira linha: "HTTP/1.1 <status_code> <status_message>"
// - Headers com "\r\n" entre eles
// - Linha em branco
// - Body
std::string Response::get_response() const
{
	// TODO: Montar resposta HTTP completa
}

// Retorna a mensagem de status para um código HTTP
// Ex: 200 -> "OK", 404 -> "Not Found"
std::string Response::get_status_message(int code) const
{
	// TODO: Retornar mensagem de status apropriada
}
