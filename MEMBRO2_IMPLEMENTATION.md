# Implementação: HTTP Request/Response & Parsing

## Resumo das Funcionalidades Implementadas

Este documento descreve as funcionalidades implementadas para o **Membro 2** do projeto WebServer, responsável pelo parsing de requisições HTTP e geração de respostas.

---

## 1. Classe Request - Parsing de Requisições HTTP

### Arquivo: `srcs/Request.cpp`

#### Funcionalidades Implementadas:

✅ **Parse de Método HTTP**
- Suporte para GET, POST, DELETE
- Normalização para uppercase
- Validação de métodos

✅ **Parse de URI**
- Extração do caminho solicitado
- Decodificação de URL (percent-encoding)
- Tratamento de query strings

✅ **Parse de Headers**
- Case-insensitive header keys
- Suporte para todos os headers HTTP padrão
- Parsing de Content-Length, Content-Type, Transfer-Encoding, etc.

✅ **Parse de Body**
- Leitura baseada em Content-Length
- **Suporte a Chunked Transfer Encoding**
  - Decodificação de chunks hexadecimais
  - Reconstrução do body completo
- Validação de tamanho do body

✅ **Validação de Requisições**
- Verificação de completude
- Validação de método e URI
- Verificação de Content-Length vs tamanho real

### Métodos Públicos:
```cpp
void parse(const std::string& raw_request);
std::string get_method() const;
std::string get_uri() const;
std::string get_version() const;
std::string get_header(const std::string& key) const;
std::string get_body() const;
bool is_complete() const;
```

---

## 2. Classe Response - Geração de Respostas HTTP

### Arquivo: `srcs/Response.cpp`

#### Funcionalidades Implementadas:

✅ **Geração de Status Codes**
- 2xx Success (200 OK, 201 Created, 204 No Content)
- 3xx Redirection (301, 302, 304)
- 4xx Client Error (400, 403, 404, 405, 413)
- 5xx Server Error (500, 501, 502, 503)

✅ **Construção de Headers**
- Content-Type automático baseado em extensão
- Content-Length calculado automaticamente
- Headers padrão (Server, Connection)
- Headers customizáveis

✅ **Servir Arquivos Estáticos**
- Leitura de arquivos do sistema
- Detecção automática de MIME types
- Verificação de permissões (403 Forbidden)
- Suporte a arquivos binários

✅ **Páginas de Erro Padrão**
- Páginas HTML personalizadas (www/error_pages/)
- Fallback para páginas de erro geradas dinamicamente
- Styling CSS para melhor apresentação

✅ **Listagem de Diretórios**
- Geração automática de HTML
- Listagem de arquivos e subdiretórios
- Links clicáveis para navegação
- Distinção visual entre arquivos e diretórios
- Link para diretório pai

✅ **Métodos HTTP Suportados**
- **GET**: Servir arquivos e diretórios
- **POST**: Criar/salvar arquivos no servidor
- **DELETE**: Remover arquivos

### Métodos Públicos:
```cpp
void generate(const Request& request, const std::string& root);
void set_status(int code);
void set_header(const std::string& key, const std::string& value);
void set_body(const std::string& body);
std::string get_response() const;
std::string get_status_message(int code) const;
```

### Métodos Privados:
```cpp
void _handle_get(const Request& request, const std::string& root);
void _handle_post(const Request& request, const std::string& root);
void _handle_delete(const Request& request, const std::string& root);
void _serve_file(const std::string& filepath);
void _serve_directory_listing(const std::string& dirpath, const std::string& uri);
void _serve_error_page(int code);
std::string _generate_directory_html(const std::string& dirpath, const std::string& uri);
```

---

## 3. Utilitários - Classe Utils

### Arquivo: `srcs/Utils.cpp`

#### Funcionalidades Implementadas:

✅ **Manipulação de Strings**
- `split()`: Divide string por delimitador
- `trim()`: Remove espaços em branco
- `to_lower()`: Converte para minúsculas

✅ **Tipos MIME**
- `get_file_extension()`: Extrai extensão de arquivo
- `get_content_type()`: Retorna MIME type
- Suporte extensivo para tipos:
  - Text: html, css, js, txt, xml, csv
  - Images: jpg, png, gif, bmp, ico, svg, webp
  - Application: json, pdf, zip, tar, gz, doc, xls
  - Audio/Video: mp3, wav, ogg, mp4, avi, webm

✅ **Gestão de Recursos**
- `file_exists()`: Verifica existência de arquivo/diretório
- `is_directory()`: Verifica se caminho é diretório
- `read_file()`: Lê conteúdo completo de arquivo
- `url_decode()`: Decodifica URLs (percent-encoding)

---

## 4. Fluxo de Processamento

### Requisição HTTP → Resposta HTTP

