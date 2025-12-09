#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <vector>

namespace Utils
{
	std::vector<std::string> split(const std::string& str, const std::string& delimiter);
	std::string trim(const std::string& str);
	std::string to_lower(const std::string& str);
	std::string get_file_extension(const std::string& filename);
	std::string get_content_type(const std::string& extension);
	bool file_exists(const std::string& path);
	std::string read_file(const std::string& path);
	bool is_directory(const std::string& path);
	std::string url_decode(const std::string& url);
}

#endif
