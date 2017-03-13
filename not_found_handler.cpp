// Based Off http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/http/server/request_handler.cpp
#include "not_found_handler.hpp"
#include <fstream>
#include <sstream>

namespace http {
	namespace server {

		RequestHandler::Status NotFoundHandler::HandleRequest(const Request &request, Response* response) {
			//Uses the stock response function to create and set a 404 not found response
			const char text[] = 
				"<html>"
  				"<head><title>Not Found</title></head>"
  				"<body><h1>404 Not Found</h1></body>"
  				"</html>";
		  	std::string body = text;
			response->SetStatus(Response::not_found);
			response->SetBody(body); //.append(req.content, req.content+ req.bytes);

			response->AddHeader("Content-Length", std::to_string(body.size()));
			response->AddHeader("Content-Type", "text/html");

			return RequestHandler::not_found;
		}


		RequestHandler::Status NotFoundHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
		{
			return RequestHandler::OK;
		}

	}
}