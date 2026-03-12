#include "Request.hpp"
#include "Utils.hpp"
#include <sstream>
#include <algorithm>
#include <cstdlib>

// Construtor
Request::Request() : _method(""), _uri(""), _version(""), _body("")
{
}

// Destrutor
Request::~Request()
{
}

// Parse completo de uma requisição HTTP bruta
// - Extrai método (GET, POST, etc.), URI e versão HTTP da linha inicial
// - Faz parse dos headers (Content-Type, Content-Length, etc.)
// - Separa body se existir
// - Trata diferentes formatos (chunked encoding, multipart, etc.)
void Request::parse(const std::string& raw_request)
{
	if (raw_request.empty())
		return;
	
	// Dividir requisição em linhas
	std::istringstream stream(raw_request);
	std::string line;
	
	// 1. Parse da linha de requisição (Request-Line)
	// Formato: METHOD URI HTTP/VERSION
	// Ex: GET /index.html HTTP/1.1
	if (std::getline(stream, line))
	{
		// Remover \r se existir
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);
		
		std::istringstream line_stream(line);
		line_stream >> _method >> _uri >> _version;
		
		// Métodos HTTP são case-sensitive (RFC 9110)
		// Não converter - aceitar apenas maiúsculas (GET, POST, etc.)
	}
	
	// 2. Parse dos headers
	// Formato: Header-Name: Header-Value
	// Headers terminam com linha vazia (\r\n)
	bool headers_done = false;
	while (std::getline(stream, line) && !headers_done)
	{
		// Remover \r se existir
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);
		
		// Linha vazia indica fim dos headers
		if (line.empty())
		{
			headers_done = true;
			break;
		}
		
		// Procurar separador ':'
		size_t colon_pos = line.find(':');
		if (colon_pos != std::string::npos)
		{
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1);
			
			// Remover espaços em branco
			key = Utils::trim(key);
			value = Utils::trim(value);
			
			// Armazenar header (case-insensitive key)
			_headers[Utils::to_lower(key)] = value;
		}
	}
	
	// 3. Parse do body (se existir)
	// Body vem depois da linha vazia
	if (headers_done)
	{
		std::ostringstream body_stream;
		std::string content_length_str = get_header("content-length");
		
		if (!content_length_str.empty())
		{
			// Body com tamanho definido
			int content_length = atoi(content_length_str.c_str());
			
			// Ler o resto do stream
			body_stream << stream.rdbuf();
			_body = body_stream.str();
			
			// Limitar ao Content-Length se necessário
			if (_body.length() > static_cast<size_t>(content_length))
				_body = _body.substr(0, content_length);
		}
		else if (get_header("transfer-encoding") == "chunked")
		{
			// Chunked transfer encoding
			// Formato: tamanho-hex\r\ndata\r\ntamanho-hex\r\ndata\r\n0\r\n\r\n
			std::string chunk_line;
			std::ostringstream decoded_body;
			
			while (std::getline(stream, chunk_line))
			{
				if (!chunk_line.empty() && chunk_line[chunk_line.length() - 1] == '\r')
					chunk_line.erase(chunk_line.length() - 1);
				
				// Ler tamanho do chunk em hexadecimal
				int chunk_size = strtol(chunk_line.c_str(), NULL, 16);
				
				if (chunk_size == 0)
					break; // Último chunk
				
				// Ler dados do chunk
				std::vector<char> chunk_data(chunk_size + 1);
				stream.read(&chunk_data[0], chunk_size);
				chunk_data[chunk_size] = '\0';
				
				decoded_body << std::string(&chunk_data[0], chunk_size);
				
				// Pular \r\n após os dados
				std::getline(stream, chunk_line);
			}
			
			_body = decoded_body.str();
		}
	}
}

// Getter: Retorna método HTTP (GET, POST, DELETE, etc.)
std::string Request::get_method() const
{
	return _method;
}

// Getter: Retorna a URI/caminho solicitado
std::string Request::get_uri() const
{
	return _uri;
}

// Getter: Retorna versão HTTP (1.0 ou 1.1)
std::string Request::get_version() const
{
	return _version;
}

// Getter: Retorna valor de um header específico (case-insensitive)
std::string Request::get_header(const std::string& key) const
{
	std::string lower_key = Utils::to_lower(key);
	std::map<std::string, std::string>::const_iterator it = _headers.find(lower_key);
	
	if (it != _headers.end())
		return it->second;
	
	return "";
}

// Getter: Retorna body da requisição
std::string Request::get_body() const
{
	return _body;
}

// Valida se a requisição está completa e válida
// - Verifica presença de método e URI
// - Verifica content-length vs tamanho real do body
// - Retorna true se pronta para processamento
bool Request::is_complete() const
{
	// Verificar se temos método e URI
	if (_method.empty() || _uri.empty())
		return false;
	
	// Se tem Content-Length, verificar se body está completo
	std::string content_length_str = get_header("content-length");
	if (!content_length_str.empty())
	{
		int expected_length = atoi(content_length_str.c_str());
		return _body.length() >= static_cast<size_t>(expected_length);
	}
	
	// Se não tem Content-Length, requisição está completa
	return true;
}
