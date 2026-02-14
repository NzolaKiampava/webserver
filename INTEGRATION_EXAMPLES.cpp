/* ************************************************************************** */
/*                                                                            */
/*   EXEMPLO DE INTEGRAÇÃO: Request/Response no Server Loop                  */
/*   Este arquivo mostra como integrar as classes Request e Response         */
/*   no loop principal do servidor                                            */
/*                                                                            */
/* ************************************************************************** */

/*
 * EXEMPLO 1: Processamento Básico de Cliente
 * 
 * No loop principal (Server.cpp), quando um cliente tem dados disponíveis:
 */

void Server::_handle_client_read(Client& client)
{
    try
    {
        // 1. Receber dados do cliente
        client.receive_data();
        
        // 2. Verificar se requisição está completa
        if (client.is_request_complete())
        {
            // 3. Parse da requisição
            client.get_request().parse(client.get_buffer());
            
            // 4. Log da requisição (opcional)
            std::cout << "Request: " 
                      << client.get_request().get_method() << " "
                      << client.get_request().get_uri() 
                      << std::endl;
            
            // 5. Gerar resposta
            client.get_response().generate(
                client.get_request(), 
                _root_directory  // Ex: "www"
            );
            
            // 6. Preparar resposta para envio
            std::string response = client.get_response().get_response();
            client.set_response_buffer(response);
            
            // 7. Log da resposta (opcional)
            std::cout << "Response: " 
                      << client.get_response().get_status_message(200)
                      << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error handling client: " << e.what() << std::endl;
        _remove_client(client.get_socket());
    }
}

/*
 * EXEMPLO 2: Processamento com Validação de Método
 * 
 * Validar métodos HTTP permitidos baseado em configuração:
 */

void Server::_process_request(Client& client)
{
    Request& request = client.get_request();
    Response& response = client.get_response();
    
    // Parse da requisição
    request.parse(client.get_buffer());
    
    // Validar método permitido
    std::string method = request.get_method();
    std::vector<std::string> allowed_methods = _config.get_allowed_methods();
    
    bool method_allowed = false;
    for (size_t i = 0; i < allowed_methods.size(); ++i)
    {
        if (allowed_methods[i] == method)
        {
            method_allowed = true;
            break;
        }
    }
    
    if (!method_allowed)
    {
        // Método não permitido
        response.set_status(405); // Method Not Allowed
        response.set_header("Allow", "GET, POST, DELETE");
        response.set_body("<html><body><h1>405 Method Not Allowed</h1></body></html>");
        response.set_header("Content-Type", "text/html");
        response.set_header("Content-Length", /* ... */);
    }
    else
    {
        // Processar normalmente
        response.generate(request, _root_directory);
    }
    
    // Enviar resposta
    client.set_response_buffer(response.get_response());
}

/*
 * EXEMPLO 3: Redirecionamentos HTTP
 * 
 * Implementar redirecionamentos baseado em configuração:
 */

void Server::_handle_redirects(Client& client)
{
    Request& request = client.get_request();
    Response& response = client.get_response();
    
    std::string uri = request.get_uri();
    
    // Verificar se existe redirecionamento configurado
    std::map<std::string, std::string> redirects = _config.get_redirects();
    std::map<std::string, std::string>::iterator it = redirects.find(uri);
    
    if (it != redirects.end())
    {
        // Configurar redirecionamento 301 ou 302
        response.set_status(301); // Moved Permanently
        response.set_header("Location", it->second);
        response.set_body("<html><body><h1>301 Moved Permanently</h1></body></html>");
        response.set_header("Content-Type", "text/html");
        
        std::ostringstream len;
        len << response.get_body().length();
        response.set_header("Content-Length", len.str());
    }
    else
    {
        // Sem redirecionamento, processar normalmente
        response.generate(request, _root_directory);
    }
    
    client.set_response_buffer(response.get_response());
}

/*
 * EXEMPLO 4: Limitação de Tamanho de Body
 * 
 * Validar tamanho máximo do body para uploads:
 */

void Server::_validate_request_size(Client& client)
{
    Request& request = client.get_request();
    Response& response = client.get_response();
    
    request.parse(client.get_buffer());
    
    // Obter tamanho máximo da configuração (ex: 10MB)
    size_t max_body_size = _config.get_max_body_size();
    
    std::string content_length_str = request.get_header("Content-Length");
    if (!content_length_str.empty())
    {
        size_t content_length = atoi(content_length_str.c_str());
        
        if (content_length > max_body_size)
        {
            // Payload muito grande
            response.set_status(413); // Payload Too Large
            response.set_header("Content-Type", "text/html");
            
            std::ostringstream body;
            body << "<html><body>";
            body << "<h1>413 Payload Too Large</h1>";
            body << "<p>Maximum allowed size: " << max_body_size << " bytes</p>";
            body << "</body></html>";
            
            response.set_body(body.str());
            
            std::ostringstream len;
            len << response.get_body().length();
            response.set_header("Content-Length", len.str());
            
            client.set_response_buffer(response.get_response());
            return;
        }
    }
    
    // Tamanho OK, processar normalmente
    response.generate(request, _root_directory);
    client.set_response_buffer(response.get_response());
}

/*
 * EXEMPLO 5: Tratamento de Erros com Try-Catch
 * 
 * Capturar exceções e gerar respostas de erro apropriadas:
 */

void Server::_safe_process_request(Client& client)
{
    try
    {
        // Tentar parse
        client.get_request().parse(client.get_buffer());
        
        // Tentar gerar resposta
        client.get_response().generate(
            client.get_request(),
            _root_directory
        );
        
        // Preparar envio
        client.set_response_buffer(
            client.get_response().get_response()
        );
    }
    catch (const std::runtime_error& e)
    {
        // Erro de runtime: 500 Internal Server Error
        std::cerr << "Runtime error: " << e.what() << std::endl;
        
        Response& response = client.get_response();
        response.set_status(500);
        response.set_header("Content-Type", "text/html");
        
        std::ostringstream body;
        body << "<html><body>";
        body << "<h1>500 Internal Server Error</h1>";
        body << "<p>An error occurred while processing your request.</p>";
        body << "</body></html>";
        
        response.set_body(body.str());
        
        std::ostringstream len;
        len << response.get_body().length();
        response.set_header("Content-Length", len.str());
        
        client.set_response_buffer(response.get_response());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        _remove_client(client.get_socket());
    }
}

/*
 * EXEMPLO 6: Headers Customizados
 * 
 * Adicionar headers específicos baseado no tipo de resposta:
 */

void Server::_add_custom_headers(Response& response, const Request& request)
{
    // Headers de segurança
    response.set_header("X-Content-Type-Options", "nosniff");
    response.set_header("X-Frame-Options", "SAMEORIGIN");
    
    // CORS (se configurado)
    if (_config.cors_enabled())
    {
        response.set_header("Access-Control-Allow-Origin", "*");
        response.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE");
    }
    
    // Cache control para arquivos estáticos
    std::string uri = request.get_uri();
    std::string extension = Utils::get_file_extension(uri);
    
    if (extension == "css" || extension == "js" || extension == "jpg" || extension == "png")
    {
        response.set_header("Cache-Control", "public, max-age=86400");
    }
    else
    {
        response.set_header("Cache-Control", "no-cache");
    }
    
    // Data/Hora
    time_t now = time(0);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
    response.set_header("Date", buffer);
}

/*
 * EXEMPLO 7: Loop Principal do Servidor (Pseudocódigo)
 */

void Server::run()
{
    while (_running)
    {
        // Poll de sockets
        int ready = poll(_poll_fds, _poll_fds_count, TIMEOUT);
        
        if (ready < 0)
        {
            std::cerr << "Poll error" << std::endl;
            break;
        }
        
        // Processar cada socket
        for (size_t i = 0; i < _poll_fds_count; ++i)
        {
            if (_poll_fds[i].revents & POLLIN)
            {
                // Dados para ler
                if (_is_listening_socket(_poll_fds[i].fd))
                {
                    _accept_new_client();
                }
                else
                {
                    Client* client = _find_client(_poll_fds[i].fd);
                    if (client)
                    {
                        // INTEGRAÇÃO AQUI: Processar requisição
                        _safe_process_request(*client);
                    }
                }
            }
            
            if (_poll_fds[i].revents & POLLOUT)
            {
                // Pronto para escrever
                Client* client = _find_client(_poll_fds[i].fd);
                if (client && client->has_response_to_send())
                {
                    client->send_response();
                    
                    if (client->response_sent())
                    {
                        // Resposta enviada completamente
                        _remove_client(client->get_socket());
                    }
                }
            }
        }
    }
}

/*
 * DICAS DE IMPLEMENTAÇÃO:
 * 
 * 1. Sempre usar try-catch ao processar requisições
 * 2. Validar tamanho de uploads antes de processar
 * 3. Adicionar logs para debug (método, URI, status code)
 * 4. Usar configuração para definir root directory
 * 5. Implementar timeouts para requisições lentas
 * 6. Limpar buffers após processar cada requisição
 * 7. Verificar métodos permitidos por location
 * 8. Adicionar headers de segurança
 * 9. Implementar rate limiting se necessário
 * 10. Tratar casos especiais (CGI, uploads, etc.)
 */
