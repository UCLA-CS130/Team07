//Used from http://www.boost.org/doc/libs/1_60_0/doc/html/boost_asio/example/cpp03/http/server/request.hpp

#ifndef ECHO_HANDLER_HPP
#define ECHO_HANDLER_HPP
// Based Off http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/http/server/request_handler.cpp
#include "request_handler.hpp"

namespace http {
	namespace server {
		class echo_handler: public RequestHandler {
			public:
				Status HandleRequest(const Request &request, Response* response);	
				Status Init(const std::string& uri_prefix, const NginxConfig& config);

				~echo_handler(){}
		};

		REGISTER_REQUEST_HANDLER(echo_handler);

	}
}

#endif // ECHO_HANDLER_HPP
