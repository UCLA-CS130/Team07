//Used from http://www.boost.org/doc/libs/1_60_0/doc/html/boost_asio/example/cpp03/http/server/request.hpp

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP
#include "header.hpp"
#include "request_parser.hpp"
#include <string>
#include <vector>
#include <memory>

namespace http {
	namespace server {

		struct request_parser_output;

		class Request {
		 	public:
				static std::unique_ptr<Request> Parse(const std::string& raw_request);

				std::string raw_request() const;
				std::string method() const;
				std::string uri() const;
				std::string version() const;

				using Headers = std::vector<std::pair<std::string, std::string>>;
				Headers headers() const;
				std::string body() const;

		 	private:
				std::string raw_request_;

				std::string method_;
				std::string uri_;
				int http_version_major_;
				int http_version_minor_;
				Headers headers_;
				std::string content_;
				long unsigned int bytes_;
		};

	} 
} 

#endif // HTTP_REQUEST_HPP
