#include <iostream>
#include <boost/asio.hpp>
#include <iostream>
#include <unordered_map>
#include "server.hpp"
#include "config.h"

int main (int argc, char* argv[]) {
	
	if (argc != 2) 
	{
	std::cerr << "Invalid call. Please call ./webserver <config_file>" << std::endl;
	return 1;
	}
	
	http::server::server server_(argv[1]);
	server_.run();

	return 0;
}
