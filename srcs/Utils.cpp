#include "../includes/Utils.hpp"
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>

namespace Utils
{
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
	
	std::string trim(const std::string& str)
	{
		size_t start = str.find_first_not_of(" \t\r\n");
		if (start == std::string::npos)
			return "";
		
		size_t end = str.find_last_not_of(" \t\r\n");
		return str.substr(start, end - start + 1);
	}
	
	std::string to_lower(const std::string& str)
	{
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return result;
	}
	
	std::string get_file_extension(const std::string& filename)
	{
		size_t pos = filename.find_last_of('.');
		if (pos != std::string::npos)
			return filename.substr(pos + 1);
		return "";
	}
	
	std::string get_content_type(const std::string& extension)
	{
		std::string ext = to_lower(extension);
		
		if (ext == "html" || ext == "htm") return "text/html";
		if (ext == "css") return "text/css";
		if (ext == "js") return "application/javascript";
		if (ext == "json") return "application/json";
		if (ext == "png") return "image/png";
		if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
		if (ext == "gif") return "image/gif";
		if (ext == "ico") return "image/x-icon";
		if (ext == "pdf") return "application/pdf";
		if (ext == "txt") return "text/plain";
		
		return "application/octet-stream";
	}
	
	bool file_exists(const std::string& path)
	{
		struct stat buffer;
		return (stat(path.c_str(), &buffer) == 0);
	}
	
	std::string read_file(const std::string& path)
	{
		std::ifstream file(path);
		if (!file.is_open())
			return "";
		
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
	
	bool is_directory(const std::string& path)
	{
		struct stat buffer;
		if (stat(path.c_str(), &buffer) != 0)
			return false;
		return S_ISDIR(buffer.st_mode);
	}
	
	std::string url_decode(const std::string& url)
	{
		std::string result;
		for (size_t i = 0; i < url.length(); ++i)
		{
			if (url[i] == '%' && i + 2 < url.length())
			{
				int hex = 0;
				sscanf(url.substr(i + 1, 2).c_str(), "%x", &hex);
				result += static_cast<char>(hex);
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
