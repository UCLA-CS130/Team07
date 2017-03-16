#include "request_handler.hpp"
#include "echo_handler.hpp"
#include "file_handler.hpp"
#include "config.h"

namespace http {
	namespace server {

		std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders = nullptr;

		RequestHandler* RequestHandler::CreateByName(const std::string& type) {
			
			const auto type_and_builder = request_handler_builders->find(type);
			if (type_and_builder == request_handler_builders->end()) 
			{
				return nullptr;
			}
			return (*type_and_builder->second)();
		}

	}
}
