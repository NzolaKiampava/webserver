#include "../includes/CGI.hpp"

// Construtor: Inicializa CGI com caminho do script e requisição
CGI::CGI(const std::string& script_path, const Request& request) : _script_path(script_path)
{
	// TODO: Chamar setup_environment
}

// Destrutor
CGI::~CGI()
{
	// TODO: Limpar recursos
}

// Configura variáveis de ambiente para script CGI
// - REQUEST_METHOD: GET, POST, etc.
// - SCRIPT_NAME: caminho do script
// - QUERY_STRING: parâmetros GET
// - CONTENT_TYPE: tipo do corpo
// - CONTENT_LENGTH: tamanho do corpo
// - REMOTE_ADDR: IP do cliente
// - PATH_INFO: informações de caminho extra
// - Outras variáveis padrão CGI
void CGI::setup_environment(const Request& request)
{
	// TODO: Configurar variáveis de ambiente CGI
}

// Executa o script CGI
// - Fork process para executar script
// - Redireciona stdin/stdout
// - Passa variáveis de environment
// - Captura output do script
// - Retorna output como string
std::string CGI::execute()
{
	// TODO: Executar script CGI via fork + execve
}

// Valida se o script é executável e seguro
// - Verifica se arquivo existe
// - Verifica permissões de execução
// - Verifica se não é um symlink malicioso
// - Verifica extensão permitida
bool CGI::is_valid_script(const std::string& path) const
{
	// TODO: Validar script CGI
}
