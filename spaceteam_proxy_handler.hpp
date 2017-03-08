#ifndef SPACETEAM_PROXY_HANDLER_HPP
#define SPACETEAM_PROXY_HANDLER_HPP

#include "request_handler.hpp"

namespace http {
namespace server {
class SpaceteamProxyHandler : public RequestHandler {
public:
	SpaceteamProxyHandler();
  virtual ~SpaceteamProxyHandler();
	Status Init(const std::string& uri_prefix, const NginxConfig& config);
	Status HandleRequest(const Request& request, Response* response);

private:
  // handler configuration
	std::string uri;
	std::string host;
	std::string port;

  // store the parsed response
  std::string http_version;
  int response_code;
  using Headers = std::vector<std::pair<std::string, std::string>>;
  Headers headers;
  std::string body;

  // the following are used for parsing the response

  enum result_type { good, bad, indeterminate };

  Status ParseResponse(const std::string& raw_response);
  result_type consume(char input);

  // Check if a byte is an HTTP character.
  static bool is_char(int c);

  // Check if a byte is an HTTP control character.
  static bool is_ctl(int c);

  // Check if a byte is defined as an HTTP tspecial character.
  static bool is_tspecial(int c);

  // Check if a byte is a digit.
  static bool is_digit(int c);

  // The current state of the parser.
  enum response_parser_state {
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    http_code_1,
    http_code_2,
    http_code_3,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3,
    body_state
  } state;
};

REGISTER_REQUEST_HANDLER(SpaceteamProxyHandler);

}
}

#endif
