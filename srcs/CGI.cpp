#include "CGI.hpp"
#include "Utils.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <signal.h>
#include <errno.h>

// Construtor: Inicializa CGI com caminho do script e requisição
CGI::CGI(const std::string& script_path, const Request& request) 
	: _script_path(script_path), _interpreter_path(""), _timeout(5)
{
	setup_environment(request);
	_request_body = request.get_body();
}

// Construtor com interpretador específico
CGI::CGI(const std::string& script_path, const std::string& interpreter, const Request& request)
	: _script_path(script_path), _interpreter_path(interpreter), _timeout(5)
{
	setup_environment(request);
	_request_body = request.get_body();
}

// Destrutor
CGI::~CGI()
{
}

// Configura variáveis de ambiente padrão
void CGI::_setup_standard_env()
{
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SERVER_SOFTWARE"] = "WebServer/1.0";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["REDIRECT_STATUS"] = "200";  // Necessário para PHP CGI
}

// Configura variáveis de ambiente baseado na requisição
void CGI::_setup_request_env(const Request& request)
{
	// REQUEST_METHOD
	_env["REQUEST_METHOD"] = request.get_method();
	
	// SCRIPT_NAME e SCRIPT_FILENAME
	_env["SCRIPT_NAME"] = _script_path;
	_env["SCRIPT_FILENAME"] = _script_path;
	
	// QUERY_STRING (parte após '?' na URI)
	std::string uri = request.get_uri();
	size_t query_pos = uri.find('?');
	if (query_pos != std::string::npos)
	{
		_query_string = uri.substr(query_pos + 1);
		_env["QUERY_STRING"] = _query_string;
		
		// PATH_INFO (parte antes do '?')
		_env["PATH_INFO"] = uri.substr(0, query_pos);
	}
	else
	{
		_env["QUERY_STRING"] = "";
		_env["PATH_INFO"] = uri;
	}
	
	// CONTENT_TYPE
	std::string content_type = request.get_header("content-type");
	if (!content_type.empty())
		_env["CONTENT_TYPE"] = content_type;
	
	// CONTENT_LENGTH
	std::string content_length = request.get_header("content-length");
	if (!content_length.empty())
		_env["CONTENT_LENGTH"] = content_length;
	else
		_env["CONTENT_LENGTH"] = "0";
	
	// HTTP_* headers (converter headers HTTP para variáveis de ambiente)
	std::string host = request.get_header("host");
	if (!host.empty())
		_env["HTTP_HOST"] = host;
	
	std::string user_agent = request.get_header("user-agent");
	if (!user_agent.empty())
		_env["HTTP_USER_AGENT"] = user_agent;
	
	std::string accept = request.get_header("accept");
	if (!accept.empty())
		_env["HTTP_ACCEPT"] = accept;
	
	std::string cookie = request.get_header("cookie");
	if (!cookie.empty())
		_env["HTTP_COOKIE"] = cookie;
	
	// SERVER_NAME e SERVER_PORT
	if (!host.empty())
	{
		size_t colon_pos = host.find(':');
		if (colon_pos != std::string::npos)
		{
			_env["SERVER_NAME"] = host.substr(0, colon_pos);
			_env["SERVER_PORT"] = host.substr(colon_pos + 1);
		}
		else
		{
			_env["SERVER_NAME"] = host;
			_env["SERVER_PORT"] = "80";
		}
	}
	
	// REMOTE_ADDR (IP do cliente - simplificado)
	_env["REMOTE_ADDR"] = "127.0.0.1";
}

// Setup completo do ambiente
void CGI::setup_environment(const Request& request)
{
	_setup_standard_env();
	_setup_request_env(request);
}

// Define interpretador
void CGI::set_interpreter(const std::string& interpreter_path)
{
	_interpreter_path = interpreter_path;
}

// Define timeout
void CGI::set_timeout(int seconds)
{
	_timeout = seconds;
}

// Cria array de environment variables para execve
char** CGI::_create_env_array() const
{
	char** env = new char*[_env.size() + 1];
	size_t i = 0;
	
	for (std::map<std::string, std::string>::const_iterator it = _env.begin();
	     it != _env.end(); ++it, ++i)
	{
		std::string env_var = it->first + "=" + it->second;
		env[i] = new char[env_var.length() + 1];
		strcpy(env[i], env_var.c_str());
	}
	
	env[i] = NULL;
	return env;
}

// Libera array de environment
void CGI::_free_env_array(char** env) const
{
	for (size_t i = 0; env[i] != NULL; i++)
		delete[] env[i];
	delete[] env;
}

// Lê dados de um file descriptor com timeout
std::string CGI::_read_from_fd(int fd, int timeout) const
{
	std::ostringstream output;
	char buffer[4096];
	
	// Set non-blocking
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	
	time_t start_time = time(NULL);
	
	while (true)
	{
		ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
		
		if (bytes_read > 0)
		{
			buffer[bytes_read] = '\0';
			output << buffer;
			start_time = time(NULL);  // Reset timeout
		}
		else if (bytes_read == 0)
		{
			// EOF
			break;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Não há dados disponíveis agora
				if (time(NULL) - start_time > timeout)
					break;  // Timeout
				
				usleep(10000);  // 10ms
				continue;
			}
			else
			{
				// Erro real
				break;
			}
		}
	}
	
	return output.str();
}

