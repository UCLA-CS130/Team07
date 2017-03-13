// Based Off http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/http/server/request_handler.cpp
#include "echo_handler.hpp"
#include <fstream>
#include <sstream>

namespace http {
	namespace server {

		RequestHandler::Status echo_handler::HandleRequest(const Request &request, Response* response) {
			response->SetStatus(Response::ok);
			response->SetBody(request.body());

			response->AddHeader("Content-Length", std::to_string(request.body().size()));
			response->AddHeader("Content-Type", "text/plain");

			return RequestHandler::OK;
		}


		RequestHandler::Status echo_handler::Init(const std::string& uri_prefix, const NginxConfig& config) {
			uri_prefix_ = uri_prefix;
			return RequestHandler::OK;
		}

	}
}
