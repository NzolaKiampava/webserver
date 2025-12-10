#include "../includes/Utils.hpp"
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>

namespace Utils
{
	// Divide uma string por delimiter
	// Ex: "a,b,c" com "," retorna ["a", "b", "c"]
	std::vector<std::string> split(const std::string& str, const std::string& delimiter)
	{
		// TODO: Implementar split de string
	}
	
	// Remove espaços em branco no início e fim da string
	// Também remove tabs, newlines, carriage returns
	std::string trim(const std::string& str)
	{
		// TODO: Remover whitespace do início e fim
	}
	
	// Converte string para minúsculas
	std::string to_lower(const std::string& str)
	{
		// TODO: Converter para lowercase
	}
	
	// Extrai extensão de um arquivo
	// Ex: "style.css" retorna "css"
	std::string get_file_extension(const std::string& filename)
	{
		// TODO: Retornar extensão do arquivo
	}
	
	// Retorna MIME type baseado na extensão
	// Ex: "html" -> "text/html", "png" -> "image/png"
	std::string get_content_type(const std::string& extension)
	{
		// TODO: Mapear extensão para Content-Type
	}
	
	// Verifica se um arquivo/diretório existe
	bool file_exists(const std::string& path)
	{
		// TODO: Usar stat() para verificar existência
	}
	
	// Lê conteúdo completo de um arquivo
	// Retorna string vazia se arquivo não puder ser lido
	std::string read_file(const std::string& path)
	{
		// TODO: Abrir e ler arquivo inteiro
	}
	
	// Verifica se caminho é um diretório
	bool is_directory(const std::string& path)
	{
		// TODO: Usar stat() e S_ISDIR para verificar
	}
	
	// Decodifica URL (percent-encoding e espaços)
	// Ex: "hello%20world" -> "hello world"
	// Ex: "foo+bar" -> "foo bar"
	std::string url_decode(const std::string& url)
	{
		// TODO: Decodificar URL encoded strings
	}
}
