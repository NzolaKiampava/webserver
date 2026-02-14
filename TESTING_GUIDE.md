# Guia de Testes - HTTP Request/Response & Parsing

## Visão Geral

Este guia apresenta estratégias e casos de teste para validar todas as funcionalidades implementadas pelo **Membro 2** no projeto WebServer.

---

## 1. Testes de Request Parsing

### 1.1 Parse de Método HTTP

**Casos de Teste:**

```
✓ GET válido
✓ POST válido
✓ DELETE válido
✓ Método em lowercase (deve normalizar)
✓ Método inválido/desconhecido
```

**Requisição Exemplo:**
```http
GET /index.html HTTP/1.1
Host: localhost
```

**Validação:**
```cpp
assert(request.get_method() == "GET");
assert(request.get_uri() == "/index.html");
assert(request.get_version() == "HTTP/1.1");
```

### 1.2 Parse de Headers

**Casos de Teste:**

```
✓ Headers simples (Host, User-Agent)
✓ Headers case-insensitive
✓ Headers com espaços
✓ Headers duplicados (último prevalece)
✓ Headers vazios
```

**Requisição Exemplo:**
```http
GET / HTTP/1.1
Host: localhost
User-Agent: TestClient/1.0
Content-Type: text/html
Content-Length: 42
```

**Validação:**
```cpp
assert(request.get_header("Host") == "localhost");
assert(request.get_header("host") == "localhost"); // case-insensitive
assert(request.get_header("user-agent") == "TestClient/1.0");
assert(request.get_header("content-length") == "42");
```

### 1.3 Parse de Body Simples

**Casos de Teste:**

```
✓ Body com Content-Length correto
✓ Body com Content-Length maior que dados
✓ Body vazio (GET requests)
✓ Body grande (stress test)
```

**Requisição Exemplo:**
```http
POST /upload HTTP/1.1
Host: localhost
Content-Length: 13

Hello, World!
```

**Validação:**
```cpp
assert(request.get_body() == "Hello, World!");
assert(request.get_body().length() == 13);
```

### 1.4 Parse de Chunked Transfer Encoding

**Casos de Teste:**

```
✓ Single chunk
✓ Multiple chunks
✓ Empty chunk (0\r\n\r\n)
✓ Large chunks
```

**Requisição Exemplo:**
```http
POST /upload HTTP/1.1
Host: localhost
Transfer-Encoding: chunked

5\r\n
Hello\r\n
7\r\n
, World\r\n
0\r\n
\r\n
```

**Validação:**
```cpp
assert(request.get_body() == "Hello, World");
assert(request.get_header("transfer-encoding") == "chunked");
```

### 1.5 Validação de Requisição Completa

**Casos de Teste:**

```
✓ Requisição completa com headers apenas
✓ Requisição completa com body
✓ Requisição incompleta (faltando body)
✓ Requisição malformada
```

**Validação:**
```cpp
assert(request.is_complete() == true);
assert(request.get_method() != "");
assert(request.get_uri() != "");
```

---

## 2. Testes de Response Generation

### 2.1 Métodos HTTP

#### GET - Arquivo Estático

**Teste:**
```bash
curl http://localhost:8080/index.html
```

**Validações:**
- Status: 200 OK
- Content-Type: text/html
- Content-Length: correto
- Body: conteúdo do arquivo

#### GET - Diretório (Listagem)

**Teste:**
```bash
curl http://localhost:8080/
```

**Validações:**
- Status: 200 OK
- Content-Type: text/html
- Body contém: "Index of"
- Body contém: links para arquivos

#### GET - Arquivo Inexistente (404)

**Teste:**
```bash
curl http://localhost:8080/naoexiste.html
```

**Validações:**
- Status: 404 Not Found
- Content-Type: text/html
- Body contém: "404" e "Not Found"

#### GET - Arquivo Sem Permissão (403)

**Teste:**
```bash
chmod 000 www/restricted.html
curl http://localhost:8080/restricted.html
```

**Validações:**
- Status: 403 Forbidden
- Body contém: "403" e "Forbidden"

#### POST - Criar Arquivo

**Teste:**
```bash
curl -X POST -d "Test content" http://localhost:8080/upload/test.txt
```

