#include "spaceteam_proxy_handler.hpp"
#include "config_parser.h"
#include "not_found_handler.hpp"
#include <string>
#include <vector>
#include <iostream>

namespace http {
namespace server {

SpaceteamProxyHandler::SpaceteamProxyHandler() {}
SpaceteamProxyHandler::~SpaceteamProxyHandler() {}

RequestHandler::Status SpaceteamProxyHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
  uri = uri_prefix;
  for (const auto& statement : config.statements_) {
    const std::vector<std::string> tokens = statement->tokens_;
    if (tokens[0] == "host") {
      if (tokens.size() >= 2) {
        this->host = tokens[1];
      }
    } else if (tokens[0] == "proxy_port") {
      if (tokens.size() >= 2) {
        this->port = tokens[1];
      }
    } else {
      return bad_request;
    }
  }

  return OK;
}


RequestHandler::Status SpaceteamProxyHandler::HandleRequest(const Request& req, Response* res){
	std::cout << host << std::endl;
	if (res == nullptr) {
		return bad_request;
	}

  while (true) {
    // connect to host
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(host, port);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator;
    try {
      endpoint_iterator = resolver.resolve(query);
    } catch (std::exception& e) {
      std::cerr << "Exception: " << e.what() << std::endl;
      NotFoundHandler not_found;
      not_found.HandleRequest(req, res);
      return OK;
    }

    boost::asio::ip::tcp::socket sock(io_service);
    boost::asio::connect(sock, endpoint_iterator);

    // get relative URI
    std::string request_uri = req.uri();
    std::string relative_uri;
    if (request_uri != uri 
        && uri != "/"
        && request_uri.size() > uri.size() 
        && request_uri.substr(0, uri.size()) == uri) {
      relative_uri = request_uri.substr(uri.size());
    } else {
      relative_uri = request_uri;
    }

    // send request to host
    std::string request = "GET " + relative_uri + " HTTP/1.1\r\n"
                        + "Host: " + host + ":" + port + "\r\n"
                        + "Connection: close\r\n\r\n";
    boost::asio::write(sock, 
                       boost::asio::buffer(request, request.length()));

    // get response from host
    boost::asio::streambuf buffer;
    boost::system::error_code error;
    while (boost::asio::read(sock, buffer, error)) {
      if (error) break;
    }
    std::string raw_response((std::istreambuf_iterator<char>(&buffer)), std::istreambuf_iterator<char>());
    ParseResponse(raw_response);

    if (response_code == 301 || response_code == 302) { // redirect. try new host
      for (auto h : headers) {
        if (h.first == "Location") {
          host = h.second;
        }
      }
    } else {
  		std::cerr << "sent response for " << relative_uri << std::endl;
      break;
    }
  }

  res->SetStatus(static_cast<Response::ResponseCode>(response_code));
  for (auto h : headers) {
    res->AddHeader(h.first, h.second);
  }
  res->SetBody(body);

  return OK;
}


RequestHandler::Status SpaceteamProxyHandler::ParseResponse(const std::string& raw_response){
  // clear response member variables
  http_version = "";
  response_code = 0;
  headers.clear();
  body.clear();
  state = http_version_h;

  // parse raw response, character by character
  for (char c : raw_response) {
    result_type result = consume(c);
    if (result == result_type::bad) {
      return bad_request;
    }
  }
  return OK;
}


SpaceteamProxyHandler::result_type SpaceteamProxyHandler::consume(char input){
  switch (state) {
    case http_version_h: {
      if (input == 'H') {
        state = http_version_t_1;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_version_t_1: {
      if (input == 'T') {
        state = http_version_t_2;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_version_t_2: {
      if (input == 'T') {
        state = http_version_p;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_version_p: {
      if (input == 'P') {
        state = http_version_slash;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_version_slash: {
      if (input == '/') {
        state = http_version_major_start;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_version_major_start: {
      if (is_digit(input)) {
        http_version.push_back(input);
        state = http_version_major;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_version_major: {
      if (input == '.') {
        http_version.push_back(input);
        state = http_version_minor_start;
        return indeterminate;
      } else if (is_digit(input)) {
        http_version.push_back(input);
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_version_minor_start: {
      if (is_digit(input)) {
        http_version.push_back(input);
        state = http_version_minor;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_version_minor: {
      if (input == ' ') {
        state = http_code_1;
        return indeterminate;
      } else if (is_digit(input)) {
        http_version.push_back(input);
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_code_1: {
      if (is_digit(input)) {
        response_code = input - '0';
        state = http_code_2;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_code_2: {
      if (is_digit(input)) {
        response_code = response_code * 10 + (input - '0');
        state = http_code_3;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case http_code_3: {
      if (is_digit(input)) {
        response_code = response_code * 10 + (input - '0');
        state = expecting_newline_1;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case expecting_newline_1: {
      if (input == '\n') {
        state = header_line_start;
        return indeterminate;
      } else {
        return indeterminate; // ignore status code message
      }
    }
    case header_line_start: {
      if (input == '\r') {
        state = expecting_newline_3;
        return indeterminate;
      } else if (!headers.empty() && (input == ' ' || input == '\t')) {
        state = header_lws;
        return indeterminate;
      } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        headers.push_back(std::pair<std::string, std::string>());
        headers.back().first.push_back(input);
        state = header_name;
        return indeterminate;
      }
    }
    case header_lws: {
      if (input == '\r') {
        state = expecting_newline_2;
        return indeterminate;
      } else if (input == ' ' || input == '\t') {
        return indeterminate;
      } else if (is_ctl(input)) {
        return bad;
      } else {
        state = header_value;
        headers.back().second.push_back(input);
        return indeterminate;
      }
    }
    case header_name: {
      if (input == ':') {
        state = space_before_header_value;
        return indeterminate;
      } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        headers.back().first.push_back(input);
        return indeterminate;
      }
    }
    case space_before_header_value: {
      if (input == ' ') {
        state = header_value;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case header_value: {
      if (input == '\r') {
        state = expecting_newline_2;
        return indeterminate;
      } else if (is_ctl(input)) {
        return bad;
      } else {
        headers.back().second.push_back(input);
        return indeterminate;
      }
    }
    case expecting_newline_2: {
      if (input == '\n') {
        state = header_line_start;
        return indeterminate;
      } else {
        return bad;
      }
    }
    case expecting_newline_3: {
      if (input == '\n') {
        state = body_state;
        return good;
      } else {
        return bad;
      }
    }
    case body_state: {
      // place the rest of the text after headers into the body
      body.push_back(input);
      return indeterminate;
    }
    default: {
      return bad;
    }
  }
}


bool SpaceteamProxyHandler::is_char(int c){
  return c >= 0 && c <= 127;
}


bool SpaceteamProxyHandler::is_ctl(int c){
  return (c >= 0 && c <= 31) || (c == 127);
}


bool SpaceteamProxyHandler::is_tspecial(int c){
  switch (c) {
    case '(': case ')': case '<': case '>': case '@':
    case ',': case ';': case ':': case '\\': case '"':
    case '/': case '[': case ']': case '?': case '=':
    case '{': case '}': case ' ': case '\t':
      return true;
    default:
      return false;
  }
}


bool SpaceteamProxyHandler::is_digit(int c){
  return c >= '0' && c <= '9';
}

}
}
