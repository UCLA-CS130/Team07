//Used from http://www.boost.org/doc/libs/1_60_0/doc/html/boost_asio/example/cpp03/http/server/request.hpp

#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP
#include "request_handler.hpp"

namespace http {
	namespace server {

		extern const char* FILE_HANDLER_ROOT_TOKEN;

		class file_handler: public RequestHandler {
			public:
				Status HandleRequest(const Request &request, Response* response);
				Status Init(const std::string& uri_prefix, const NginxConfig& config);

				~file_handler(){}
			private:
				bool url_decode(const std::string& in, std::string& out);
				std::string docroot;
		};

		REGISTER_REQUEST_HANDLER(file_handler);

	}
}

#endif // FILE_HANDLER_HPP