**Validações:**
- Status: 201 Created
- Arquivo criado no servidor
- Arquivo contém: "Test content"

#### DELETE - Remover Arquivo

**Teste:**
```bash
curl -X DELETE http://localhost:8080/upload/test.txt
```

**Validações:**
- Status: 200 OK
- Arquivo removido do servidor
- GET subsequente retorna 404

### 2.2 Status Codes

**Casos de Teste:**

| Código | Cenário | Teste |
|--------|---------|-------|
| 200 | OK | GET de arquivo existente |
| 201 | Created | POST bem-sucedido |
| 301 | Moved Permanently | Redirecionamento configurado |
| 400 | Bad Request | Requisição malformada |
| 403 | Forbidden | Arquivo sem permissão |
| 404 | Not Found | Recurso inexistente |
| 405 | Method Not Allowed | PUT não implementado |
| 413 | Payload Too Large | Body > max_size |
| 500 | Internal Server Error | Erro no servidor |
| 501 | Not Implemented | Método desconhecido |

### 2.3 Headers de Resposta

**Validações Necessárias:**

```
✓ Content-Type: correto para cada extensão
✓ Content-Length: igual ao tamanho do body
✓ Server: identificação do servidor
✓ Connection: close
✓ Date: presente (opcional)
```

**Exemplo:**
```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234
Server: WebServer/1.0
Connection: close

<!DOCTYPE html>...
```

---

## 3. Testes de Utilidades

### 3.1 MIME Types

**Casos de Teste:**

```cpp
assert(Utils::get_content_type("html") == "text/html");
assert(Utils::get_content_type("css") == "text/css");
assert(Utils::get_content_type("js") == "text/javascript");
assert(Utils::get_content_type("jpg") == "image/jpeg");
assert(Utils::get_content_type("png") == "image/png");
assert(Utils::get_content_type("json") == "application/json");
assert(Utils::get_content_type("pdf") == "application/pdf");
assert(Utils::get_content_type("unknown") == "application/octet-stream");
```

### 3.2 URL Decoding

**Casos de Teste:**

```cpp
assert(Utils::url_decode("hello%20world") == "hello world");
assert(Utils::url_decode("foo+bar") == "foo bar");
assert(Utils::url_decode("test%2Fpath") == "test/path");
assert(Utils::url_decode("100%25") == "100%");
assert(Utils::url_decode("normal") == "normal");
```

### 3.3 Manipulação de Strings

**Casos de Teste:**

```cpp
// Split
std::vector<std::string> tokens = Utils::split("a,b,c", ",");
assert(tokens.size() == 3);
assert(tokens[0] == "a");

// Trim
assert(Utils::trim("  hello  ") == "hello");
assert(Utils::trim("\t\ntest\r\n") == "test");

// To Lower
assert(Utils::to_lower("HELLO") == "hello");
assert(Utils::to_lower("MiXeD") == "mixed");
```

### 3.4 File Operations

**Casos de Teste:**

```cpp
// File exists
assert(Utils::file_exists("www/index.html") == true);
assert(Utils::file_exists("naoexiste.txt") == false);

// Is directory
assert(Utils::is_directory("www") == true);
assert(Utils::is_directory("www/index.html") == false);

// Read file
std::string content = Utils::read_file("www/index.html");
assert(content.length() > 0);
assert(content.find("<!DOCTYPE") != std::string::npos);
```

---

## 4. Testes de Integração

### 4.1 Fluxo Completo: GET

**Passos:**
1. Cliente conecta
2. Envia requisição GET
3. Servidor recebe dados
4. Parser processa requisição
5. Response gera resposta
6. Servidor envia resposta
7. Cliente recebe e valida

**Validação End-to-End:**
```bash
curl -v http://localhost:8080/index.html
```

### 4.2 Fluxo Completo: POST + GET + DELETE

**Passos:**
1. POST: criar arquivo
2. Validar status 201
3. GET: ler arquivo criado
4. Validar conteúdo
5. DELETE: remover arquivo
6. Validar status 200
7. GET: verificar 404

**Script:**
```bash
# Criar
curl -X POST -d "Test" http://localhost:8080/test.txt

# Ler
curl http://localhost:8080/test.txt

# Deletar
curl -X DELETE http://localhost:8080/test.txt

# Verificar
curl http://localhost:8080/test.txt  # Deve retornar 404
```

