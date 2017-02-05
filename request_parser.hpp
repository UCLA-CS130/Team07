//Used from http://www.boost.org/doc/libs/1_60_0/doc/html/boost_asio/example/cpp03/http/server/request_parser.hpp

#ifndef HTTP_REQUEST_PARSER_HPP
#define HTTP_REQUEST_PARSER_HPP

#include <tuple>
#include "request.hpp"

namespace http {
namespace server {

class request_parser
{
public:
  request_parser();

  enum result_type { good, bad, indeterminate };

  template <typename InputIt>
  std::tuple<result_type, InputIt> parse(request& req,
      InputIt begin, InputIt end)
  {
    req.content.append(begin, end);
    req.bytes = end - begin;
    while (begin != end)
    {
      result_type result = consume(req, *begin++);
      if (result == good || result == bad)
        return std::make_tuple(result, begin);
    }
    return std::make_tuple(indeterminate, begin);
  }

private:
  result_type consume(request& req, char input);
  static bool is_char(int c);
  static bool is_ctl(int c);
  static bool is_tspecial(int c);
  static bool is_digit(int c);

  /// The current state of the parser.
  enum state
  {
    method_start,
    method,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3
  } state_;
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_PARSER_HPP
