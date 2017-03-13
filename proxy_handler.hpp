#ifndef PROXY_HANDLER_HPP
#define PROXY_HANDLER_HPP

#include "request_handler.hpp"

namespace http {
	namespace server {

		class ProxyHandler : public RequestHandler {
			
			public:
			    ProxyHandler();
			    virtual ~ProxyHandler();
			    Status Init(const std::string& uri_prefix, const NginxConfig& config);
			    Status HandleRequest(const Request &request, Response* response);
			
			private:
			    Response::ResponseCode RedirectRequest(std::string& location, const std::string& uri, Response* response);
			    Response::ResponseCode GetResponseCode(const unsigned int& status_code);

			    std::string location_;
			    std::string uri_prefix_;
		};

		REGISTER_REQUEST_HANDLER(ProxyHandler);
	}
}

#endif
