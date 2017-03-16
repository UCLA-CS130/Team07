#include "request.hpp"

#include <iostream>
#include <cstring>

namespace http {
	namespace server {

		//This function was created as a work-around to use the common API with
		//least changes. This should probably be changed in the future.
		std::unique_ptr<Request> Request::Parse(const std::string& raw_request)
		{
			std::array<char, 30000ul> raw_request_;
			std::memcpy(raw_request_.data(), raw_request.c_str(), raw_request.size());

			request_parser request_parser_;
			std::unique_ptr<Request> new_request = std::unique_ptr<Request>(new Request());
			request_parser_.parse(raw_request_.data(), raw_request_.data() + raw_request.size());

			new_request->method_ = request_parser_.req.method;
			new_request->uri_ = request_parser_.req.uri;
			new_request->http_version_major_ = request_parser_.req.http_version_major;
			new_request->http_version_minor_ = request_parser_.req.http_version_minor;
			for(size_t i = 0; i < request_parser_.req.headers.size(); i++)
			{
				new_request->headers_.push_back(std::make_pair(request_parser_.req.headers[i].name, request_parser_.req.headers[i].value));
			}
			auto pos = request_parser_.req.content.find("\r\n\r\n");
			new_request->content_ = request_parser_.req.content.substr(0, pos+1);
			new_request->bytes_ = request_parser_.req.bytes;
			new_request->raw_request_ = raw_request;

			return new_request;
		}

		std::string Request::raw_request() const
		{
			return raw_request_;
		}

		std::string Request::method() const
		{
			return method_;
		}

		std::string Request::uri() const
		{
			return uri_;
		}
		std::string Request::version() const
		{
			return std::to_string(http_version_major_) + "." + std::to_string(http_version_minor_);
		}

		using Headers = std::vector<std::pair<std::string, std::string>>;
		Headers Request::headers() const
		{
			return headers_;
		}

		std::string Request::body() const
		{
			return content_;
		}

	}
}
