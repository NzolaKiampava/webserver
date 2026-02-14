#include "Response.hpp"
#include "Utils.hpp"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <dirent.h>

// Construtor: Inicializa resposta com status 200 OK
Response::Response() : _status_code(200), _status_message("OK")
{
	// Headers padrão
	_headers["Server"] = "WebServer/1.0";
	_headers["Connection"] = "close";
}

// Destrutor
Response::~Response()
{
}

// Gera resposta apropriada baseado na requisição
// - Verifica método HTTP (GET, POST, DELETE, etc.)
// - Para GET: localiza arquivo, lê conteúdo, define content-type
// - Para POST: salva arquivo/dados no servidor
// - Para DELETE: remove arquivo
// - Trata erros (404, 403, 500) com páginas de erro
// - Define headers apropriados (Content-Length, Content-Type, etc.)
void Response::generate(const Request& request, const std::string& root)
{
	std::string method = request.get_method();
	
	try
	{
		if (method == "GET")
		{
			_handle_get(request, root);
		}
		else if (method == "POST")
		{
			_handle_post(request, root);
		}
		else if (method == "DELETE")
		{
			_handle_delete(request, root);
		}
		else
		{
			// Método não implementado
			set_status(501); // Not Implemented
			_serve_error_page(501);
		}
	}
	catch (const std::exception& e)
	{
		// Erro interno do servidor
		set_status(500);
		_serve_error_page(500);
	}
	
	// Adicionar Content-Length
	std::ostringstream length_stream;
	length_stream << _body.length();
	set_header("Content-Length", length_stream.str());
}

// Handler para requisições GET
void Response::_handle_get(const Request& request, const std::string& root)
{
	std::string uri = request.get_uri();
	
	// Decodificar URI
	uri = Utils::url_decode(uri);
	
	// Remover query string se existir
	size_t query_pos = uri.find('?');
	if (query_pos != std::string::npos)
		uri = uri.substr(0, query_pos);
	
	// Construir caminho completo do arquivo
	std::string filepath = root + uri;
	
	// Verificar se o arquivo/diretório existe
	if (!Utils::file_exists(filepath))
	{
		set_status(404);
		_serve_error_page(404);
		return;
	}
	
	// Se for diretório, tentar servir index.html ou listar conteúdo
	if (Utils::is_directory(filepath))
	{
		// Adicionar '/' no final se não tiver
		if (filepath[filepath.length() - 1] != '/')
			filepath += "/";
		
		std::string index_path = filepath + "index.html";
		
		if (Utils::file_exists(index_path))
		{
			_serve_file(index_path);
		}
		else
		{
			// Listar diretório
			_serve_directory_listing(filepath, uri);
		}
	}
	else
	{
		// Servir arquivo
		_serve_file(filepath);
	}
}

// Handler para requisições POST
void Response::_handle_post(const Request& request, const std::string& root)
{
    std::string uri = request.get_uri();
    std::string content_type = request.get_header("Content-Type");

    // 1. Verificar se é um upload de ficheiro (Multipart)
    if (content_type.find("multipart/form-data") != std::string::npos)
    {
        std::string upload_dir = root + uri;
        
        // Remove a barra final para o Utils::file_exists não se baralhar
        if (!upload_dir.empty() && upload_dir[upload_dir.length() - 1] == '/')
            upload_dir.erase(upload_dir.length() - 1);

        // 2. Verificar se a pasta existe (tu já criaste com mkdir)
        if (!Utils::file_exists(upload_dir))
        {
            set_status(404);
            _serve_error_page(404);
            return;
        }

        // 3. USAR A TUA FUNÇÃO DE MULTIPART!
        // Ela vai extrair o "EDUARDA.txt" do body e salvar na pasta.
        if (_process_multipart_upload(request.get_body(), content_type, upload_dir))
        {
            set_status(201); // Created
            _body = "<html><body><h1>Sucesso!</h1><p>Ficheiro guardado em " + uri + "</p></body></html>";
        }
        else
        {
            set_status(403); // Se falhar a escrita do ficheiro real
            _serve_error_page(403);
        }
        return;
    }

    // Lógica para POSTs normais (se não for ficheiro)
    // Aqui podes manter o que tinhas, mas garante que não tentas abrir pastas
    set_status(405); // Método não permitido para este tipo de POST
    _serve_error_page(405);
}

