#include "gtest/gtest.h"
#include "request.hpp"
#include "response.hpp"
#include "proxy_handler.hpp"
#include "request_handler.hpp"
#include "config_parser.h"
#include <string>
#include <limits.h>
#include <unistd.h>

namespace http {
    namespace server {

        class ProxyHandlerTest : public ::testing::Test {
            protected:
                RequestHandler::Status Init() {
                    Response resp;
                    NginxConfig* config = new NginxConfig();
                    NginxConfigStatement* st = new NginxConfigStatement();

                    st->tokens_.push_back("root");
                    st->tokens_.push_back("ucla.edu");

                    return proxy_handler_.Init("/proxy", *config);
                }

                ProxyHandler proxy_handler_;
        };

        TEST_F(ProxyHandlerTest, InitTest) {
            EXPECT_EQ(404, Init());
        }

        TEST_F(ProxyHandlerTest, BadRequestTest) {
            Response resp;
            std::string body = "GET /proxy HTTP/1.1\r\n\r\n";
            std::unique_ptr<Request> req = Request::Parse(body);

            Init();

            EXPECT_EQ("HTTP/1.1 500 Internal Server Error\r\n", resp.ToString());
        }

    }
}
