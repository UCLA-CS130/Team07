//Based off http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/connection.hpp
// and http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/server.hpp

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <string>
#include <thread>
#include <boost/unordered_map.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "response.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "config.h"
#include "request_handler.hpp"

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> HTTPS; 

namespace http {
namespace server {

class connection
  : public std::enable_shared_from_this<connection>
{
public:
	connection(const connection&) = delete;
	connection& operator=(const connection&) = delete;
	explicit connection(boost::asio::ip::tcp::socket socket, boost::asio::io_service& io_service,
		boost::asio::ssl::context& context, bool isHttps);
	explicit connection(boost::asio::ip::tcp::socket socket);
	void start();
	void stop();
	boost::unordered_map<std::string, RequestHandler*>* handlers_;
	HTTPS* ssl_socket_;
private:
	void do_read();
	void do_write();
	void handle_read(boost::system::error_code ec, std::size_t bytes);
	void handle_write(boost::system::error_code ec, std::size_t bytes);

	boost::asio::ip::tcp::socket socket_;
	
	std::array<char, 16384> buffer_;
	Response response_;
	std::unique_ptr<Request> request_;
	bool isHttps_;
};
      

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
	std::string get_password() const;

	void handle_accept(connection* con, const boost::system::error_code& ec);
	void https_handle_accept(connection* con, const boost::system::error_code& ec);
	void https_handle_handshake(connection* con, const boost::system::error_code& ec);

	void InitHandlers();

	ServerConfig* config;
	std::vector<std::thread> threads_;
	boost::unordered_map<std::string, RequestHandler*> handlers_;

	boost::asio::ssl::context context_;
};
} // namespace server
} // namespace http

#endif // HTTP_SERVER_HPP