// Handler para requisições DELETE
void Response::_handle_delete(const Request& request, const std::string& root)
{
	std::string uri = request.get_uri();
	uri = Utils::url_decode(uri);
	
	std::string filepath = root + uri;
	
	// Verificar se arquivo existe
	if (!Utils::file_exists(filepath))
	{
		set_status(404);
		_serve_error_page(404);
		return;
	}
	
	// Não permitir deletar diretórios
	if (Utils::is_directory(filepath))
	{
		set_status(403);
		_serve_error_page(403);
		return;
	}
	
	// Tentar remover arquivo
	if (remove(filepath.c_str()) != 0)
	{
		set_status(500);
		_serve_error_page(500);
		return;
	}
	
	// Sucesso
	set_status(200);
	set_header("Content-Type", "text/html");
	_body = "<html><body><h1>200 OK</h1><p>Resource deleted successfully.</p></body></html>";
}

// Serve um arquivo estático
void Response::_serve_file(const std::string& filepath)
{
	// Verificar permissões de leitura
	std::ifstream file(filepath.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		set_status(403); // Forbidden
		_serve_error_page(403);
		return;
	}
	
	// Ler conteúdo do arquivo
	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	
	_body = content.str();
	
	// Determinar Content-Type baseado na extensão
	std::string extension = Utils::get_file_extension(filepath);
	std::string content_type = Utils::get_content_type(extension);
	
	set_status(200);
	set_header("Content-Type", content_type);
}

// Serve listagem de diretório em HTML
void Response::_serve_directory_listing(const std::string& dirpath, const std::string& uri)
{
	set_status(200);
	set_header("Content-Type", "text/html");
	_body = _generate_directory_html(dirpath, uri);
}

// Gera HTML para listagem de diretório
std::string Response::_generate_directory_html(const std::string& dirpath, const std::string& uri)
{
	std::ostringstream html;
	
	html << "<!DOCTYPE html>\n";
	html << "<html>\n<head>\n";
	html << "<meta charset=\"UTF-8\">\n";
	html << "<title>Index of " << uri << "</title>\n";
	html << "<style>\n";
	html << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
	html << "h1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n";
	html << "table { border-collapse: collapse; width: 100%; }\n";
	html << "th { text-align: left; padding: 8px; background-color: #f0f0f0; }\n";
	html << "td { padding: 8px; border-bottom: 1px solid #ddd; }\n";
	html << "a { text-decoration: none; color: #0066cc; }\n";
	html << "a:hover { text-decoration: underline; }\n";
	html << "</style>\n";
	html << "</head>\n<body>\n";
	html << "<h1>Index of " << uri << "</h1>\n";
	html << "<table>\n";
	html << "<tr><th>Name</th><th>Type</th></tr>\n";
	
	// Link para diretório pai
	if (uri != "/")
	{
		html << "<tr><td><a href=\"..\">[Parent Directory]</a></td><td>DIR</td></tr>\n";
	}
	
	// Listar arquivos e diretórios
	DIR* dir = opendir(dirpath.c_str());
	if (dir)
	{
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			
			// Pular . e ..
			if (name == "." || name == "..")
				continue;
			
			std::string fullpath = dirpath + "/" + name;
			bool is_dir = Utils::is_directory(fullpath);
			
			std::string link_uri = uri;
			if (link_uri[link_uri.length() - 1] != '/')
				link_uri += "/";
			link_uri += name;
			
			if (is_dir)
				link_uri += "/";
			
			html << "<tr>";
			html << "<td><a href=\"" << link_uri << "\">" << name;
			if (is_dir) html << "/";
			html << "</a></td>";
			html << "<td>" << (is_dir ? "DIR" : "FILE") << "</td>";
			html << "</tr>\n";
		}
		closedir(dir);
	}
	
	html << "</table>\n";
	html << "</body>\n</html>";
	
	return html.str();
}

