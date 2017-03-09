//Based off http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/connection.hpp
// and http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/server.hpp

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <boost/unordered_map.hpp>
#include "response.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "config.h"
#include "request_handler.hpp"

namespace http {
namespace server {


class server
{
public:
  server(const server&) = delete;
  server& operator=(const server&) = delete;
  explicit server(const std::string& sconfig_path);
  ~server();
  void run();
  
private:
  void do_accept();
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::tcp::socket socket_;

  void InitHandlers();

  ServerConfig* config;
  boost::unordered_map<std::string, RequestHandler*> handlers_;
};

class connection
  : public std::enable_shared_from_this<connection>
{
public:
  connection(const connection&) = delete;
  connection& operator=(const connection&) = delete;
  explicit connection(boost::asio::ip::tcp::socket socket);
  void start();
  void stop();
  boost::unordered_map<std::string, RequestHandler*>* handlers_;

private:
  void do_read();
  void do_write();

  boost::asio::ip::tcp::socket socket_;
  std::array<char, 16384> buffer_;
  Response response_;
  std::unique_ptr<Request> request_;

};

} // namespace server
} // namespace http

#endif // HTTP_SERVER_HPP
