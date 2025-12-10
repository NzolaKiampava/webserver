#include "../includes/Server.hpp"
#include "../includes/Config.hpp"
#include <iostream>

// Construtor: Inicializa o servidor com arquivo de configuração
// - Carrega as configurações do arquivo
// - Define porta, host e outras propriedades
Server::Server(const std::string& configFile) : _socket(-1), _port(8080), _host("localhost")
{
	// TODO: Implementar carregamento da configuração
}

// Destrutor: Limpa recursos e fecha o servidor
Server::~Server()
{
	// TODO: Fechar o servidor de forma segura
}

// Getter: Retorna o socket do servidor
int Server::get_socket() const
{
	// TODO: Retornar o descritor de socket
}

// Getter: Retorna a porta em que o servidor está escutando
int Server::get_port() const
{
	// TODO: Retornar a porta configurada
}

// Inicia o loop principal do servidor
// - Cria o socket
// - Faz bind e listen
// - Loop infinito com select/poll/epoll para múltiplas conexões
// - Aceita conexões e processa clientes
void Server::run()
{
	// TODO: Implementar loop principal do servidor
}

// Aceita uma nova conexão de cliente
// - Usa accept() para receber conexões
// - Cria novo objeto Client
// - Adiciona à lista de clientes
void Server::accept_connection()
{
	// TODO: Implementar aceitação de conexões
}

// Processa requisição de um cliente
// - Recebe dados do cliente
// - Faz parse da requisição HTTP
// - Gera resposta apropriada
// - Envia resposta ao cliente
void Server::handle_client(Client& client)
{
	// TODO: Implementar manipulação de requisições
}

// Fecha o servidor de forma segura
// - Fecha todos os sockets de clientes
// - Fecha o socket principal
// - Libera recursos
void Server::close_server()
{
	// TODO: Fechar servidor e liberar recursos
}
