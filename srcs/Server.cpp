/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:54:24 by nkiampav          #+#    #+#             */
/*   Updated: 2026/03/09 10:35:43 by nkiampav         ###   ########.fr       */
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
Server::Server(const std::string& configFile) : _config(configFile)
{
	// Configurar signals handlers para shutdowns gracioso
	signal(SIGINT, signal_handler); // CTRL+C
	signal(SIGTERM, signal_handler); // KILL
	signal(SIGPIPE, SIG_IGN); // Ignorar SIGPIPE (cliente desconecta durante write)
	
	std::cout << "Server initialized with " << _config.get_server_count() << " server configuration(s)" << std::endl;
}

// Destrutor: Limpa recursos e fecha o servidor
Server::~Server()
{
	close_server();
}

// Getter: Retorna os sockets dos servidores
const std::vector<int>& Server::get_sockets() const
{
	return _server_sockets;
}

// Getter: Retorna o número de servidores configurados
size_t Server::get_server_count() const
{
	return _config.get_server_count();
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

// Cria e faz bind de um socket individual para um host:port
// Retorna o descritor do socket ou lança exceção em caso de erro
int Server::_create_and_bind_socket(const std::string& host, int port)
{
	// Criar socket (IPv4, TCP)
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		throw std::runtime_error("Error: Could not create socket");
	
	// Permitir reutilização de porta/endereço
	// util quando reinciamos o servidor rapidamente
	int reuse = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		close(sock);
		throw std::runtime_error("Error: Could not set socket options");
	}

	// configurar socket para non-blocking
	_set_nonblocking(sock);
	
	// Configurar estrutura de endereço do servidor
	struct sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // Ipv4
	server_addr.sin_port = htons(port);  // porta em network byte order
	
	if (inet_aton(host.c_str(), &server_addr.sin_addr) == 0)
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind do socket ao endereço e porta
	if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		std::ostringstream oss;
		oss << "Error: Could not bind socket to " << host << ":" << port;
		close(sock);
		throw std::runtime_error(oss.str());
	}
	
	// Começar a escutar por conexões
	if (listen(sock, SOMAXCONN) < 0)
	{	
		close(sock);
		throw std::runtime_error("Error: Could not listen on socket");
	}
	
	std::cout << "Server listening on " << host << ":" << port << std::endl;
	return sock;
}

