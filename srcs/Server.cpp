/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:54:24 by nkiampav          #+#    #+#             */
/*   Updated: 2026/03/01 23:02:44 by nkiampav         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <signal.h>
#include <poll.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

// variavel global para controlar shutdown graciosos
// esse valor eh modificado a cada sinal
volatile sig_atomic_t g_shutdown = 0;

void signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
	{
		std::cout << "\nSignal received [" << signum  << "]" << std::endl;
		g_shutdown = 1;
	}
}

// Construtor: Inicializa o servidor com arquivo de configuração
// - Carrega as configurações do arquivo
// - Define porta, host e outras propriedades
Server::Server(const std::string& configFile) : _socket(-1), _port(8080), _host("localhost"), _config(configFile)
{
	_port = _config.get_port();
	_host = _config.get_host();

	// Configurar signals handlers para shutdowns gracioso
	signal(SIGINT, signal_handler); // CTRL+C
	signal(SIGTERM, signal_handler); // KILL
	signal(SIGPIPE, SIG_IGN); // Ignorar SIGPIPE (cliente desconecta durante write)
	std::cout << "Server initialized with port: " << _port << " and host: " << _host << std::endl;
}

// Destrutor: Limpa recursos e fecha o servidor
Server::~Server()
{
	close_server();
}

// Getter: Retorna o socket do servidor
int Server::get_socket() const
{
	return _socket;
}

// Getter: Retorna a porta em que o servidor está escutando
int Server::get_port() const
{
	return _port;
}

// COnfigura um socket para modo non-blocking
// - Usa fcntl() para definir flag O_NONBLOCK
// - Essencial para evitar ue operacoes de I/O bloqueiem o servidor
void Server::_set_nonblocking(int socket)
{
	// obter flags atuais do socket
	int flags = fcntl(socket, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Error: fcntl F_GETFL failed");

	// Adicionar flag O_NONBLOCK
	if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Error: fcntl F_SETFL O_NONBLOCK failed");
}

// Cria o socket do servidor e faz bind/listen
// - cria socket ipv4 tcp
// - configura opcoes do socke (SO_RESUSEADDR)
// - Defina modo non-blocking
// - Faz bind ao endereco e porta
// - coloca socket em modo listening
void Server::_setup_socket()
{
	// Criar socket (IPv4, TCP)
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
		throw std::runtime_error("Error: Could not create socket");
	
	// Permitir reutilização de porta/endereço
	// util quando reinciamos o servidor rapidamente
	int reuse = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		close(_socket);
		throw std::runtime_error("Error: Could not set socket options");
	}

	// configurar socket para non-blocking
	_set_nonblocking(_socket);
	
	// Configurar estrutura de endereço do servidor
	struct sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // Ipv4
	server_addr.sin_port = htons(_port);  // porta em network byte order
	
	if (inet_aton(_host.c_str(), &server_addr.sin_addr) == 0)
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind do socket ao endereço e porta
	if (bind(_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		std::ostringstream oss;
		oss << "Error: Could not bind socket to port " << _port;
		close(_socket);
		throw std::runtime_error(oss.str());
	}
	
	// Começar a escutar por conexões
	if (listen(_socket, SOMAXCONN) < 0)
	{	
		close(_socket);
		throw std::runtime_error("Error: Could not listen on socket");
	}
	
	std::cout << "Server listening on " << _host << ":" << _port << std::endl;
}

// Inicia o loop principal do servidor com poll
// - Cria array do pollfd para monitorar sockets
// - Loop infinito aguardando eventos
// - Trata novas conexoes
// - Trata dados de clientes existentes
// - Remove clientes desconectados
void Server::run()
{
	try
	{
		// Setup do socket (bind, listen)
		_setup_socket();

		_running = true;
		
		std::cout << "Server is running. Waiting for connections..." << std::endl;
		std::cout << "Press Ctrl+c to shutdown gracefully" << std::endl;

		// Loop principal do servidor
		while (_running && !g_shutdown)
		{
			// Preparar array de pollfd
            // pollfd[0] será sempre o socket do servidor
            // pollfd[1..n] serão os sockets dos clientes
            std::vector<struct pollfd> poll_fds;
            
            // Adicionar socket do servidor ao poll
            struct pollfd server_pollfd;
            server_pollfd.fd = _socket;           // Socket a monitorar
            server_pollfd.events = POLLIN;        // Interessado em eventos de leitura
            server_pollfd.revents = 0;            // Limpar eventos retornados
            poll_fds.push_back(server_pollfd);
            
            // Adicionar todos os sockets de clientes ao poll
            for (size_t i = 0; i < _clients.size(); ++i)
            {
                struct pollfd client_pollfd;
                client_pollfd.fd = _clients[i].get_socket();
                
                // Monitorar leitura (POLLIN) e escrita (POLLOUT) se há dados para enviar
                client_pollfd.events = POLLIN;
                
                // Se cliente tem resposta pronta, também monitorar POLLOUT
                if (_clients[i].has_response_to_send())
                    client_pollfd.events |= POLLOUT;
                
                client_pollfd.revents = 0;
                poll_fds.push_back(client_pollfd);
            }
            
            // Chamar poll() com timeout de 5000ms (5 segundos)
            // poll() bloqueia até:
            // 1. Haver atividade em algum socket
            // 2. Timeout expirar
            // 3. Sinal interromper
            int poll_count = poll(&poll_fds[0], poll_fds.size(), 5000);
            
            // Erro no poll (exceto EINTR que é interrupção por sinal)
            if (poll_count < 0)
            {
                if (errno == EINTR)
                    continue; // Sinal recebido, continuar loop
                throw std::runtime_error("Error: poll() failed");
            }
            
            // Timeout - nenhuma atividade
            if (poll_count == 0)
            {
                // Aqui podemos verificar timeouts de clientes
                _check_client_timeouts();
                continue;
            }
            
            // Há atividade em um ou mais sockets
            
            // Verificar se há nova conexão no socket principal (índice 0)
            if (poll_fds[0].revents & POLLIN)
            {
                accept_connection();
            }
            
            // Verificar atividade nos sockets dos clientes (índices 1..n)
            // Iteramos de trás para frente para poder remover clientes com segurança
            for (int i = poll_fds.size() - 1; i > 0; --i)
            {
                // Índice do cliente é (i - 1) porque pollfd[0] é o servidor
                size_t client_index = i - 1;
                
                // POLLNVAL significa socket inválido - remover imediatamente
                if (poll_fds[i].revents & POLLNVAL)
                {
                    std::cerr << "[ERROR] Invalid socket for client " << _clients[client_index].get_ip() << std::endl;
                    _remove_client(client_index);
                    continue;
                }
                
                // Se há dados para ler (POLLIN), processar SEMPRE primeiro
                // Mesmo que POLLHUP esteja setado, pode haver dados no buffer
                if (poll_fds[i].revents & POLLIN)
                {
                    handle_client_read(_clients[client_index], client_index);
                    // Não fazer continue - pode ter sido removido em handle_client_read
                }
                
                // Se socket está pronto para escrita (POLLOUT) e há dados para enviar
                else if ((poll_fds[i].revents & POLLOUT) && client_index < _clients.size())
                {
                    handle_client_write(_clients[client_index], client_index);
                }
                
                // Verificar erro/desconexão se não processou dados e cliente ainda existe
                else if ((poll_fds[i].revents & (POLLERR | POLLHUP)) && client_index < _clients.size())
                {
                    _remove_client(client_index);
                }
            }
        }
		std::cout << "Server shutting down..." << std::endl;
		close_server();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Server error: " << e.what() << std::endl;
		close_server();
		throw;
	}
}

// Aceita uma nova conexão de cliente
// - Usa accept() para receber conexões
// - configura socket do cliente em non-blocking
// - Cria novo objeto Client
// - Adiciona à lista de clientes
void Server::accept_connection()
{
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	
	int client_socket = accept(_socket, (struct sockaddr*)&client_addr, &client_addr_len);
	
	if (client_socket < 0)
	{
		// Em non-blocking, se poll() indicou POLLIN mas accept() falhou,
		// pode ser porque a conexão foi fechada antes de accept() ser chamado
		// Simplesmente retornamos sem erro
		return;
	}

	// configurar socket do client para non-blocking

	try 
	{
		_set_nonblocking(client_socket);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error setting client socket non-blocking: " << e.what() << std::endl;
		close(client_socket);
		return;
	}
	
	// Obter IP do cliente em formato string
	std::string client_ip = inet_ntoa(client_addr.sin_addr);
	
	std::cout << "New connection accepted from " << client_ip << ":" << ntohs(client_addr.sin_port)
			  << " (socket " << client_socket << ")" << std::endl;
	
	// Criar novo cliente e adicionar à lista
	_clients.push_back(Client(client_socket, client_ip));
}

// Processa requisição de um cliente
// - Recebe dados do cliente
// - Adiciona ao buffer do cliente
// - Faz parse da requisição HTTP
// - Gera resposta apropriada
// - Envia resposta ao cliente
void Server::handle_client_read(Client& client, size_t client_index)
{
	try
	{
		// Receber dados do cliente
		client.receive_data();
		
		// Verificar se a requisição está completa
		if (client.is_request_complete())
		{
			std::cout << "Complete request received from " << client.get_ip() << std::endl;

			// Processar a requisição
			Request request;
			request.parse(client.get_buffer());
			
			// Armazenar request no cliente para poder verificar headers depois
			client.get_request() = request;
			
			std::cout << "Request: " << request.get_method() << " " 
			          << request.get_uri() << " " << request.get_version() << std::endl;
			
			// Gerar resposta
			Response response;
			response.generate(request, "./www");
			
			// Obter resposta HTTP completa
			std::string http_response = response.get_response();
			
			// Definir buffer de resposta no cliente
			client.set_response_buffer(http_response);
			
			std::cout << "Response generated: " << response.get_status_code() 
			          << " " << response.get_status_message(response.get_status_code()) << std::endl;
			
			// Tentar enviar a resposta imediatamente
			client.send_response();
			
			// Se resposta foi completamente enviada, verificar keep-alive
			if (client.response_sent())
			{
				std::cout << "Response fully sent to " << client.get_ip() << std::endl;
				
				// Verificar se o cliente quer manter a conexão aberta
				std::string conn_header = client.get_request().get_header("Connection");
				
				if (conn_header == "keep-alive") {
					std::cout << "Keeping connection alive for " << client.get_ip() << std::endl;
					client.reset_for_next_request();
				} else {
					std::cout << "Closing connection (Connection: close) for " << client.get_ip() << std::endl;
					_remove_client(client_index);
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error handling client: " << e.what() << std::endl;
		_remove_client(client_index);
	}
}

// Processa escrita de dados para um cliente
// - Envia resposta HTTP do cliente
// - Trata erros de envio
// - Remove cliente apos enviar resposta completa (HTTP/1.0) ou mantem conexao (HTTP/1.1)
void Server::handle_client_write(Client& client, size_t client_index) {
    try {
        client.send_response();
        
        if (client.response_sent()) {
            // Verifica se o cliente quer manter a conexão aberta
            // E se o seu servidor também suporta (ex: limitando a conexões HTTP/1.1)
            std::string conn_header = client.get_request().get_header("Connection");
            
            if (conn_header == "keep-alive") {
                client.reset_for_next_request();
            } else {
                std::cout << "Closing connection (Connection: close) for " << client.get_ip() << std::endl;
                _remove_client(client_index);
            }
        }
    } catch (const std::exception& e) {
        _remove_client(client_index);
    }
}

// Remove um cliente da lista
// - Fecha o socket do cliente
// - Remove da lista de clientes ativos
void Server::_remove_client(size_t index)
{
    if (index < _clients.size())
    {
        int client_socket = _clients[index].get_socket();
		std::cout << "Removing client " << _clients[index].get_ip() << " (socket " << client_socket << ")" << std::endl;
        if (client_socket >= 0)
            close(client_socket);
        
        // Remove cliente do vector
        _clients.erase(_clients.begin() + index);
    }
}

// Verifica timeouts de clientes inativos
// - Clientes que não enviam dados completos em tempo razoável são desconectados
// - Previne ataques de Slowloris
void Server::_check_client_timeouts() {
    time_t now = time(NULL);
    const int TIMEOUT_LIMIT = 60; // 60 segundos de inatividade

    // Percorremos de trás para frente para não errar o índice ao remover
    for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
        if (now - _clients[i].get_last_activity() > TIMEOUT_LIMIT) {
            std::cout << "[TIMEOUT] Closing connection for " << _clients[i].get_ip() << std::endl;
            _remove_client(i);
        }
    }
}


// Fecha o servidor de forma segura
// - Fecha todos os sockets de clientes
// - Fecha o socket principal
// - Libera recursos
void Server::close_server()
{
	// Fechar todos os sockets de clientes
	std::cout << "Closing " << _clients.size() << " client connection(s)..." << std::endl;
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		int client_socket = _clients[i].get_socket();
		if (client_socket >= 0)
		{
			close(client_socket);
		}
	}
	_clients.clear();
	
	// Fechar o socket principal do servidor
	if (_socket >= 0)
	{
		close(_socket);
		_socket = -1;
	}
	
	_running = false;
	std::cout << "Server closed successfully" << std::endl;
}
