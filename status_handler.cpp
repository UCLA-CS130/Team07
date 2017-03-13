// Based Off http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/http/server/request_handler.cpp
#include "status_handler.hpp"
#include "server_stats.hpp"
#include <fstream>
#include <sstream>

namespace http {
	namespace server {

		RequestHandler::Status StatusHandler::HandleRequest(const Request &request, Response* response) {
			std::string body = setBodyString();
			response->SetStatus(Response::ok);

			response->SetBody(body); 

			response->AddHeader("Content-Length", std::to_string(body.size()));
			response->AddHeader("Content-Type", "text/html");

			return RequestHandler::OK;
		}


		RequestHandler::Status StatusHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
			return RequestHandler::OK;
		}

		//sets body of string used for response->SetBody()
		std::string StatusHandler::setBodyString() {
			int numReq = ServerStats::getInstance().getNumRequests();
			std::vector<std::pair<std::string, std::string>> handlers = ServerStats::getInstance().getHandlers();
			std::vector<std::pair<std::string, Response::ResponseCode>> requests = ServerStats::getInstance().getRequests();

			std::string status_page = "<html>";
			status_page += "<head><title>Status Page</title></head>";
			status_page += "<h1>Server Information</h1>";
			status_page += "<h2>Handlers</h2>";
			status_page += "<table><tr><th>URL Prefix</th><th>Handler</th></tr>";
			
			for (unsigned int i = 0; i < handlers.size(); i++)
			{
				status_page += "<tr><td>" + handlers[i].first + "</td><td>"+ handlers[i].second + "</td></tr>"; 
			}

			status_page += "</table><br>"; 
			status_page += "<h2>Requests</h2>";
			status_page += "<p>Number of Requests: " + std::to_string(numReq) + "</p>" ;
			status_page += "<table><tr><th>URL Requested</th><th>Response Code</th></tr>";

			for (unsigned int i = 0; i < requests.size(); i++)
			{
				status_page += "<tr><td>" + requests[i].first + "</td><td>"+ std::to_string(requests[i].second) + "</td></tr>"; 
			}

			status_page += "</table></html>";

			return status_page;

		}

	}
}