// Serve página de erro personalizada
void Response::_serve_error_page(int code)
{
	std::string error_page_path = "www/error_pages/" + 
								  static_cast<std::ostringstream&>(std::ostringstream() << code).str() + 
								  ".html";
	
	// Tentar carregar página de erro personalizada
	if (Utils::file_exists(error_page_path))
	{
		_body = Utils::read_file(error_page_path);
		set_header("Content-Type", "text/html");
	}
	else
	{
		// Página de erro padrão
		std::ostringstream html;
		html << "<!DOCTYPE html>\n";
		html << "<html>\n<head>\n";
		html << "<meta charset=\"UTF-8\">\n";
		html << "<title>" << code << " " << get_status_message(code) << "</title>\n";
		html << "<style>\n";
		html << "body { font-family: Arial, sans-serif; text-align: center; margin-top: 100px; }\n";
		html << "h1 { font-size: 48px; color: #333; }\n";
		html << "p { font-size: 18px; color: #666; }\n";
		html << "</style>\n";
		html << "</head>\n<body>\n";
		html << "<h1>" << code << " " << get_status_message(code) << "</h1>\n";
		html << "<p>An error occurred while processing your request.</p>\n";
		html << "</body>\n</html>";
		
		_body = html.str();
		set_header("Content-Type", "text/html");
	}
}

// Define o código de status HTTP
void Response::set_status(int code)
{
	_status_code = code;
	_status_message = get_status_message(code);
}

