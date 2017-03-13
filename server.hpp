//Based off http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/connection.hpp
// and http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/server.hpp

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <string>
#include <thread>
#include <boost/unordered_map.hpp>
#include <boost/asio.hpp>
#include <openssl/ssl.h>
#include <boost/asio/ssl.hpp>

#include "response.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "config.h"
#include "request_handler.hpp"

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> HTTPS; 

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
				std::string session_id_context;
				bool set_session_id_context = false;

				void https_handle_accept(std::shared_ptr<HTTPS> socket, const boost::system::error_code& error);
				void https_handle_handshake(const boost::system::error_code& ec);

				void InitHandlers();

				ServerConfig* config;
				std::vector<std::thread> threads_;
				boost::unordered_map<std::string, RequestHandler*> handlers_;

				boost::asio::ssl::context context_;
			};

			class connection
			  : public std::enable_shared_from_this<connection>
		{
			public:
				connection(const connection&) = delete;
				connection& operator=(const connection&) = delete;
				explicit connection(boost::asio::ip::tcp::socket socket, boost::asio::io_service* io_service = nullptr, boost::asio::ssl::context* context = nullptr);
				void start();
				void stop();
				boost::unordered_map<std::string, RequestHandler*>* handlers_;

			private:
				void do_read();
				void do_write();

				boost::asio::ip::tcp::socket socket_;
				HTTPS* ssl_socket_;
				std::array<char, 16384> buffer_;
				Response response_;
				std::unique_ptr<Request> request_;
		};
      
	} 
} 

#endif // HTTP_SERVER_HPP
