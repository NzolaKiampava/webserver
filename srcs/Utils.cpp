#include "Utils.hpp"
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <dirent.h>

namespace Utils
{
	// Divide uma string por delimiter
	// Ex: "a,b,c" com "," retorna ["a", "b", "c"]
	std::vector<std::string> split(const std::string& str, const std::string& delimiter)
	{
		std::vector<std::string> tokens;
		size_t start = 0;
		size_t end = str.find(delimiter);
		
		while (end != std::string::npos)
		{
			tokens.push_back(str.substr(start, end - start));
			start = end + delimiter.length();
			end = str.find(delimiter, start);
		}
		
		tokens.push_back(str.substr(start));
		return tokens;
	}
	
	// Remove espaços em branco no início e fim da string
	// Também remove tabs, newlines, carriage returns
	std::string trim(const std::string& str)
	{
		const char* whitespace = " \t\n\r\f\v";
		size_t start = str.find_first_not_of(whitespace);
		
		if (start == std::string::npos)
			return "";
		
		size_t end = str.find_last_not_of(whitespace);
		return str.substr(start, end - start + 1);
	}
	
	// Converte string para minúsculas
	std::string to_lower(const std::string& str)
	{
		std::string result = str;
		for (size_t i = 0; i < result.length(); ++i)
			result[i] = std::tolower(static_cast<unsigned char>(result[i]));
		return result;
	}
	
	// Extrai extensão de um arquivo
	// Ex: "style.css" retorna "css"
	std::string get_file_extension(const std::string& filename)
	{
		size_t dot_pos = filename.find_last_of('.');
		if (dot_pos == std::string::npos || dot_pos == filename.length() - 1)
			return "";
		
		return filename.substr(dot_pos + 1);
	}
	
	// Retorna MIME type baseado na extensão
	// Ex: "html" -> "text/html", "png" -> "image/png"
	std::string get_content_type(const std::string& extension)
	{
		std::string ext = to_lower(extension);
		
		// Text types
		if (ext == "html" || ext == "htm") return "text/html";
		if (ext == "css") return "text/css";
		if (ext == "js") return "text/javascript";
		if (ext == "txt") return "text/plain";
		if (ext == "xml") return "text/xml";
		if (ext == "csv") return "text/csv";
		
		// Image types
		if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
		if (ext == "png") return "image/png";
		if (ext == "gif") return "image/gif";
		if (ext == "bmp") return "image/bmp";
		if (ext == "ico") return "image/x-icon";
		if (ext == "svg") return "image/svg+xml";
		if (ext == "webp") return "image/webp";
		
		// Application types
		if (ext == "json") return "application/json";
		if (ext == "pdf") return "application/pdf";
		if (ext == "zip") return "application/zip";
		if (ext == "tar") return "application/x-tar";
		if (ext == "gz") return "application/gzip";
		if (ext == "doc") return "application/msword";
		if (ext == "docx") return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
		if (ext == "xls") return "application/vnd.ms-excel";
		if (ext == "xlsx") return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
		
		// Audio/Video types
		if (ext == "mp3") return "audio/mpeg";
		if (ext == "wav") return "audio/wav";
		if (ext == "ogg") return "audio/ogg";
		if (ext == "mp4") return "video/mp4";
		if (ext == "avi") return "video/x-msvideo";
		if (ext == "webm") return "video/webm";
		
		// Default
		return "application/octet-stream";
	}
	
	// Verifica se um arquivo/diretório existe
	bool file_exists(const std::string& path)
	{
		struct stat buffer;
		return (stat(path.c_str(), &buffer) == 0);
	}
	
	// Lê conteúdo completo de um arquivo
	// Retorna string vazia se arquivo não puder ser lido
	std::string read_file(const std::string& path)
	{
		std::ifstream file(path.c_str(), std::ios::binary);
		if (!file.is_open())
			return "";
		
		std::ostringstream content;
		content << file.rdbuf();
		file.close();
		
		return content.str();
	}
	
	// Verifica se caminho é um diretório
	bool is_directory(const std::string& path)
	{
		struct stat buffer;
		if (stat(path.c_str(), &buffer) != 0)
			return false;
		
		return S_ISDIR(buffer.st_mode);
	}
	
	// Decodifica URL (percent-encoding e espaços)
	// Ex: "hello%20world" -> "hello world"
	// Ex: "foo+bar" -> "foo bar"
	std::string url_decode(const std::string& url)
	{
		std::string result;
		result.reserve(url.length());
		
		for (size_t i = 0; i < url.length(); ++i)
		{
			if (url[i] == '%' && i + 2 < url.length())
			{
				// Decodificar hexadecimal
				std::string hex = url.substr(i + 1, 2);
				char ch = static_cast<char>(strtol(hex.c_str(), NULL, 16));
				result += ch;
				i += 2;
			}
			else if (url[i] == '+')
			{
				result += ' ';
			}
			else
			{
				result += url[i];
			}
		}
		
		return result;
	}
}
