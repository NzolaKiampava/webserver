#include "../includes/CGI.hpp"

CGI::CGI(const std::string& script_path, const Request& request) : _script_path(script_path)
{
	setup_environment(request);
}

CGI::~CGI()
{
}

void CGI::setup_environment(const Request& request)
{
	// TODO: Setup CGI environment variables
	_env["REQUEST_METHOD"] = request.get_method();
	_env["SCRIPT_NAME"] = _script_path;
	_env["QUERY_STRING"] = "";
	_env["CONTENT_TYPE"] = request.get_header("Content-Type");
	_env["CONTENT_LENGTH"] = request.get_header("Content-Length");
}

std::string CGI::execute()
{
	// TODO: Execute CGI script and return output
	return "";
}

bool CGI::is_valid_script(const std::string& path) const
{
	// TODO: Validate if script is executable
	return true;
}
