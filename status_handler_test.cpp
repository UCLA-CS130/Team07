#include "gtest/gtest.h"
#include "request.hpp"
#include "response.hpp"
#include "status_handler.hpp"
#include "server_stats.hpp"
#include <string>
#include <limits.h>
#include <unistd.h>

namespace http {
	namespace server {

		TEST(StatusHandlerTest, SimpleInit) {
			StatusHandler status_handler;
			NginxConfig conf;
			ASSERT_EQ(status_handler.Init("", conf), RequestHandler::Status::OK);
		}

		TEST(StatusHandlerTest, HandleReq) {
			Request req;
			Response resp;
			StatusHandler status_handler;
			NginxConfig conf;
			ASSERT_EQ(status_handler.Init("", conf), RequestHandler::Status::OK);
			

			std::string status_page = "HTTP/1.1 200 OK\r\n";
			status_page += "Content-Length: 473\r\n";
			status_page += "Content-Type: text/html\r\n";
			status_page += "<html>";
			status_page += "<head><title>Status Page</title></head>";
			status_page += "<h1>Server Information</h1>";
			status_page += "<h2>Handlers</h2>";
			status_page += "<table><tr><th>URL Prefix</th><th>Handler</th></tr>";
			status_page += "<tr><td>/echo</td><td>echo_handler</td></tr>";
			status_page += "<tr><td>/static1</td><td>file_handler</td></tr>";
			status_page += "</table><br>"; 
			status_page += "<h2>Requests</h2>";
			status_page += "<p>Number of Requests: 3</p>" ;
			status_page += "<table><tr><th>URL Requested</th><th>Response Code</th></tr>";
			status_page += "<tr><td>/echo</td><td>200</td></tr>";
			status_page += "<tr><td>/static1</td><td>200</td></tr>";
			status_page += "<tr><td>default</td><td>404</td></tr>";
			status_page += "</table></html>";

			ServerStats::getInstance().clearAllEntries();
			ServerStats::getInstance().insertHandler("/echo", "echo_handler");
			ServerStats::getInstance().insertHandler("/static1", "file_handler");
			ServerStats::getInstance().insertRequest("/echo", Response::ResponseCode::ok);
			ServerStats::getInstance().insertRequest("/static1", Response::ResponseCode::ok);
			ServerStats::getInstance().insertRequest("default", Response::ResponseCode::not_found);

			ASSERT_EQ(status_handler.HandleRequest(req, &resp), RequestHandler::Status::OK);
			
			EXPECT_EQ(status_page, resp.ToString());
		}

	}
}