#ifndef BASE_REQUEST_HANDLER_HPP
#define BASE_REQUEST_HANDLER_HPP

#include "request.hpp"
#include "response.hpp"
#include "config_parser.h"

#include <map>
#include <memory>
#include <string>

namespace http {
	namespace server {

		class RequestHandler {
			public:	
				enum Status {
					OK = 0,
					docroot_missing = -1,
					bad_request = 400,
					not_found = 404
				};

				static RequestHandler* CreateByName(const std::string& type);

				virtual Status Init(const std::string& uri_prefix, const NginxConfig& config) = 0;
				virtual Status HandleRequest(const Request& request, Response* response) = 0;

				virtual ~RequestHandler() {}

			protected:
				std::string uri_prefix_;
			
		};

		extern std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders;
		template<typename T>
		class RequestHandlerRegisterer {
			public:
				RequestHandlerRegisterer(const std::string& type) 
				{
					if (request_handler_builders == nullptr) 
					{
						request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>;
					}
					(*request_handler_builders)[type] = RequestHandlerRegisterer::Create;
				}

				static RequestHandler* Create() 
				{
					return new T;
				}
		};

		#define REGISTER_REQUEST_HANDLER(ClassName) \
		  static RequestHandlerRegisterer<ClassName> ClassName##__registerer(#ClassName)

	} 
} 

#endif //  BASE_REQUEST_HANDLER_HPP
