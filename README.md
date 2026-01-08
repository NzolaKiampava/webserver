# WebServer - Documentação Técnica

Um servidor web HTTP de alta performance implementado em C++98, com suporte a múltiplas conexões simultâneas usando multiplexação de I/O com `select()`.

## 📋 Índice

1. [Visão Geral](#visão-geral)
2. [Arquitetura](#arquitetura)
3. [Componentes Principais](#componentes-principais)
4. [Conceitos Técnicos](#conceitos-técnicos)
5. [Fluxo de Execução](#fluxo-de-execução)
6. [Estrutura do Projeto](#estrutura-do-projeto)
7. [Como Compilar e Executar](#como-compilar-e-executar)
8. [Configuração](#configuração)

---

## 🎯 Visão Geral

O WebServer é um servidor HTTP não-bloqueante que utiliza:

- **Sockets Unix** para comunicação de rede
- **Multiplexação de I/O** com `select()` para gerenciar múltiplas conexões
- **Arquitetura orientada a objetos** em C++
- **Protocolo HTTP/1.1** para comunicação com clientes
- **Suporte a CGI** para execução de scripts dinâmicos

**Principais características:**

- ✅ Aceita múltiplas conexões simultâneas
- ✅ Configurável via arquivo de configuração
- ✅ Suporte a rotas e páginas de erro
- ✅ Processamento de requisições HTTP
- ✅ Gerenciamento automático de recursos

---

## 🏗️ Arquitetura

```
┌─────────────────────────────────────────────────────────────┐
│                          main()                              │
│              Inicializa o servidor e loop                   │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
        ┌──────────────────────┐
        │   Server (run())     │
        │  Loop Principal      │
        │  select() Multiplex  │
        └────┬──────────────┬──┘
             │              │
      ┌──────▼──────┐  ┌────▼──────────┐
      │ accept()    │  │ handle_client()│
      │ Nova conex. │  │ Processa req.  │
      └─────────────┘  └────┬───────────┘
                             │
                    ┌────────┴────────┐
                    │                 │
              ┌─────▼─────┐    ┌──────▼──────┐
              │ Request   │    │  Response   │
              │ Parse HTTP│    │ Gera resposta│
              └───────────┘    └─────────────┘
```

---

## 🔧 Componentes Principais

### 1. **Server (Server.hpp / Server.cpp)**

Classe responsável pelo gerenciamento do servidor, sockets e loop principal.

**Atributos principais:**
```cpp
class Server {
    int _socket;                    // File descriptor do socket principal
    int _port;                      // Porta de escuta (ex: 8080)
    std::string _host;              // Endereço IP de escuta (ex: 127.0.0.1)
    std::vector<Client> _clients;   // Vetor de clientes conectados
    Config _config;                 // Configurações do servidor
};
```

**Métodos principais:**

| Método | Descrição |
|--------|-----------|
| `Server(configFile)` | Construtor - inicializa com arquivo de config |
| `run()` | Loop principal com select() |
| `_setup_socket()` | Cria socket, bind e listen |
| `accept_connection()` | Aceita nova conexão de cliente |
| `handle_client()` | Processa requisição do cliente |
| `close_server()` | Encerra servidor e libera recursos |

### 2. **Client (Client.hpp / Client.cpp)**

Representa uma conexão de cliente.

**Atributos:**
```cpp
class Client {
    int _socket;              // File descriptor da conexão
    std::string _ip_address;  // IP do cliente
    Request _request;         // Objeto de requisição HTTP
    Response _response;       // Objeto de resposta HTTP
    std::string _buffer;      // Buffer de dados recebidos
};
```

### 3. **Config (Config.hpp / Config.cpp)**

Gerencia configurações do servidor lidas de arquivo.

**Funcionalidades:**
- Parse de arquivo de configuração
- Definição de porta, host, diretório raiz
- Configuração de páginas de erro
- Suporte a índices padrão

### 4. **Request (Request.hpp / Request.cpp)**

Parse e análise de requisições HTTP.

**Processa:**
- Método HTTP (GET, POST, PUT, DELETE, etc)
- URI e parâmetros
- Headers HTTP
- Body da requisição

### 5. **Response (Response.hpp / Response.cpp)**

Geração de respostas HTTP.

**Implementa:**
- Status codes (200, 404, 500, etc)
- Headers de resposta
- Body (conteúdo do arquivo)
- Compressão opcional

### 6. **CGI (CGI.hpp / CGI.cpp)**

Execução de scripts CGI.

---

## 🔌 Conceitos Técnicos

### Socket e Binding

#### O que é um Socket?

Um **socket** é um endpoint de comunicação de rede. Pense nele como um "plugue" que permite que dois processos se comuniquem pela rede.

**Tipos de socket:**
```cpp
socket(AF_INET,      // Família: IPv4
       SOCK_STREAM,  // Tipo: TCP (stream orientado a conexão)
       0)            // Protocolo: padrão
```

- `AF_INET`: Address Family Internet (IPv4)
- `SOCK_STREAM`: Transmissão confiável de sequência de bytes (TCP)
- `SOCK_DGRAM`: Datagramas sem conexão (UDP)

#### Criação do Socket

```cpp
_socket = socket(AF_INET, SOCK_STREAM, 0);
if (_socket < 0)
    throw std::runtime_error("Could not create socket");
```

Retorna um **file descriptor** (número inteiro ≥ 0) ou -1 em caso de erro.

---

### Socket Options (setsockopt)

**SO_REUSEADDR** permite reutilizar a porta imediatamente após encerramento:

```cpp
int reuse = 1;
setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
```

Sem isso, a porta permaneceria em estado TIME_WAIT por 30-120 segundos, impedindo reinicialização rápida do servidor.

---

### Bind (Ligação ao Endereço)

**Bind** liga o socket a um endereço IP e porta específicos.

#### Estrutura de Endereço (sockaddr_in)

```cpp
struct sockaddr_in {
    short sin_family;           // AF_INET para IPv4
    unsigned short sin_port;    // Porta em network byte order
    struct in_addr sin_addr;    // Endereço IP
    char sin_zero[8];           // Preenchimento (não usado)
};
```

#### Processo de Bind

```cpp
struct sockaddr_in server_addr;
std::memset(&server_addr, 0, sizeof(server_addr));

server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(8080);  // Host To Network Short (big-endian)
server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Qualquer interface

bind(_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
```

**Parâmetros:**
- `INADDR_ANY` (0.0.0.0): Escuta em todas as interfaces de rede
- `inet_aton("192.168.1.1", &addr)`: Específico para um IP

**Erros comuns:**
- `Address already in use`: Porta ocupada ou em TIME_WAIT
- `Permission denied`: Porta < 1024 requer privilégios root

---

### Listen (Colocando em Modo Escuta)

**Listen** coloca o socket em modo passivo, pronto para aceitar conexões.

```cpp
listen(_socket, SOMAXCONN);
```

**Parâmetros:**
- `SOMAXCONN`: Máximo de conexões pendentes na fila (tipicamente 128 ou 1024)

**O que acontece:**
- Cria uma fila de conexões pendentes
- Socket fica em estado LISTEN
- Pronto para `accept()` ser chamado

---

### Accept (Aceitando Conexões)

**Accept** remove uma conexão da fila e cria um novo socket para comunicação:

```cpp
struct sockaddr_in client_addr;
socklen_t client_addr_len = sizeof(client_addr);

int client_socket = accept(_socket, 
                          (struct sockaddr*)&client_addr, 
                          &client_addr_len);

if (client_socket < 0)
    throw std::runtime_error("Accept failed");
```

**O que retorna:**
- Novo **file descriptor** para comunicação com cliente
- Estrutura `client_addr` preenchida com info do cliente
- Socket original permanece escutando

**Informações do cliente:**
```cpp
std::string client_ip = inet_ntoa(client_addr.sin_addr);
int client_port = ntohs(client_addr.sin_port);
```

---

### Select (Multiplexação de I/O)

**Select** monitora múltiplos file descriptors, detectando quando há dados disponíveis.

#### Por que usar select()?

Sem select(), o servidor teria que:
- Aceitar apenas um cliente por vez
- Ficar bloqueado esperando dados
- Não conseguir servir múltiplos clientes simultaneamente

**Com select():** Monitora vários sockets e detecta atividade em qualquer um.

#### Implementação

```cpp
fd_set read_fds;           // Conjunto de file descriptors
FD_ZERO(&read_fds);        // Limpar o conjunto
FD_SET(_socket, &read_fds);// Adicionar socket servidor

// Adicionar sockets dos clientes
for (size_t i = 0; i < _clients.size(); ++i) {
    FD_SET(_clients[i].get_socket(), &read_fds);
}

struct timeval timeout;
timeout.tv_sec = 5;        // Timeout de 5 segundos
timeout.tv_usec = 0;

// Retorna quando há atividade ou timeout
int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

// Verificar qual socket tem dados
if (FD_ISSET(_socket, &read_fds))
    accept_connection();    // Nova conexão

for (size_t i = 0; i < _clients.size(); ++i) {
    if (FD_ISSET(_clients[i].get_socket(), &read_fds))
        handle_client(_clients[i]);
}
```

**Parâmetros de select():**
```cpp
int select(int nfds,              // Maior FD + 1
           fd_set *readfds,       // FDs para leitura
           fd_set *writefds,      // FDs para escrita (NULL neste caso)
           fd_set *exceptfds,     // Condições excepcionais (NULL)
           struct timeval *timeout); // Timeout (NULL = bloqueante)
```

**Retorna:**
- Número de file descriptors com atividade
- 0 se timeout ocorreu
- -1 se erro

#### Macros para manipular fd_set

```cpp
FD_ZERO(&set);          // Limpar conjunto
FD_SET(fd, &set);       // Adicionar fd
FD_CLR(fd, &set);       // Remover fd
FD_ISSET(fd, &set);     // Verificar se fd está no conjunto
```

---

### Fluxo de Dados - Envio e Recepção

#### Receber dados de cliente

```cpp
char buffer[1024];
ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

if (bytes_read < 0)
    throw std::runtime_error("recv() failed");

if (bytes_read == 0)
    // Cliente fechou conexão
    close(client_socket);

buffer[bytes_read] = '\0';  // Null-terminate
```

#### Enviar resposta para cliente

```cpp
std::string response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: 13\r\n"
                       "\r\n"
                       "Hello, World!";

ssize_t bytes_sent = send(client_socket, response.c_str(), response.size(), 0);

if (bytes_sent < 0)
    throw std::runtime_error("send() failed");
```

---

### Network Byte Order

Computadores podem usar diferentes **endianness** (ordem de bytes):

- **Big-endian**: Byte mais significativo primeiro (padrão de rede)
- **Little-endian**: Byte menos significativo primeiro (x86/x64)

**Funções de conversão:**
```cpp
htons(value);  // Host To Network Short (16-bit)
htonl(value);  // Host To Network Long (32-bit)
ntohs(value);  // Network To Host Short
ntohl(value);  // Network To Host Long
```

**Exemplo:**
```cpp
server_addr.sin_port = htons(8080);  // Converter 8080 para network order
int port = ntohs(client_addr.sin_port);  // Converter de volta
```

---

## 📊 Fluxo de Execução

### 1. Inicialização

```
main()
  │
  └─> Server(configFile)
       │
       └─> Config::parse_config()
           ├─ Ler arquivo
           ├─ Extrair porta, host
           └─ Retornar configuração
```

### 2. Execução Principal (server.run())

```
run()
  │
  ├─> _setup_socket()
  │    ├─ socket() - criar socket
  │    ├─ setsockopt() - opções
  │    ├─ bind() - ligar a porta
  │    ├─ listen() - modo escuta
  │    └─ Pronto!
  │
  └─> while (true) - Loop infinito
       │
       ├─> select() - Esperar atividade (timeout 5s)
       │    │
       │    ├─ Socket servidor tem nova conexão?
       │    │   └─> accept_connection()
       │    │
       │    └─ Clientes têm dados?
       │        └─> handle_client()
       │
       └─ [Continua esperando...]
```

### 3. Aceitar Conexão

```
accept_connection()
  │
  ├─> accept(_socket, &client_addr, &len)
  │    └─ Cria novo socket de comunicação
  │
  ├─> inet_ntoa(client_addr.sin_addr) - Extrair IP
  │
  ├─> Client(client_socket, ip) - Criar cliente
  │
  └─> _clients.push_back() - Adicionar à lista
```

### 4. Processar Cliente

```
handle_client(client)
  │
  ├─> client.receive_data()
  │    ├─ recv() - Ler dados do socket
  │    └─ Buffer com HTTP request
  │
  ├─ client.is_request_complete() - Requisição completa?
  │    │
  │    ├─> Request::parse() - Parser HTTP
  │    │    ├─ Extrair método (GET, POST, etc)
  │    │    ├─ Extrair URI
  │    │    ├─ Extrair headers
  │    │    └─ Extrair body
  │    │
  │    └─> Response::generate() - Gerar resposta
  │         ├─ Status code (200, 404, etc)
  │         ├─ Headers
  │         └─ Body (arquivo ou CGI)
  │
  └─> client.send_response()
       └─ send() - Enviar dados via socket
```

### 5. Encerramento

```
Ctrl+C (SIGINT)
  │
  └─> close_server()
       │
       ├─ Fechar todos os sockets de clientes
       │
       ├─ Fechar socket principal
       │
       └─ Liberar recursos
```

---

## 📁 Estrutura do Projeto

```
webserver/
├── README.md                 # Este arquivo
├── Makefile                  # Script de compilação
├── includes/                 # Headers (.hpp)
│   ├── Server.hpp           # Classe do servidor
│   ├── Client.hpp           # Classe de cliente
│   ├── Config.hpp           # Classe de configuração
│   ├── Request.hpp          # Parser de requisição HTTP
│   ├── Response.hpp         # Gerador de resposta HTTP
│   ├── CGI.hpp              # Executor de scripts CGI
│   └── Utils.hpp            # Utilitários
├── srcs/                     # Implementações (.cpp)
│   ├── main.cpp             # Ponto de entrada
│   ├── Server.cpp           # Implementação Server
│   ├── Client.cpp           # Implementação Client
│   ├── Config.cpp           # Implementação Config
│   ├── Request.cpp          # Implementação Request
│   ├── Response.cpp         # Implementação Response
│   ├── CGI.cpp              # Implementação CGI
│   └── Utils.cpp            # Implementação Utils
├── config/                   # Arquivos de configuração
│   ├── default.conf         # Configuração padrão
│   └── test.conf            # Configuração de teste
├── cgi-bin/                 # Scripts CGI
│   └── test.py              # Script Python de teste
└── www/                      # Diretório raiz (webroot)
    ├── index.html           # Página inicial
    ├── error_pages/         # Páginas de erro
    │   ├── 403.html         # Forbidden
    │   ├── 404.html         # Not Found
    │   └── 500.html         # Internal Server Error
    └── uploads/             # Diretório de uploads
```

---

## 🛠️ Como Compilar e Executar

### Compilação

```bash
# Compilar o projeto
make

# Limpeza de objetos
make clean

# Limpeza completa (remover binário)
make fclean

# Recompilar tudo
make re
```

### Execução

```bash
# Executar com configuração padrão
make run

# Ou diretamente
./webserv config/default.conf

# Executar com configuração de teste
make test

# Ou diretamente
./webserv config/test.conf
```

### Teste do Servidor

```bash
# Terminal 1: Iniciar servidor
./webserv config/default.conf

# Terminal 2: Fazer requisição
curl http://localhost:8080/
curl http://localhost:8080/index.html
curl -X POST http://localhost:8080/upload
```

---

## ⚙️ Configuração

### Arquivo de Configuração (config/default.conf)

```nginx
server {
    listen 8080;
    host 127.0.0.1;
    
    root www;
    index index.html;
    
    autoindex on;
    
    error_page 403 error_pages/403.html;
    error_page 404 error_pages/404.html;
    error_page 500 error_pages/500.html;
    
    # Rotas
    location / {
        methods GET POST DELETE;
    }
    
    location /cgi-bin/ {
        methods GET POST;
        cgi_path cgi-bin/;
    }
    
    location /upload/ {
        methods POST;
        max_body_size 10M;
        upload_path uploads/;
    }
}
```

---

## 🐛 Troubleshooting

### Erro: "Address already in use"

**Causa:** Socket anterior ainda em TIME_WAIT
**Solução:** Aguarde 30-120s ou use `SO_REUSEADDR` (já implementado)

### Servidor não recebe conexões

**Verifique:**
- Porta correta no arquivo de configuração
- Firewall não está bloqueando
- IP correto (localhost vs 0.0.0.0)

### Cliente não recebe resposta

**Verifique:**
- Response headers corretos (Content-Length)
- Fim de linha correto (\r\n\r\n)
- Socket não foi fechado prematuramente

### Vazamento de memória

**Use valgrind:**
```bash
valgrind --leak-check=full ./webserv config/default.conf
```

---

## 📚 Recursos e Referências

### Funções Socket (man pages)

```bash
man socket           # Criar socket
man bind             # Ligar a endereço
man listen           # Colocar em escuta
man accept           # Aceitar conexão
man select           # Multiplexação I/O
man send             # Enviar dados
man recv             # Receber dados
man setsockopt       # Opções de socket
man inet_aton        # Converter IP string
```

### Protocolos

- **HTTP/1.1**: RFC 7230-7235
- **TCP**: RFC 793
- **IPv4**: RFC 791

### C++ References

- `<sys/socket.h>`: Socket functions
- `<netinet/in.h>`: IPv4 structures
- `<arpa/inet.h>`: Network utilities
- `<sys/select.h>`: Select function
- `<unistd.h>`: POSIX API (read, write, close)

---

## 📝 Licença

Este projeto foi desenvolvido como exercício educacional.

---

**Última atualização:** Janeiro 2026

