#include "gtest/gtest.h"
#include "request.hpp"
#include "response.hpp"
#include "not_found_handler.hpp"
#include "request_handler.hpp"
#include <string>
#include <limits.h>
#include <unistd.h>

namespace http {
	namespace server {

			TEST(NotFoundHandlerTest, SimpleTest) {
				Request req;
				Response resp;
				NotFoundHandler not_found_handler;
				NginxConfig conf;
				ASSERT_EQ(not_found_handler.Init("", conf), RequestHandler::Status::OK);
				

				const char text[] =
					"HTTP/1.1 404 Not Found\r\n"
					"Content-Length: 85\r\n"
					"Content-Type: text/html\r\n" 
					"<html>"
			  		"<head><title>Not Found</title></head>"
			  		"<body><h1>404 Not Found</h1></body>"
			  		"</html>";
			  	std::string body = text;

			  	ASSERT_EQ(not_found_handler.HandleRequest(req, &resp), RequestHandler::Status::not_found);
				
				EXPECT_EQ(body, resp.ToString());
			}
	}
}