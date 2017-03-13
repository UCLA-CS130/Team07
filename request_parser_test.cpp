#include "gtest/gtest.h"
#include "request_parser.hpp"
#include "request.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace http {
	namespace server {

		TEST(RequestParsingTesting, EmptyParse) {
			request_parser request_parser_;
			request_parser::result_type res;
			Request request_;
			std::string buffer = "";

			std::tie(res, std::ignore) = request_parser_.parse(buffer.begin(), buffer.end());
			EXPECT_EQ(res, request_parser::indeterminate);
		}

		TEST(RequestParsingTesting, BadParse) {
			request_parser request_parser_;
			request_parser::result_type res;
			Request request_;
			std::string buffer = "()";

			std::tie(res, std::ignore) = request_parser_.parse(buffer.begin(), buffer.end());
			EXPECT_EQ(res, request_parser::bad);
		}

		TEST(RequestParsingTesting, GoodParse) {
			request_parser request_parser_;
			request_parser::result_type res;
			Request request_;
			std::string buffer = "GET /index.html HTTP/1.1\r\n\r\n";

			std::tie(res, std::ignore) = request_parser_.parse(buffer.begin(), buffer.end());
			EXPECT_EQ(res, request_parser::good);
		}

		TEST(RequestParsingTesting, GoodLongParse) {
			request_parser request_parser_;
			request_parser::result_type res;
			Request request_;
			std::string buffer = "GET /index.html HTTP/1.1\r\nHost: localhost:12345\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp;q=0.8\r\nAccept-Encoding: gzip, deflate, sdch, br\r\nAccept-Language: en-US,en;q=0.8\r\n\r\n";

			std::tie(res, std::ignore) = request_parser_.parse(buffer.begin(), buffer.end());
			EXPECT_EQ(res, request_parser::good);
		}

	}
}