### 4.3 Testes de Concorrência

**Cenário:**
- Múltiplos clientes simultâneos
- Requisições GET/POST/DELETE concorrentes
- Verificar integridade dos dados

**Ferramenta:**
```bash
# Apache Bench
ab -n 1000 -c 10 http://localhost:8080/

# wrk
wrk -t10 -c100 -d30s http://localhost:8080/
```

### 4.4 Testes de Stress

**Cenários:**
- Upload de arquivos grandes (> 1MB)
- Requisições com body muito grande
- Muitas requisições pequenas
- Timeouts e reconexões

---

## 5. Testes de Edge Cases

### 5.1 Requisições Malformadas

```
✓ Linha de requisição incompleta
✓ Headers sem ":"
✓ Body maior que Content-Length
✓ Body menor que Content-Length
✓ Caracteres inválidos na URI
```

### 5.2 Condições de Erro

```
✓ Disco cheio (POST fail)
✓ Permissões insuficientes
✓ Arquivo sendo usado por outro processo
✓ Diretório não existe
✓ Symlinks quebrados
```

### 5.3 Casos Especiais

```
✓ URI com query string: /path?key=value
✓ URI com fragmento: /path#section
✓ URI com caracteres especiais: /path%20with%20spaces
✓ Arquivos sem extensão
✓ Arquivos ocultos (começam com .)
```

---

## 6. Checklist de Validação

### Request Parsing
- [ ] Parse de GET, POST, DELETE
- [ ] Headers case-insensitive
- [ ] Body com Content-Length
- [ ] Chunked transfer encoding
- [ ] URL decoding
- [ ] Validação de completude

### Response Generation
- [ ] Status codes corretos (2xx, 3xx, 4xx, 5xx)
- [ ] Content-Type por extensão
- [ ] Content-Length calculado
- [ ] Servir arquivos estáticos
- [ ] Páginas de erro customizadas
- [ ] Directory listing HTML

### File Operations
- [ ] Leitura de arquivos
- [ ] Criação de arquivos (POST)
- [ ] Deleção de arquivos (DELETE)
- [ ] Verificação de permissões
- [ ] Detecção de diretórios

### Error Handling
- [ ] 404 Not Found
- [ ] 403 Forbidden
- [ ] 500 Internal Server Error
- [ ] 501 Not Implemented
- [ ] Exceções capturadas

---

## 7. Ferramentas de Teste

### cURL
```bash
# GET
curl http://localhost:8080/

# POST
curl -X POST -d "data" http://localhost:8080/file.txt

# DELETE
curl -X DELETE http://localhost:8080/file.txt

# Verbose (mostrar headers)
curl -v http://localhost:8080/

# Save output
curl -o output.html http://localhost:8080/
```

### Telnet/Netcat
```bash
# Conexão raw
telnet localhost 8080

# Enviar requisição manual
nc localhost 8080 << EOF
GET / HTTP/1.1
Host: localhost

EOF
```

### Python Script
```python
# Ver test_http.py incluído no projeto
python3 test_http.py
```

### Navegador Web
- Abrir: http://localhost:8080/
- Testar navegação
- Verificar listagem de diretórios
- Validar páginas de erro

---

## 8. Resultados Esperados

### Parsing Correto
✓ Todos os métodos reconhecidos  
✓ Headers extraídos corretamente  
✓ Body completo recebido  
✓ Chunked decoding funcional  

### Respostas Apropriadas
✓ Status codes precisos  
✓ Content-Type correto  
✓ Files servidos integralmente  
✓ Errors com páginas HTML  

### Performance
✓ < 10ms para requisições simples  
✓ Suporta 100+ conexões concorrentes  
✓ Sem memory leaks  
✓ Estável sob stress  

---

## Conclusão

Todas as funcionalidades do **Membro 2** foram implementadas e podem ser testadas usando os métodos descritos neste guia. O sistema está pronto para:

1. **Parse completo de requisições HTTP**
2. **Geração de respostas apropriadas**
3. **Gestão de recursos do sistema**
4. **Tratamento robusto de erros**

Execute os testes regularmente para garantir a qualidade e estabilidade do webserver.
