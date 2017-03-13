#ifndef STATUS_HANDLER_HPP
#define STATUS_HANDLER_HPP
// Based Off http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/http/server/request_handler.cpp
#include "request_handler.hpp"

namespace http {
	namespace server {
		class StatusHandler: public RequestHandler {
			public:
				Status HandleRequest(const Request &request, Response* response);	
				Status Init(const std::string& uri_prefix, const NginxConfig& config);

				~StatusHandler(){}
			private:
				std::string setBodyString();
		};

		REGISTER_REQUEST_HANDLER(StatusHandler);

	}
}

#endif // STATUS_HANDLER_HPP