#include "gtest/gtest.h"
#include "server.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>

TEST(ServerTesting, ConstructorCheck) {
	try
	{
		http::server::server server_("config");
		
	}
	catch(boost::system::error_code &e)
	{
		EXPECT_EQ(e.message(), "Numerical argument out of domain");
	}

}

TEST(ServerTesting, AddressCheck) {
	try
	{
		http::server::server server_("config");	
	}
	catch(boost::system::error_code &e)
	{
		EXPECT_EQ(e.message(), "Bad address");
	}

}

TEST(ConnectionTesting, InitializationAndConnectionTest) {
	try
	{
		http::server::server server_("config");	
	}
	catch(boost::system::error_code &e)
	{
		std::cout << e.message() << std::endl;
	}

}

class Handler{
	public:
		boost::array<char, 1> data_;
		void handlerWrite(
		  const boost::system::error_code& error, // Result of operation.
		  std::size_t bytes_transferred           // Number of bytes written.
		){}

		void handlerRead(const boost::system::error_code& error, std::size_t bytes_transferred)
		{}
};

TEST(ConnectionTesting, ConnectionConstructor) {
	try 
	{
		Handler* handler = new Handler();
		boost::asio::io_service io;
		boost::asio::ip::tcp::socket sock(io);

		std::string buffer = "GET /index.html HTTP/1.1\r\n\r\n";

		std::make_shared<http::server::connection>(std::move(sock))->start();
		sock.async_write_some(boost::asio::buffer(buffer), 
			boost::bind(&Handler::handlerWrite, handler,
		    	boost::asio::placeholders::error, boost::asio::placeholders::iterator));
		sock.async_read_some(boost::asio::buffer(handler->data_), 
			boost::bind(&Handler::handlerRead, handler,
		    	boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		delete(handler);
	}
	catch(boost::system::error_code &e) 
	{
		//std::cout << e.message();
		EXPECT_EQ(e.message(), "Success");
	}
}

