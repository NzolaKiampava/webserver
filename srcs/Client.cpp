#include "../includes/Client.hpp"
#include <unistd.h>
#include <cstring>

// Construtor: Inicializa cliente com socket e IP
Client::Client(int socket, const std::string& ip) : _socket(socket), _ip_address(ip)
{
	// TODO: Inicializar cliente
}

// Destrutor: Fecha o socket do cliente
Client::~Client()
{
	// TODO: Fechar socket e limpar recursos
}

// Recebe dados do cliente
// - Usa recv() em non-blocking mode
// - Adiciona dados ao buffer
// - Se buffer contém requisição completa, faz parse
void Client::receive_data()
{
	// TODO: Receber dados do cliente com recv()
}

// Envia resposta ao cliente
// - Usa send() para enviar resposta HTTP
// - Trata possíveis erros de envio
// - Pode enviar em múltiplos chunks se resposta for grande
void Client::send_response()
{
	// TODO: Enviar resposta ao cliente
}

// Getter: Retorna descritor de socket
int Client::get_socket() const
{
	// TODO: Retornar socket
}

// Getter: Retorna endereço IP do cliente
std::string Client::get_ip() const
{
	// TODO: Retornar IP
}

// Getter: Retorna referência para objeto Request
Request& Client::get_request()
{
	// TODO: Retornar request
}

// Getter: Retorna referência para objeto Response
Response& Client::get_response()
{
	// TODO: Retornar response
}

// Getter: Retorna referência para buffer de dados recebido
std::string& Client::get_buffer()
{
	// TODO: Retornar buffer
}

// Verifica se a requisição do cliente está completa
bool Client::is_request_complete() const
{
	// TODO: Verificar se request está completa
}
