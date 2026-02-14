# Implementação: Configuration & CGI

## Resumo das Funcionalidades Implementadas

Este documento descreve as funcionalidades implementadas para o **Membro 3** do projeto WebServer, responsável por parsing de configuração e execução de CGI.

---

## 1. Classe Config - Parser de Configuração

### Arquivo: `srcs/Config.cpp`

#### Funcionalidades Implementadas:

✅ **Parsing de Arquivo .conf (estilo NGINX)**
- Suporte a múltiplos blocos `server`
- Parsing de blocos `location` com rotas específicas
- Comentários (#) ignorados
- Estrutura hierárquica mantida

✅ **Configurações de Servidor**
- `listen`: Porta do servidor (8080, 80, etc.)
- `server_name`: Nome do servidor virtual
- `host`: IP/hostname para bind (0.0.0.0, localhost)
- `root`: Document root do servidor
- `index`: Arquivos índice padrão (index.html, index.php)
- `autoindex`: Habilitar/desabilitar listagem de diretórios
- `error_page`: Páginas de erro customizadas por código
- `client_max_body_size`: Tamanho máximo de upload (100M, 1G, etc.)
- `timeout`: Timeout de requisições em segundos
- `cgi_timeout`: Timeout específico para CGI

✅ **Configurações de Location (Rotas)**
- Matching por prefixo (`location /uploads`)
- Matching por regex (`location ~ \.py$`)
- `allow`: Métodos HTTP permitidos (GET, POST, DELETE, all)
- `root`: Document root específico para a rota
- `autoindex`: Override de autoindex para a rota
- `return`: Redirecionamento HTTP
- `cgi on/off`: Habilitar CGI para a rota
- `cgi_path`: Caminho do interpretador (/usr/bin/python3)
- `upload_dir`: Diretório específico para uploads

✅ **Múltiplos Servidores**
- Suporte a múltiplos blocos `server` em um arquivo
- Cada servidor com porta e configurações independentes
- Virtual hosting básico

✅ **Validação de Configuração**
- Verificação de portas válidas (1-65535)
- Validação de document root
- Detecção de erros de sintaxe
- Throw exceptions em caso de erro

### Estruturas de Dados:

```cpp
struct LocationConfig {
    std::string path;
    std::vector<std::string> allowed_methods;
    std::string root;
    bool autoindex;
    std::string redirect;
    std::vector<std::string> index_files;
    bool cgi_enabled;
    std::string cgi_path;
    std::string cgi_extension;
    std::string upload_dir;
};

struct ServerConfig {
    int port;
    std::string host;
    std::string server_name;
    std::string root;
    std::vector<std::string> index_files;
    std::map<int, std::string> error_pages;
    bool autoindex;
    size_t client_max_body_size;
    int timeout;
    int cgi_timeout;
    std::vector<LocationConfig> locations;
};
```

### Métodos Públicos:

```cpp
// Parsing
void parse_config(const std::string& configFile);
void validate_config() const;

// Acesso a múltiplos servers
std::vector<ServerConfig> get_servers() const;
ServerConfig get_server(size_t index) const;
size_t get_server_count() const;

// Getters (usa primeiro servidor por padrão)
int get_port() const;
std::string get_host() const;
std::string get_root() const;
size_t get_max_body_size() const;
int get_timeout() const;
int get_cgi_timeout() const;

// Busca de location
LocationConfig* find_location(const std::string& uri, size_t server_index = 0);

// Validação de métodos
bool is_method_allowed(const std::string& method, const std::string& uri, size_t server_index = 0) const;
```

---

## 2. Classe CGI - Execução de Scripts

### Arquivo: `srcs/CGI.cpp`

#### Funcionalidades Implementadas:

✅ **Execução de Scripts CGI**
- Fork + execve para criar processo filho
- Suporte a múltiplos interpretadores (Python, PHP, Perl, Ruby, Shell)
- Detecção automática de interpretador por extensão
- Execução com timeout configurável

✅ **Variáveis de Ambiente CGI (padrão RFC 3875)**
- `REQUEST_METHOD`: GET, POST, DELETE
- `SCRIPT_NAME`: Caminho do script
- `SCRIPT_FILENAME`: Caminho completo do arquivo
- `QUERY_STRING`: Parâmetros GET (após ?)
- `PATH_INFO`: Informações adicionais de caminho
- `CONTENT_TYPE`: Tipo do body
- `CONTENT_LENGTH`: Tamanho do body
- `SERVER_NAME`: Nome do servidor
- `SERVER_PORT`: Porta do servidor
- `SERVER_PROTOCOL`: HTTP/1.1
- `SERVER_SOFTWARE`: WebServer/1.0
- `GATEWAY_INTERFACE`: CGI/1.1
- `REMOTE_ADDR`: IP do cliente
- `HTTP_*`: Headers HTTP convertidos

✅ **Comunicação via Pipes**
- Pipe para stdin: Envia body da requisição ao CGI
- Pipe para stdout: Captura output do CGI
- Redirecionamento correto de file descriptors
- Leitura não-bloqueante com timeout

✅ **Tratamento de Timeout**
- Timeout configurável (padrão: 5 segundos)
- Monitoramento do processo filho
- Kill automático (SIGKILL) se exceder timeout
- Cleanup correto de recursos

✅ **Validação de Scripts**
- Verificação de existência do arquivo
- Validação de permissões de leitura/execução
- Verificação de tipo de arquivo (regular file)
- Segurança básica contra scripts maliciosos

✅ **Dechunking para CGI**
- Método estático para decodificar chunked transfer
- Reconstrução do body original
- Necessário quando request vem com Transfer-Encoding: chunked

### Métodos Públicos:

```cpp
// Construtores
CGI(const std::string& script_path, const Request& request);
CGI(const std::string& script_path, const std::string& interpreter, const Request& request);

// Configuração
void setup_environment(const Request& request);
void set_interpreter(const std::string& interpreter_path);
void set_timeout(int seconds);

// Execução
std::string execute();
std::string execute_with_body(const std::string& body);

// Validação e utilitários
bool is_valid_script(const std::string& path) const;
std::string get_interpreter_for_extension(const std::string& extension) const;
static std::string dechunk_body(const std::string& chunked_body);
```

### Interpretadores Suportados:

| Extensão | Interpretador | Caminho Padrão |
|----------|--------------|----------------|
| .py | Python | /usr/bin/python3 |
| .php | PHP CGI | /usr/bin/php-cgi |
| .pl | Perl | /usr/bin/perl |
| .rb | Ruby | /usr/bin/ruby |
| .sh | Shell | /bin/sh |

---

## 3. Upload de Arquivos

### Arquivo: `srcs/Response.cpp` (atualizado)

#### Funcionalidades Implementadas:

✅ **Processamento de POST com Arquivos**
- Parsing de multipart/form-data
- Extração de boundary
- Identificação de partes do formulário
- Extração de filenames

✅ **Salvamento em Diretório Configurado**
- Uso do `upload_dir` da configuração de location
- Criação de arquivo no servidor
- Gravação binária (suporta qualquer tipo de arquivo)

✅ **Validação de Tamanho Máximo**
- Verificação contra `client_max_body_size`
- Retorno de 413 (Payload Too Large) se exceder
- Validação antes do processing completo

✅ **Método DELETE para Remoção**
- Remoção de arquivos do servidor
- Validação de existência
- Proteção contra delete de diretórios
- Retorno de 200 OK em sucesso

### Novos Métodos:

```cpp
// Upload
void generate_with_config(const Request& request, const std::string& root, size_t max_body_size);
void set_max_body_size(size_t size);
bool _handle_file_upload(const Request& request, const std::string& upload_dir);
std::string _parse_multipart_filename(const std::string& content_disposition);
std::string _extract_file_content(const std::string& body, const std::string& boundary);
```

---

## 4. Exemplos de Configuração

### Configuração Simples (`config/default.conf`):

```nginx
server {
    listen 8080;
    server_name localhost;
    root ./www;
    index index.html;
    
    autoindex off;
    
    error_page 404 /error_pages/404.html;
    error_page 403 /error_pages/403.html;
    error_page 500 /error_pages/500.html;
    
    client_max_body_size 100M;
    
    location / {
        allow all;
    }
    
    location /uploads {
        allow POST;
        allow GET;
    }
    
    location ~ \.py$ {
        cgi on;
        cgi_path /usr/bin/python3;
    }
}
```

### Configuração Avançada (`config/advanced.conf`):

```nginx
# Servidor principal
server {
    listen 8080;
    server_name localhost;
    host 0.0.0.0;
    
    root ./www;
    index index.html index.htm;
    
    autoindex on;
    
    error_page 404 /error_pages/404.html;
    error_page 403 /error_pages/403.html;
    error_page 500 /error_pages/500.html;
    
    client_max_body_size 100M;
    timeout 60;
    cgi_timeout 10;
    
    location / {
        allow GET;
        allow POST;
        allow DELETE;
        autoindex on;
    }
    
    location /uploads {
        allow POST;
        allow GET;
        allow DELETE;
        upload_dir ./www/uploads;
        autoindex on;
    }
    
    location ~ \.py$ {
        allow GET;
        allow POST;
        cgi on;
        cgi_path /usr/bin/python3;
    }
    
    location ~ \.php$ {
        allow GET;
        allow POST;
        cgi on;
        cgi_path /usr/bin/php-cgi;
    }
    
    location /cgi-bin {
        allow GET;
        allow POST;
        cgi on;
        cgi_path /usr/bin/python3;
    }
}

# Servidor adicional (API)
server {
    listen 8081;
    server_name api.localhost;
    host 0.0.0.0;
    
    root ./www/api;
    index index.json;
    
    autoindex off;
    
    error_page 404 /error.json;
    
    client_max_body_size 10M;
    timeout 30;
    cgi_timeout 5;
    
    location / {
        allow GET;
        allow POST;
    }
    
    location /uploads {
        allow POST;
        upload_dir ./www/api/uploads;
    }
}
```

---

## 5. Scripts CGI de Teste

### `cgi-bin/env_info.py` - Informações de Ambiente

Mostra todas as variáveis de ambiente CGI e informações do sistema.

**URL:** `http://localhost:8080/cgi-bin/env_info.py`

**Funcionalidades:**
- Display de variáveis CGI
- Informações do servidor
- Informações do cliente
- Versão do Python
- Processamento de POST data

### `cgi-bin/form_handler.py` - Processador de Formulários

Processa formulários GET e POST.

**URL:** `http://localhost:8080/cgi-bin/form_handler.py`

**Funcionalidades:**
- Parse de query strings
- Processamento de POST data
- Formulários HTML integrados
- Display de parâmetros recebidos

---

## 6. Fluxo de Execução CGI

```
1. Request chega com URI para script CGI
   ↓
2. Server identifica que URI match location com cgi_enabled
   ↓
3. CGI::setup_environment() configura variáveis de ambiente
   ↓
4. fork() cria processo filho
   ↓
5. Processo filho:
   - Redireciona stdin (para receber body)
   - Redireciona stdout (para capturar output)
   - execve() executa interpretador + script
   ↓
6. Processo pai:
   - Escreve body no stdin do filho
   - Lê output do stdout do filho (com timeout)
   - Aguarda término do filho (com timeout)
   ↓
7. Output do CGI é enviado como resposta HTTP
```

---

## 7. Fluxo de Upload de Arquivo

```
1. Client envia POST com multipart/form-data
   ↓
2. Server verifica Content-Length vs client_max_body_size
   ↓
3. Se exceder → 413 Payload Too Large
   ↓
4. Parse do body multipart:
   - Extrai boundary
   - Identifica partes
   - Parse headers de cada parte
   - Extrai filename do Content-Disposition
   - Extrai conteúdo binário
   ↓
5. Salva arquivo em upload_dir
   ↓
6. Retorna 201 Created
```

---

## 8. Exemplos de Uso

### Executar Script CGI (GET):

```bash
curl http://localhost:8080/cgi-bin/env_info.py
```

### Executar Script CGI com Query String:

```bash
curl "http://localhost:8080/cgi-bin/form_handler.py?name=John&email=john@example.com"
```

### Executar Script CGI (POST):

```bash
curl -X POST -d "username=alice&age=25" http://localhost:8080/cgi-bin/form_handler.py
```

### Upload de Arquivo:

```bash
curl -X POST -F "file=@myfile.txt" http://localhost:8080/uploads/
```

### Upload com HTML Form:

```html
<form action="/uploads/" method="POST" enctype="multipart/form-data">
    <input type="file" name="file">
    <button type="submit">Upload</button>
</form>
```

---

## 9. Integração com Server

### Uso Típico no Loop Principal:

```cpp
// Carregar configuração
Config config("config/default.conf");

// Para cada cliente
if (client.is_request_complete())
{
    Request& request = client.get_request();
    request.parse(client.get_buffer());
    
    // Buscar location apropriado
    LocationConfig* location = config.find_location(request.get_uri());
    
    // Verificar método permitido
    if (!config.is_method_allowed(request.get_method(), request.get_uri()))
    {
        // 405 Method Not Allowed
        response.set_status(405);
        response.set_header("Allow", "GET, POST");
        // ...
    }
    else if (location && location->cgi_enabled)
    {
        // Executar CGI
        CGI cgi(script_path, location->cgi_path, request);
        cgi.set_timeout(config.get_cgi_timeout());
        
        try {
            std::string output = cgi.execute();
            client.set_response_buffer(output);
        }
        catch (const std::exception& e) {
            // CGI error ou timeout
            response.set_status(500);
            // ...
        }
    }
    else
    {
        // Processamento normal
        response.generate_with_config(
            request,
            config.get_root(),
            config.get_max_body_size()
        );
        
        client.set_response_buffer(response.get_response());
    }
}
```

---

## 10. Checklist de Funcionalidades

### Parser de Configuração:
- [x] Leitura de arquivo .conf
- [x] Parse de múltiplos blocos server
- [x] Parse de blocos location
- [x] Validação de configurações
- [x] Suporte a comentários
- [x] Parse de tamanhos (100M, 1G)
- [x] Error pages customizadas
- [x] Timeout configurável

### Execução de CGI:
- [x] Fork + execve
- [x] Variáveis de ambiente CGI completas
- [x] Comunicação via pipes
- [x] Timeout de execução
- [x] Suporte a múltiplos interpretadores
- [x] Validação de scripts
- [x] Dechunking para CGI

### Upload de Arquivos:
- [x] Parse de multipart/form-data
- [x] Extração de filename
- [x] Salvamento em diretório configurado
- [x] Validação de tamanho máximo
- [x] DELETE para remoção

### Rotas (Locations):
- [x] Matching por prefixo
- [x] Matching por regex (extensão)
- [x] Métodos HTTP permitidos
- [x] Upload directory específico
- [x] CGI on/off por rota
- [x] Autoindex override

---

## 11. Segurança

### Medidas Implementadas:

✅ **Validação de Tamanho**
- Limite client_max_body_size para prevenir DoS
- Verificação antes de processar body completo

✅ **Timeout de CGI**
- Previne scripts CGI infinitos
- Kill automático após timeout
- Cleanup de recursos

✅ **Validação de Scripts**
- Verificação de tipo de arquivo
- Validação de permissões
- Apenas arquivos regulares aceitos

✅ **Validação de Métodos**
- Restrição por location
- 405 Method Not Allowed quando apropriado

✅ **Validação de Configuração**
- Portas válidas
- Document root obrigatório
- Throw exceptions em erro

---

## 12. Performance

### Otimizações:

- **Fork apenas quando necessário**: CGI só executa se enabled
- **Pipes não-bloqueantes**: Leitura com timeout eficiente
- **Caching de configuração**: Parse uma vez, use múltiplas vezes
- **Location matching eficiente**: Busca pelo match mais específico

---

## 13. Teste e Debug

### Testar Configuração:

```bash
# Carregar e validar
./webserver config/advanced.conf
```

### Testar CGI:

```bash
# Direto
python3 cgi-bin/env_info.py

# Via servidor
curl http://localhost:8080/cgi-bin/env_info.py
```

### Testar Upload:

```bash
# Criar arquivo de teste
echo "Test content" > test.txt

# Upload
curl -X POST -F "file=@test.txt" http://localhost:8080/uploads/

# Verificar
curl http://localhost:8080/uploads/test.txt

# Deletar
curl -X DELETE http://localhost:8080/uploads/test.txt
```

---

## Autor

**Membro 3**: Responsável por Configuration & CGI

**Data**: Fevereiro 2026

**Status**: ✅ Implementação Completa

---

## Conclusão

Todas as responsabilidades do Membro 3 foram implementadas com sucesso:

1. ✅ Parser robusto de configuração estilo NGINX
2. ✅ Execução completa de CGI com timeout e segurança
3. ✅ Upload de arquivos com validação
4. ✅ Suporte a múltiplos servidores e locations
5. ✅ Gestão completa de variáveis de ambiente CGI
6. ✅ Dechunking e comunicação via pipes

O sistema está pronto para executar scripts CGI, processar uploads e gerenciar configurações complexas!
