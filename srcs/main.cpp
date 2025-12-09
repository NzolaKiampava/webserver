#include "includes/Server.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
	std::string config_file = "config/default.conf";
	
	if (argc > 1)
		config_file = argv[1];
	
	try
	{
		Server server(config_file);
		server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	
	return 0;
}
