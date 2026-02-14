#include "Client.hpp"
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <sys/socket.h>
#include <iostream>

// Tamanho do buffer para receber dados
#define BUFFER_SIZE 4096

// Construtor: Inicializa cliente com socket e IP
Client::Client(int socket, const std::string& ip) 
	: _socket(socket), 
	_ip_address(ip),
	_request_complete(false),
	_response_ready(false),
	_response_sent(false),
	_bytes_sent(0)

{
    update_activity();
	// TODO: Adicionar timestamp para controle de timeout
    std::cout << "Client created: " << _ip_address << " on socket " << _socket << std::endl;
}
// Destrutor: Fecha o socket do cliente
Client::~Client()
{
    // Remova o close(_socket) daqui.
    // O socket deve ser fechado apenas quando o servidor decidir 
    // remover o cliente da lista ativa.
    std::cout << "Client object destroyed for IP: " << _ip_address << std::endl;
}

// Recebe dados do cliente
// - Usa recv() para ler dados dados do socket
// - Adiciona dados ao buffer interno
// - Detecta quando cliente desconecta (recv retirna 0)
// - Trata erros (EAGAIN/EWOULDBLOCK sao normais em non-blocking)
void Client::receive_data()
{
    char buffer[BUFFER_SIZE];
    
    // recv() em non-blocking retorna imediatamente
    // - Retorna > 0: número de bytes recebidos
    // - Retorna 0: cliente fechou conexão
    // - Retorna -1: erro (verificar errno)
    ssize_t bytes_received = recv(_socket, buffer, BUFFER_SIZE - 1, 0);
    
    std::cout << "[DEBUG] recv() returned: " << bytes_received << " (errno=" << errno << ")" << std::endl;
    
    if (bytes_received > 0)
    {
        // Dados recebidos com sucesso
        buffer[bytes_received] = '\0';  // Null-terminator
        _buffer.append(buffer, bytes_received);  // Adicionar ao buffer interno
        
        std::cout << "Received " << bytes_received << " bytes from " << _ip_address << std::endl;
        std::cout << "[DEBUG] Request buffer: " << _buffer << std::endl;
        
        // Verificar se temos uma requisição HTTP completa
        _check_request_complete();
    }
    else if (bytes_received == 0)
    {
        // Cliente fechou a conexão
        std::cout << "Client " << _ip_address << " closed connection" << std::endl;
        throw std::runtime_error("Client disconnected");
    }
    else
    {
        // Erro no recv()
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Não há dados disponíveis agora (normal em non-blocking)
            std::cout << "[DEBUG] EAGAIN/EWOULDBLOCK - no data available" << std::endl;
            return;
        }
        else
        {
            // Erro real
            std::cerr << "Error receiving data from " << _ip_address 
                      << ": " << strerror(errno) << std::endl;
            throw std::runtime_error("recv() error");
        }
    }
}

// Envia resposta HTTP ao cliente em modo non-blocking
// - Usa send() para enviar dados do buffer de resposta
// - Pode enviar em múltiplos chunks se resposta for grande
// - Mantém controle de quantos bytes já foram enviados
// - Trata erros (EAGAIN/EWOULDBLOCK são normais em non-blocking)
void Client::send_response()
{
    // Se não há resposta pronta, não fazer nada
    if (!_response_ready || _response_buffer.empty())
        return;
    
    // Calcular quantos bytes ainda faltam enviar
    size_t remaining = _response_buffer.size() - _bytes_sent;
    
    if (remaining == 0)
    {
        // Tudo já foi enviado
        _response_sent = true;
        return;
    }
    
    // send() em non-blocking pode enviar menos bytes que o solicitado
    // - Retorna > 0: número de bytes enviados
    // - Retorna -1: erro (verificar errno)
    ssize_t bytes_sent = send(_socket, 
                               _response_buffer.c_str() + _bytes_sent, 
                               remaining, 
                               0);
    
    if (bytes_sent > 0)
    {
        _bytes_sent += bytes_sent;
        std::cout << "Sent " << bytes_sent << " bytes to " << _ip_address 
                  << " (total: " << _bytes_sent << "/" << _response_buffer.size() << ")" 
                  << std::endl;
        
        // Verificar se enviamos tudo
        if (_bytes_sent >= _response_buffer.size())
        {
            _response_sent = true;
            std::cout << "Response fully sent to " << _ip_address << std::endl;
        }
    }
    else if (bytes_sent == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Socket não está pronto para escrita agora (normal em non-blocking)
            // Vamos tentar novamente na próxima iteração do poll()
            return;
        }
        else
        {
            // Erro real
            std::cerr << "Error sending data to " << _ip_address 
                      << ": " << strerror(errno) << std::endl;
            throw std::runtime_error("send() error");
        }
    }
}

