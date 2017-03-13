/* Source:
    www.boost.org/doc/libs/1_62_0/doc/html/boost_aAsio/example/cpp03/http/client/sync_client.cpp
*/

#include <iostream>
#include <cstring>
#include <istream>
#include <ostream>
#include <sstream>
#include <vector>
#include "proxy_handler.hpp"
#include "config_parser.h"

using boost::asio::ip::tcp;

namespace http {
    namespace server {

        ProxyHandler::ProxyHandler() {}

        ProxyHandler::~ProxyHandler() {}

        RequestHandler::Status ProxyHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
            uri_prefix_ = uri_prefix;

            if (config.statements_.size() == 0)
            {
                return RequestHandler::not_found;
            }

            location_ = config.statements_[0]->tokens_[1];
            return RequestHandler::OK;
        }

        RequestHandler::Status ProxyHandler::HandleRequest(const Request &request, Response* response) {
            // Continue redirecting until status code other than 302
            while (true) 
            {
                std::string request_uri = request.uri() == uri_prefix_ ? "/" : request.uri();
                Response::ResponseCode response_code = RedirectRequest(location_, request_uri, response);
                if (response_code != Response::ResponseCode::moved_temporarily) 
                {
                    break;
                }
            }
            return RequestHandler::OK;
        }

        Response::ResponseCode ProxyHandler::RedirectRequest(std::string& location, const std::string& uri, Response* response) {
            boost::asio::io_service io_service;
            tcp::resolver resolver(io_service);
            tcp::resolver::query query(location, "80");
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

            tcp::socket socket(io_service);
            boost::asio::connect(socket, endpoint_iterator);
            
            std::string req = "GET " + uri + " HTTP/1.1\r\nHost: " + location + "\r\nAccept: */*\r\nConnection: close\r\n\r\n";
            boost::asio::write(socket, boost::asio::buffer(req, req.size()));

            boost::asio::streambuf resp;
            boost::asio::read_until(socket, resp, "\r\n");
            
            std::istream response_stream(&resp);
            std::string http_version;
            unsigned int status_code;
            std::string status_message;
            
            response_stream >> http_version;
            response_stream >> status_code;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/")
            {
                std::cout << "Invalid response\n";
                response->SetStatus(Response::ResponseCode::internal_server_error);
                return Response::ResponseCode::internal_server_error;
            }

            // Read the response headers, which are terminated by a blank line.
            boost::asio::read_until(socket, resp, "\r\n\r\n");

            // Process the response headers.
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
            {
                size_t colon_pos = header.find(":");
                std::string key = header.substr(0, colon_pos);
                std::string value = header.substr(colon_pos + 2);
                if (status_code == 302 && key == "Location") 
                {
                    size_t pos = value.find("//");
                    std::string host = value.substr(pos + 2);

                    if (host[host.size() - 2] == '/')
                    {
                        host = host.substr(0, host.size() - 2);
                    }

                    location = host;
                    return Response::ResponseCode::moved_temporarily;
                }
                response->AddHeader(key, value);
            }

            // Read until EOF, writing data to output as we go.
            boost::system::error_code error;
            while (boost::asio::read(socket, resp, error)) 
            {
                std::ostringstream ss;
                ss << &resp;
                response->SetBody(ss.str());
                if (error == boost::asio::error::eof) 
                {
                    break;
                }
            }
            
            if (error != boost::asio::error::eof)
            {
                std::cout << "Error: " << error << std::endl;
            }

            response->SetStatus(GetResponseCode(status_code));
            return Response::ResponseCode::ok;
        }

        Response::ResponseCode ProxyHandler::GetResponseCode(const unsigned int& status_code) {
            switch (status_code) 
            {
            case 200:
                return Response::ResponseCode::ok;
            case 201:
                return Response::ResponseCode::created;
            case 202:
                return Response::ResponseCode::accepted;
            case 204:
                return Response::ResponseCode::no_content;
            case 300:
                return Response::ResponseCode::multiple_choices;
            case 301:
                return Response::ResponseCode::moved_permanently;
            case 302:
                return Response::ResponseCode::moved_temporarily;
            case 304:
                return Response::ResponseCode::not_modified;
            case 400:
                return Response::ResponseCode::bad_request;
            case 401:
                return Response::ResponseCode::unauthorized;
            case 403:
                return Response::ResponseCode::forbidden;
            case 404:
                return Response::ResponseCode::not_found;
            case 500:
                return Response::ResponseCode::internal_server_error;
            case 501:
                return Response::ResponseCode::not_implemented;
            case 502:
                return Response::ResponseCode::bad_gateway;
            case 503:
                return Response::ResponseCode::service_unavailable;
            default:
                return Response::ResponseCode::internal_server_error;
            }
        }

    }
}