```
1. Cliente envia requisição
   ↓
2. Client::receive_data() recebe dados brutos
   ↓
3. Client::_check_request_complete() verifica completude
   ↓
4. Request::parse() faz parsing completo
   ↓
5. Response::generate() gera resposta apropriada
   ↓
6. Response::get_response() formata resposta HTTP
   ↓
7. Client::send_response() envia ao cliente
```

---

## 5. Exemplos de Uso

### Exemplo 1: GET de arquivo estático
```
GET /index.html HTTP/1.1
Host: localhost:8080

→ Resposta:
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234
Server: WebServer/1.0
Connection: close

<!DOCTYPE html>...
```

### Exemplo 2: Listagem de diretório
```
GET / HTTP/1.1
Host: localhost:8080

→ Resposta: HTML com listagem de arquivos
```

### Exemplo 3: POST de arquivo
```
POST /upload/file.txt HTTP/1.1
Content-Length: 13

Hello, World!

→ Resposta:
HTTP/1.1 201 Created
Content-Type: text/html
...
```

### Exemplo 4: DELETE de arquivo
```
DELETE /upload/file.txt HTTP/1.1

→ Resposta:
HTTP/1.1 200 OK
Content-Type: text/html
...
```

### Exemplo 5: Erro 404
```
GET /naoexiste.html HTTP/1.1

→ Resposta:
HTTP/1.1 404 Not Found
Content-Type: text/html
...
```

---

## 6. Verificação de Permissões

### Implementado:
- ✅ Verificação de leitura de arquivos (403 se não acessível)
- ✅ Verificação de existência (404 se não existe)
- ✅ Proteção contra DELETE de diretórios
- ✅ Validação de diretórios pai para POST

---

## 7. Redirecionamentos HTTP

### Suporte:
- Status codes 301 (Moved Permanently) e 302 (Found) implementados
- Infraestrutura pronta para redirecionamentos
- Pode ser configurado através de `set_status()` e headers `Location`

---

## 8. Tratamento de Erros

### Erros Tratados:
- **400** Bad Request: Requisição malformada
- **403** Forbidden: Sem permissão de acesso
- **404** Not Found: Recurso não encontrado
- **405** Method Not Allowed: Método não suportado
- **413** Payload Too Large: Body muito grande
- **500** Internal Server Error: Erro no servidor
- **501** Not Implemented: Método não implementado

---

## 9. Características Especiais

### Chunked Transfer Encoding
- Parsing completo de chunks hexadecimais
- Reconstrução automática do body
- Suporte a múltiplos chunks

### Directory Listing
- HTML gerado dinamicamente
- Estilização CSS integrada
- Navegação por links
- Distinção visual entre arquivos e diretórios

### MIME Type Detection
- Mais de 30 tipos MIME suportados
- Fallback para `application/octet-stream`
- Detecção automática por extensão

### URL Decoding
- Suporte a percent-encoding (%20, etc.)
- Conversão de '+' para espaço
- Parsing de caracteres especiais

---

## 10. Integração com Server/Client

As classes Request e Response estão prontas para integração com:
- `Client::get_request()`: Retorna referência para Request
- `Client::get_response()`: Retorna referência para Response
- `Client::set_response_buffer()`: Define resposta para envio

### Uso Típico:
```cpp
// No processamento do cliente
if (client.is_request_complete())
{
    // Parse da requisição
    client.get_request().parse(client.get_buffer());
    
    // Gerar resposta
    client.get_response().generate(client.get_request(), "www");
    
    // Preparar para envio
    client.set_response_buffer(client.get_response().get_response());
}
```

---

## 11. Compilação e Teste

### Compilar o projeto:
```bash
make
```

### Executar o servidor:
```bash
./webserver config/default.conf
```

### Testar com curl:
```bash
# GET
curl http://localhost:8080/

# POST
curl -X POST -d "Hello" http://localhost:8080/test.txt

# DELETE
curl -X DELETE http://localhost:8080/test.txt
```

---

## 12. Checklist de Funcionalidades

### Parsing de Requisições:
- [x] Parse de method (GET, POST, DELETE)
- [x] Parse de headers (case-insensitive)
- [x] Parse de body (incluindo chunked transfer)
- [x] Validação de requisições

### Geração de Respostas:
- [x] Geração de status codes corretos
- [x] Construção de headers de resposta
- [x] Servir arquivos estáticos
- [x] Páginas de erro padrão
- [x] Listagem de diretórios

### Gestão de Recursos:
- [x] Localização de arquivos no sistema
- [x] Verificação de permissões
- [x] Tipos MIME
- [x] Redirecionamentos HTTP (estrutura pronta)

---

## Autor

**Membro 2**: Responsável por HTTP Request/Response & Parsing

**Data**: Fevereiro 2026

**Status**: ✅ Implementação Completa