// Cria sockets para todos os servidores configurados
// - Itera sobre todas as configurações de servidor
// - Cria um socket listening para cada porta
// - Mapeia cada socket ao índice de sua configuração
void Server::_setup_sockets()
{
	std::vector<ServerConfig> servers = _config.get_servers();
	
	for (size_t i = 0; i < servers.size(); ++i)
	{
		try 
		{
			int sock = _create_and_bind_socket(servers[i].host, servers[i].port);
			_server_sockets.push_back(sock);
			_socket_to_config[sock] = i;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to setup server " << i << ": " << e.what() << std::endl;
			// Fechar sockets já criados
			for (size_t j = 0; j < _server_sockets.size(); ++j)
				close(_server_sockets[j]);
			_server_sockets.clear();
			_socket_to_config.clear();
			throw;
		}
	}
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
		// Setup dos sockets (bind, listen) para todos os servidores
		_setup_sockets();

		_running = true;
		
		std::cout << "Server is running. Waiting for connections..." << std::endl;
		std::cout << "Press Ctrl+c to shutdown gracefully" << std::endl;

		// Loop principal do servidor
		while (_running && !g_shutdown)
		{
			// Preparar array de pollfd
            // pollfd[0..N-1] serão os sockets dos servidores (listening)
            // pollfd[N..M] serão os sockets dos clientes
            std::vector<struct pollfd> poll_fds;
            
            // Adicionar todos os sockets dos servidores ao poll
            for (size_t i = 0; i < _server_sockets.size(); ++i)
            {
            	struct pollfd server_pollfd;
            	server_pollfd.fd = _server_sockets[i];
            	server_pollfd.events = POLLIN;
            	server_pollfd.revents = 0;
            	poll_fds.push_back(server_pollfd);
            }
            
            size_t num_server_sockets = _server_sockets.size();
            
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
            
            // Verificar se há nova conexão em algum dos sockets principais
            // Aceitar TODAS as conexões pendentes (não apenas uma)
            for (size_t i = 0; i < num_server_sockets; ++i)
            {
            	if (poll_fds[i].revents & POLLIN)
            	{
            		int server_sock = _server_sockets[i];
            		size_t config_index = _socket_to_config[server_sock];
            		
            		// Loop para aceitar múltiplas conexões pendentes
            		// O socket é non-blocking, então accept() retorna -1 quando não há mais conexões
            		while (true)
            		{
            			size_t old_clients_size = _clients.size();
            			accept_connection(server_sock, config_index);
            			// Se não aceitou nova conexão, sair do loop
            			if (_clients.size() == old_clients_size)
            				break;
            		}
            	}
            }
            
            // Verificar atividade nos sockets dos clientes
            // Iteramos de trás para frente para poder remover clientes com segurança
            for (int i = poll_fds.size() - 1; i >= static_cast<int>(num_server_sockets); --i)
            {
                // Índice do cliente é (i - num_server_sockets)
                size_t client_index = i - num_server_sockets;
                
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
void Server::accept_connection(int server_socket, size_t config_index)
{
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	
	int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
	
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
	
	// Obter informação da configuração do servidor
	ServerConfig server_cfg = _config.get_server(config_index);
	
	std::cout << "New connection accepted from " << client_ip << ":" << ntohs(client_addr.sin_port)
			  << " on server " << server_cfg.host << ":" << server_cfg.port
			  << " (socket " << client_socket << ")" << std::endl;
	
	// Criar novo cliente e adicionar à lista (passando o índice de configuração)
	_clients.push_back(Client(client_socket, client_ip, config_index));
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
			
		// Obter configuração do servidor para este cliente
		size_t config_index = client.get_server_config_index();
		ServerConfig server_cfg = _config.get_server(config_index);
		
		// Gerar resposta
		Response response;
		response.set_error_pages(server_cfg.error_pages);  // Configurar páginas de erro
		response.set_root(server_cfg.root);                // Configurar root
		response.set_index_files(server_cfg.index_files);  // Configurar arquivos index
		response.generate(request, server_cfg.root);
			
		// Obter resposta HTTP completa
		std::string http_response = response.get_response();
		
		// Definir buffer de resposta no cliente
		client.set_response_buffer(http_response);
		
		std::cout << "Response generated: " << response.get_status_code() 
		          << " " << response.get_status_message(response.get_status_code()) << std::endl;
		
		// A resposta será enviada quando poll() indicar POLLOUT
		// (próxima iteração do loop, pois has_response_to_send() retornará true)
		}
	}
	catch (const std::exception& e)
	{
		std::string error_msg = e.what();
		// Se o cliente apenas fechou a conexão, não é um erro
		if (error_msg == "Client disconnected")
		{
			// Log informativo apenas, sem "Error"
		}
		else
		{
			std::cerr << "Error handling client: " << e.what() << std::endl;
		}
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
            std::cout << "Response fully sent to " << client.get_ip() << std::endl;
            
            // Verifica se o cliente quer manter a conexão aberta
            // E se o seu servidor também suporta (ex: limitando a conexões HTTP/1.1)
            std::string conn_header = client.get_request().get_header("Connection");
            
            if (conn_header == "keep-alive") {
                std::cout << "Keeping connection alive for " << client.get_ip() << std::endl;
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
	
	// Fechar todos os sockets dos servidores
	for (size_t i = 0; i < _server_sockets.size(); ++i)
	{
		if (_server_sockets[i] >= 0)
		{
			close(_server_sockets[i]);
		}
	}
	_server_sockets.clear();
	_socket_to_config.clear();
	
	_running = false;
	std::cout << "Server closed successfully" << std::endl;
}