// Aguarda processo filho com timeout
bool CGI::_wait_with_timeout(pid_t pid, int timeout, int& status) const
{
	time_t start_time = time(NULL);
	
	while (true)
	{
		pid_t result = waitpid(pid, &status, WNOHANG);
		
		if (result == pid)
		{
			// Processo terminou
			return true;
		}
		else if (result == -1)
		{
			// Erro
			return false;
		}
		
		// Verificar timeout
		if (time(NULL) - start_time > timeout)
		{
			// Timeout! Matar o processo
			kill(pid, SIGKILL);
			waitpid(pid, &status, 0);
			return false;
		}
		
		usleep(10000);  // 10ms
	}
}

// Executa o script CGI
std::string CGI::execute()
{
	return execute_with_body(_request_body);
}

// Executa CGI com body específico
std::string CGI::execute_with_body(const std::string& body)
{
	if (!is_valid_script(_script_path))
		throw std::runtime_error("Invalid CGI script");
	
	// Criar pipes para stdin e stdout
	int pipe_in[2];   // Para enviar dados ao CGI
	int pipe_out[2];  // Para receber output do CGI
	
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
		throw std::runtime_error("Failed to create pipes");
	
	// Fork process
	pid_t pid = fork();
	
	if (pid == -1)
	{
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		throw std::runtime_error("Failed to fork process");
	}
	
	if (pid == 0)
	{
		// Processo filho
		
		// Redirecionar stdin
		close(pipe_in[1]);  // Fechar escrita
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_in[0]);
		
		// Redirecionar stdout
		close(pipe_out[0]);  // Fechar leitura
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_out[1]);
		
		// Preparar environment
		char** env = _create_env_array();
		
		// Executar script
		char* args[3];
		
		if (!_interpreter_path.empty())
		{
			// Usar interpretador específico
			args[0] = const_cast<char*>(_interpreter_path.c_str());
			args[1] = const_cast<char*>(_script_path.c_str());
			args[2] = NULL;
		}
		else
		{
			// Executar script diretamente
			args[0] = const_cast<char*>(_script_path.c_str());
			args[1] = NULL;
		}
		
		execve(args[0], args, env);
		
		// Se chegou aqui, execve falhou
		_free_env_array(env);
		exit(1);
	}
	else
	{
		// Processo pai
		
		// Fechar lados não usados dos pipes
		close(pipe_in[0]);
		close(pipe_out[1]);
		
		// Enviar body para stdin do CGI
		if (!body.empty())
		{
			ssize_t written = write(pipe_in[1], body.c_str(), body.length());
			(void)written;  // Suprimir warning
		}
		close(pipe_in[1]);
		
		// Ler output do CGI com timeout
		std::string output = _read_from_fd(pipe_out[0], _timeout);
		close(pipe_out[0]);
		
		// Aguardar processo filho com timeout
		int status;
		bool completed = _wait_with_timeout(pid, _timeout, status);
		
		if (!completed)
			throw std::runtime_error("CGI timeout");
		
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			throw std::runtime_error("CGI script failed");
		
		return output;
	}
	
	return "";
}

// Valida se script é executável e seguro
bool CGI::is_valid_script(const std::string& path) const
{
	struct stat st;
	
	// Verificar se arquivo existe
	if (stat(path.c_str(), &st) != 0)
		return false;
	
	// Verificar se é arquivo regular
	if (!S_ISREG(st.st_mode))
		return false;
	
	// Verificar permissões de leitura
	if (!(st.st_mode & S_IRUSR))
		return false;
	
	// Se temos interpretador, não precisa ser executável
	if (!_interpreter_path.empty())
		return true;
	
	// Verificar permissões de execução
	if (!(st.st_mode & S_IXUSR))
		return false;
	
	return true;
}

// Retorna interpretador baseado na extensão
std::string CGI::get_interpreter_for_extension(const std::string& extension) const
{
	std::string ext = Utils::to_lower(extension);
	
	if (ext == "py" || ext == ".py")
		return "/usr/bin/python3";
	
	if (ext == "php" || ext == ".php")
		return "/usr/bin/php-cgi";
	
	if (ext == "pl" || ext == ".pl")
		return "/usr/bin/perl";
	
	if (ext == "rb" || ext == ".rb")
		return "/usr/bin/ruby";
	
	if (ext == "sh" || ext == ".sh")
		return "/bin/sh";
	
	return "";
}

// Dechunking de body (método estático)
std::string CGI::dechunk_body(const std::string& chunked_body)
{
	std::ostringstream decoded;
	size_t pos = 0;
	
	while (pos < chunked_body.length())
	{
		// Ler tamanho do chunk (hexadecimal)
		size_t line_end = chunked_body.find("\r\n", pos);
		if (line_end == std::string::npos)
			break;
		
		std::string size_line = chunked_body.substr(pos, line_end - pos);
		int chunk_size = strtol(size_line.c_str(), NULL, 16);
		
		if (chunk_size == 0)
			break;  // Último chunk
		
		// Pular para os dados
		pos = line_end + 2;
		
		// Ler dados do chunk
		if (pos + chunk_size <= chunked_body.length())
		{
			decoded << chunked_body.substr(pos, chunk_size);
			pos += chunk_size;
		}
		
		// Pular \r\n após os dados
		if (pos + 2 <= chunked_body.length())
			pos += 2;
	}
	
	return decoded.str();
}
