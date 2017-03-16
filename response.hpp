// Based of http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/reply.hpp
//and http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/header.hpp

#ifndef HTTP_REPLY_HPP
#define HTTP_REPLY_HPP
#include "header.hpp"
#include <string>
#include <vector>
#include <boost/asio.hpp>

namespace http {
	namespace server {


		class Response {
			public:
				enum ResponseCode {
					ok = 200,
					created = 201,
					accepted = 202,
					no_content = 204,
					multiple_choices = 300,
					moved_permanently = 301,
					moved_temporarily = 302,
					not_modified = 304,
					bad_request = 400,
					unauthorized = 401,
					forbidden = 403,
					not_found = 404,
					internal_server_error = 500,
					not_implemented = 501,
					bad_gateway = 502,
					service_unavailable = 503
				};

				void SetStatus(const ResponseCode response_code);
				void AddHeader(const std::string& header_name, const std::string& header_value);
				void SetBody(const std::string& body);

				/// Convert the reply into a vector of buffers. The buffers do not own the
				/// underlying memory blocks, therefore the reply object must remain valid and
				/// not be changed until the write operation has completed.
				std::vector<boost::asio::const_buffer> to_buffers();

				/// Get a stock reply.
				static Response stock_response(ResponseCode rcode);
				ResponseCode getResponseCode() { return response_code; }
				std::string getContent() {return content; };
				std::string ToString();

			private:
				ResponseCode response_code;

				/// The headers to be included in the reply.
				std::vector<header> headers;

				/// The content to be sent in the reply.
				std::string content;
		};

	}
}

#endif // HTTP_REPLY_HPP
