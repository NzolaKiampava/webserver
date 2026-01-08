#include "../includes/Server.hpp"
#include "../includes/Config.hpp"
#include <iostream>
#include <cstring>
#include <sys/select.h>
#include <signal.h>

// Construtor: Inicializa o servidor com arquivo de configuração
// - Carrega as configurações do arquivo
// - Define porta, host e outras propriedades
Server::Server(const std::string& configFile) : _socket(-1), _port(8080), _host("localhost"), _config(configFile)
{
	_port = _config.get_port();
	_host = _config.get_host();
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

// Cria o socket do servidor e faz bind/listen
void Server::_setup_socket()
{
	// Criar socket (IPv4, TCP)
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
		throw std::runtime_error("Error: Could not create socket");
	
	// Permitir reutilização de porta/endereço
	int reuse = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
		throw std::runtime_error("Error: Could not set socket options");
	
	// Configurar estrutura de endereço do servidor
	struct sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(_port);
	server_addr.sin_addr.s_addr = inet_aton(_host.c_str(), &server_addr.sin_addr) ? server_addr.sin_addr.s_addr : htonl(INADDR_ANY);
	
	// Bind do socket ao endereço e porta
	if (bind(_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw std::runtime_error("Error: Could not bind socket to port " + std::to_string(_port));
	
	// Começar a escutar por conexões
	if (listen(_socket, SOMAXCONN) < 0)
		throw std::runtime_error("Error: Could not listen on socket");
	
	std::cout << "Server listening on " << _host << ":" << _port << std::endl;
}

// Inicia o loop principal do servidor
// - Cria o socket
// - Faz bind e listen
// - Loop infinito com select para múltiplas conexões
// - Aceita conexões e processa clientes
void Server::run()
{
	try
	{
		// Setup do socket (bind, listen)
		_setup_socket();
		
		std::cout << "Server is running. Waiting for connections..." << std::endl;
		
		// Loop principal do servidor
		while (true)
		{
			// Criar set de file descriptors para select()
			fd_set read_fds;
			FD_ZERO(&read_fds);
			FD_SET(_socket, &read_fds);
			
			int max_fd = _socket;
			
			// Adicionar sockets dos clientes ao set
			for (size_t i = 0; i < _clients.size(); ++i)
			{
				int client_socket = _clients[i].get_socket();
				FD_SET(client_socket, &read_fds);
				if (client_socket > max_fd)
					max_fd = client_socket;
			}
			
			// Timeout de 5 segundos
			struct timeval timeout;
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
			
			// select() retorna quando há atividade
			int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
			
			if (activity < 0)
				throw std::runtime_error("Error: select() failed");
			
			// Verificar se há nova conexão no socket principal
			if (FD_ISSET(_socket, &read_fds))
				accept_connection();
			
			// Verificar atividade nos sockets dos clientes
			for (int i = _clients.size() - 1; i >= 0; --i)
			{
				int client_socket = _clients[i].get_socket();
				if (FD_ISSET(client_socket, &read_fds))
					handle_client(_clients[i]);
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		close_server();
		throw;
	}
}

// Aceita uma nova conexão de cliente
// - Usa accept() para receber conexões
// - Cria novo objeto Client
// - Adiciona à lista de clientes
void Server::accept_connection()
{
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	
	int client_socket = accept(_socket, (struct sockaddr*)&client_addr, &client_addr_len);
	
	if (client_socket < 0)
	{
		std::cerr << "Error: Could not accept connection" << std::endl;
		return;
	}
	
	// Obter IP do cliente
	std::string client_ip = inet_ntoa(client_addr.sin_addr);
	
	std::cout << "New connection accepted from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
	
	// Criar novo cliente e adicionar à lista
	_clients.push_back(Client(client_socket, client_ip));
}

// Processa requisição de um cliente
// - Recebe dados do cliente
// - Faz parse da requisição HTTP
// - Gera resposta apropriada
// - Envia resposta ao cliente
void Server::handle_client(Client& client)
{
	try
	{
		// Receber dados do cliente
		client.receive_data();
		
		// Verificar se a requisição está completa
		if (client.is_request_complete())
		{
			// TODO: Processar a requisição (parse, geração de resposta)
			
			// Enviar resposta ao cliente
			client.send_response();
			
			std::cout << "Response sent to client " << client.get_ip() << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error handling client: " << e.what() << std::endl;
	}
}

// Fecha o servidor de forma segura
// - Fecha todos os sockets de clientes
// - Fecha o socket principal
// - Libera recursos
void Server::close_server()
{
	// Fechar todos os sockets de clientes
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
	
	std::cout << "Server closed successfully" << std::endl;
}