// Adiciona um header à resposta
void Response::set_header(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

// Define o body da resposta
void Response::set_body(const std::string& body)
{
	_body = body;
}

// Monta a resposta HTTP completa formatada
// - Primeira linha: "HTTP/1.1 <status_code> <status_message>"
// - Headers com "\r\n" entre eles
// - Linha em branco
// - Body
std::string Response::get_response() const
{
	std::ostringstream response;
	
	// Status line
	response << "HTTP/1.1 " << _status_code << " " << _status_message << "\r\n";
	
	// Headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		 it != _headers.end(); ++it)
	{
		response << it->first << ": " << it->second << "\r\n";
	}
	
	// Linha em branco separando headers do body
	response << "\r\n";
	
	// Body
	response << _body;
	
	return response.str();
}

// Retorna a mensagem de status para um código HTTP
// Ex: 200 -> "OK", 404 -> "Not Found"
std::string Response::get_status_message(int code) const
{
	switch (code)
	{
		// 2xx Success
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		
		// 3xx Redirection
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 304: return "Not Modified";
		
		// 4xx Client Error
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		
		// 5xx Server Error
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		
		default: return "Unknown";
	}
}

// Gera resposta com configuração (validação de tamanho)
void Response::generate_with_config(const Request& request, const std::string& root, size_t max_body_size)
{
	_max_body_size = max_body_size;
	
	// Validar tamanho do body
	std::string content_length_str = request.get_header("content-length");
	if (!content_length_str.empty())
	{
		size_t content_length = atoi(content_length_str.c_str());
		if (content_length > _max_body_size)
		{
			set_status(413); // Payload Too Large
			_serve_error_page(413);
			return;
		}
	}
	
	generate(request, root);
}

// Define tamanho máximo do body
void Response::set_max_body_size(size_t size)
{
	_max_body_size = size;
}

// Retorna status code
int Response::get_status_code() const
{
	return _status_code;
}

// Handler para upload de arquivos
bool Response::_handle_file_upload(const Request& request, const std::string& upload_dir)
{
	std::string content_type = request.get_header("content-type");
	
	// Verificar se é multipart/form-data
	if (content_type.find("multipart/form-data") == std::string::npos)
		return false;
	
	// Extrair boundary
	size_t boundary_pos = content_type.find("boundary=");
	if (boundary_pos == std::string::npos)
		return false;
	
	std::string boundary = "--" + content_type.substr(boundary_pos + 9);
	
	// Processar body multipart
	std::string body = request.get_body();
	size_t pos = 0;
	
	while (pos < body.length())
	{
		// Encontrar próximo boundary
		size_t boundary_start = body.find(boundary, pos);
		if (boundary_start == std::string::npos)
			break;
		
		pos = boundary_start + boundary.length();
		
		// Verificar se é o último boundary
		if (pos + 2 < body.length() && body.substr(pos, 2) == "--")
			break;
		
		// Pular \r\n após boundary
		if (pos + 2 <= body.length() && body.substr(pos, 2) == "\r\n")
			pos += 2;
		
		// Ler headers da parte
		size_t headers_end = body.find("\r\n\r\n", pos);
		if (headers_end == std::string::npos)
			break;
		
		std::string part_headers = body.substr(pos, headers_end - pos);
		pos = headers_end + 4;
		
		// Extrair filename do Content-Disposition
		std::string filename = _parse_multipart_filename(part_headers);
		
		if (filename.empty())
			continue;
		
		// Encontrar próximo boundary (fim do conteúdo)
		size_t content_end = body.find(boundary, pos);
		if (content_end == std::string::npos)
			break;
		
		// Extrair conteúdo do arquivo (remover \r\n antes do boundary)
		std::string file_content = body.substr(pos, content_end - pos - 2);
		
		// Salvar arquivo
		std::string filepath = upload_dir + "/" + filename;
		std::ofstream file(filepath.c_str(), std::ios::binary);
		
		if (!file.is_open())
			return false;
		
		file << file_content;
		file.close();
		
		pos = content_end;
	}
	
	return true;
}

// Parse filename do header Content-Disposition
std::string Response::_parse_multipart_filename(const std::string& content_disposition)
{
	size_t filename_pos = content_disposition.find("filename=\"");
	if (filename_pos == std::string::npos)
		return "";
	
	filename_pos += 10;  // Pular 'filename="'
	
	size_t filename_end = content_disposition.find("\"", filename_pos);
	if (filename_end == std::string::npos)
		return "";
	
	return content_disposition.substr(filename_pos, filename_end - filename_pos);
}

// Extrai conteúdo de arquivo de um body multipart
std::string Response::_extract_file_content(const std::string& body, const std::string& boundary)
{
	size_t content_start = body.find("\r\n\r\n");
	if (content_start == std::string::npos)
		return "";
	
	content_start += 4;
	
	size_t content_end = body.find(boundary, content_start);
	if (content_end == std::string::npos)
		return body.substr(content_start);
	
	return body.substr(content_start, content_end - content_start - 2);
}

bool Response::_process_multipart_upload(const std::string& body, const std::string& content_type, const std::string& upload_dir)
{
	// 1. Extrair o boundary do header Content-Type
	size_t boundary_pos = content_type.find("boundary=");
	if (boundary_pos == std::string::npos)
		return false;
	
	std::string boundary = "--" + content_type.substr(boundary_pos + 9);
	
	size_t pos = body.find(boundary);
	if (pos == std::string::npos)
		return false;

	while (pos != std::string::npos)
	{
		pos += boundary.length();
		if (body.substr(pos, 2) == "--") break; // Fim do corpo
		
		pos += 2; // Pular \r\n
		
		// 2. Extrair headers da parte (Content-Disposition, etc.)
		size_t headers_end = body.find("\r\n\r\n", pos);
		if (headers_end == std::string::npos) break;
		
		std::string part_headers = body.substr(pos, headers_end - pos);
		std::string filename = _parse_multipart_filename(part_headers);
		
		pos = headers_end + 4; // Ir para o início dos dados do ficheiro
		
		// 3. Encontrar o próximo boundary para saber onde o ficheiro acaba
		size_t next_boundary = body.find(boundary, pos);
		if (next_boundary == std::string::npos) break;
		
		if (!filename.empty())
		{
			// O conteúdo do ficheiro está entre os headers e o próximo boundary (menos o \r\n final)
			std::string file_content = body.substr(pos, next_boundary - pos - 2);
			
			std::string filepath = upload_dir + "/" + filename;
			std::ofstream file(filepath.c_str(), std::ios::binary);
			
			if (file.is_open())
			{
				file << file_content;
				file.close();
				std::cout << "[DEBUG] Ficheiro guardado em: " << filepath << std::endl;
			}
			else
				return false;
		}
		pos = next_boundary;
	}
	return true;
}