// Getter: Retorna descritor de socket do cliente
int Client::get_socket() const
{
    return _socket;
}

// Getter: Retorna endereço IP do cliente
std::string Client::get_ip() const
{
    return _ip_address;
}

// Getter: Retorna referência para objeto Request
// TODO: Será implementado quando a classe Request estiver pronta (Membro 2)
Request& Client::get_request()
{
    return _request;
}

// Getter: Retorna referência para objeto Response
// TODO: Será implementado quando a classe Response estiver pronta (Membro 2)
Response& Client::get_response()
{
    return _response;
}

// Getter: Retorna referência para buffer de dados recebidos
std::string& Client::get_buffer()
{
    return _buffer;
}

// Verifica se a requisição HTTP do cliente está completa
// - Uma requisição HTTP termina com "\r\n\r\n" (linha vazia)
// - Para POST/PUT, também precisa verificar Content-Length
bool Client::is_request_complete() const
{
    return _request_complete;
}

// Verifica se há resposta pronta para ser enviada
bool Client::has_response_to_send() const
{
    return _response_ready && !_response_sent;
}

// Verifica se a resposta foi completamente enviada
bool Client::response_sent() const
{
    return _response_sent;
}

// Define o buffer de resposta a ser enviado
// - Chamado pela camada de Response (Membro 2) quando resposta está pronta
void Client::set_response_buffer(const std::string& response)
{
    _response_buffer = response;
    _response_ready = true;
    _response_sent = false;
    _bytes_sent = 0;
}

// Verifica se o buffer contém uma requisição HTTP completa
// - Procura por "\r\n\r\n" que indica fim dos headers
// - Para métodos com body (POST/PUT), verifica Content-Length
// - Marca flag _request_complete quando detecta requisição completa
void Client::_check_request_complete()
{
    // Procurar pelo fim dos headers HTTP ("\r\n\r\n")
    size_t header_end = _buffer.find("\r\n\r\n");
    
    if (header_end == std::string::npos)
    {
        // Ainda não recebemos os headers completos
        _request_complete = false;
        return;
    }
    
    // Headers completos recebidos
    std::string headers = _buffer.substr(0, header_end);
    
    // Verificar se há body (Content-Length presente)
    size_t content_length_pos = headers.find("Content-Length:");
    
    if (content_length_pos != std::string::npos)
    {
        // Extrair valor do Content-Length
        size_t value_start = content_length_pos + 15; // strlen("Content-Length:")
        size_t value_end = headers.find("\r\n", value_start);
        
        std::string length_str = headers.substr(value_start, value_end - value_start);
        
        // Remover espaços em branco
        size_t first_digit = length_str.find_first_of("0123456789");
        if (first_digit != std::string::npos)
            length_str = length_str.substr(first_digit);
        
        int content_length = atoi(length_str.c_str());
        
        // Calcular quantos bytes do body já recebemos
        size_t body_start = header_end + 4; // +4 para pular "\r\n\r\n"
        size_t body_received = _buffer.size() - body_start;
        
        // Verificar se recebemos todo o body
        if (body_received >= static_cast<size_t>(content_length))
        {
            _request_complete = true;
            std::cout << "Complete request received (with body) from " << _ip_address << std::endl;
        }
        else
        {
            _request_complete = false;
            std::cout << "Partial request: body " << body_received 
                      << "/" << content_length << " bytes received" << std::endl;
        }
    }
    else
    {
        // Sem Content-Length, requisição está completa após headers
        // (para GET, DELETE, etc.)
        _request_complete = true;
        std::cout << "Complete request received (headers only) from " << _ip_address << std::endl;
    }
}

// Atualiza o timestamp de atividade (chame após recv ou send bem-sucedido)
void Client::update_activity() {
    _last_activity = time(NULL);
}

// Getter para o timestamp
time_t Client::get_last_activity() const {
    return _last_activity;
}

// Prepara o objeto para uma nova requisição na mesma conexão (Keep-Alive)
// Limpa os dados da requisição anterior para permitir uma nova na mesma conexão
void Client::reset_for_next_request() {
    _buffer.clear();
    _response_buffer.clear();
    _request_complete = false;
    _response_ready = false;
    _response_sent = false;
    _bytes_sent = 0;
    
    // É importante ter um método clear() nas suas classes Request e Response
    _request = Request(); 
    _response = Response();
    
    update_activity();
    std::cout << "[DEBUG] Client " << _ip_address << " reset for Keep-Alive" << std::endl;
}
