
//Based off: http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/server.cpp
// and http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/http/server/connection.cpp
// and for HTTPS, https://github.com/eidheim/Simple-Web-Server/blob/master/server_https.hpp

#include <cstdlib>
#include <iostream>
#include <utility>
#include <boost/filesystem.hpp>
#include <string.h>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <algorithm> 

#include "server.hpp"
//TODO: check necessary
#include "file_handler.hpp"
#include "echo_handler.hpp"
#include "server_stats.hpp"


namespace http {
	namespace server { 

connection::connection(boost::asio::ip::tcp::socket socket, boost::asio::io_service& io_service,
		boost::asio::ssl::context& context, bool isHttps)
 	: socket_(std::move(socket))
{
	isHttps_ = isHttps;
	if(isHttps_)
		ssl_socket_ = new HTTPS(io_service, context);
}

connection::connection(boost::asio::ip::tcp::socket socket)
 	: socket_(std::move(socket))
{}

void connection::start() {
	try 
	{
		do_read();
	}
	catch(boost::system::error_code &e) 
	{
		throw e;
	}
}

void connection::stop() {
	socket_.close();
	delete(ssl_socket_);
}

void connection::handle_read(boost::system::error_code ec, std::size_t bytes)
{
	if(buffer_.empty() && isHttps_)
	{
		isHttps_ = !isHttps_;
		start();
	}
	request_ = Request::Parse(buffer_.data());
        std::string uri = request_->uri();
	std::string cur_prefix = uri;
        
	while((*handlers_)[cur_prefix] == nullptr && cur_prefix.compare("/")!=0 && !cur_prefix.empty())
	{ 
		
		if(!cur_prefix.empty() && cur_prefix.back() == '/')
			cur_prefix.pop_back();
		
		if((*handlers_)[cur_prefix] != nullptr)
			break;

		while(!cur_prefix.empty() && cur_prefix.back() != '/')
			cur_prefix.pop_back();
	}
	
	// assign request to proxy handler if referer field exists
	for (auto pair : request_->headers()) {
		if (pair.first == "Referer") {
			auto ref_uri = pair.second.find_last_of("/");
			cur_prefix = pair.second.substr(ref_uri);
		}
	}

	if((*handlers_)[cur_prefix] != nullptr)
	{       
		(*handlers_)[cur_prefix]->HandleRequest(*request_, &response_);
		{		
			boost::unique_lock<boost::mutex> lock(ServerStats::getInstance().sync_mutex);
			ServerStats::getInstance().insertRequest(cur_prefix, response_.getResponseCode());
		} //lock object destroyed => mutex unlocked
		
	}
	else 
	{
		(*handlers_)["default"]->HandleRequest(*request_, &response_);
		{	
			boost::unique_lock<boost::mutex> lock(ServerStats::getInstance().sync_mutex);
			ServerStats::getInstance().insertRequest("default", response_.getResponseCode());
		} //lock object destroyed => mutex unlocked
	}
        do_write();
}

void connection::handle_read(std::shared_ptr<connection>& self, boost::system::error_code ec, std::size_t bytes)
{
	request_ = Request::Parse(buffer_.data());
        std::string uri = request_->uri();
	std::string cur_prefix = uri;
        
	while((*handlers_)[cur_prefix] == nullptr && cur_prefix.compare("/")!=0 && !cur_prefix.empty())
	{ 
		
		if(!cur_prefix.empty() && cur_prefix.back() == '/')
			cur_prefix.pop_back();
		
		if((*handlers_)[cur_prefix] != nullptr)
			break;

		while(!cur_prefix.empty() && cur_prefix.back() != '/')
			cur_prefix.pop_back();
	}
	
	// assign request to proxy handler if referer field exists
	for (auto pair : request_->headers()) {
		if (pair.first == "Referer") {
			auto ref_uri = pair.second.find_last_of("/");
			cur_prefix = pair.second.substr(ref_uri);
		}
	}

	if((*handlers_)[cur_prefix] != nullptr)
	{       
		(*handlers_)[cur_prefix]->HandleRequest(*request_, &response_);
		{		
			boost::unique_lock<boost::mutex> lock(ServerStats::getInstance().sync_mutex);
			ServerStats::getInstance().insertRequest(cur_prefix, response_.getResponseCode());
		} //lock object destroyed => mutex unlocked
		
	}
	else 
	{
		(*handlers_)["default"]->HandleRequest(*request_, &response_);
		{	
			boost::unique_lock<boost::mutex> lock(ServerStats::getInstance().sync_mutex);
			ServerStats::getInstance().insertRequest("default", response_.getResponseCode());
		} //lock object destroyed => mutex unlocked
	}
        do_write();
}

void connection::do_read() {

	if(isHttps_)
	 	ssl_socket_->async_read_some(boost::asio::buffer(buffer_,16384),
					boost::bind(&connection::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	else 
	{
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(buffer_),
					boost::bind(&connection::handle_read, this, self, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

	
	}
}

void connection::do_write() {
	//auto self(shared_from_this());

	if(isHttps_)
		boost::asio::async_write(*ssl_socket_, response_.to_buffers(),
					 boost::bind(&connection::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	else 
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_, response_.to_buffers(),
					 boost::bind(&connection::handle_write, this, self, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
}

void connection::handle_write(boost::system::error_code ec, std::size_t bytes)
{
	if (!ec)
	{
		boost::system::error_code ignored_ec;
		stop();
	}
}


void connection::handle_write(std::shared_ptr<connection>& self, boost::system::error_code ec, std::size_t bytes)
{
	if (!ec)
	{
		boost::system::error_code ignored_ec;
		stop();
	}
}

// SERVER CONNECTION RELATED FUNCTIONS

server::server(const std::string& sconfig_path)
  : io_service_(), acceptor_(io_service_),socket_(io_service_), context_(boost::asio::ssl::context::sslv23)
{
	config = new ServerConfig(sconfig_path);

	int port = config->GetPortNo();
	if (port <= 0 || port > 65535) {
		throw boost::system::errc::make_error_code(boost::system::errc::argument_out_of_domain);
	}

	if(config->IsHttps())
	{
		context_.set_options(boost::asio::ssl::context::default_workarounds
					| boost::asio::ssl::context::no_sslv2
					| boost::asio::ssl::context::single_dh_use);
		context_.set_password_callback(boost::bind(&server::get_password, this));
		context_.use_certificate_chain_file(config->GetCertFilePath());
		context_.use_private_key_file(config->GetKeyFilePath(), boost::asio::ssl::context::pem);
		context_.use_tmp_dh_file(config->GetTmpDhFilePath());
	}

	InitHandlers();

	boost::asio::ip::tcp::resolver resolver(io_service_);
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();

	do_accept();
}

std::string server::get_password() const{
    return "";
}

void server::InitHandlers() {
	BOOST_FOREACH(auto path_element, config->GetPaths()) 
	{
		Path* path = std::get<1>(path_element);
		auto handler = RequestHandler::CreateByName(path->handler_name);
  		handler->Init(path->token, *(path->child_block_));
		handlers_[path->token] = handler;
		//inserting handler info to server stats (used for status handler)
		{		
			boost::unique_lock<boost::mutex> lock(ServerStats::getInstance().sync_mutex);
			ServerStats::getInstance().insertHandler(path->token, path->handler_name);
		} //lock object destroyed => mutex unlocked
	}

	Path* default_ = std::get<1>(config->GetDefault());
	auto handler = RequestHandler::CreateByName(default_->handler_name);
	handler->Init(default_->token, *(default_->child_block_));
	handlers_["default"] = handler;
	{		
		boost::unique_lock<boost::mutex> lock(ServerStats::getInstance().sync_mutex);
		ServerStats::getInstance().insertHandler("default", default_->handler_name);
	} //lock object destroyed => mutex unlocked
}

server::~server() {
	delete(config);
}

void server::run() {
	threads_.clear();
	for(int c = 1; c < config->GetThreadPoolSize(); c++) {
		threads_.emplace_back([this]() {
			io_service_.run();
		});
	}

	if(config->GetThreadPoolSize() > 0)
		io_service_.run();
	
	
	for(auto& t: threads_) {
		t.join();
	}

}

void server::do_accept() {
	try {
		if(!config->IsHttps())
		{
			acceptor_.async_accept(socket_,
			  boost::bind(&server::handle_accept, this,
			    boost::asio::placeholders::error, nullptr));
		}
		else
		{
			
			connection* con = new connection(std::move(socket_), io_service_, context_, config->IsHttps());
			acceptor_.async_accept(con->ssl_socket_->lowest_layer(),
			  boost::bind(&server::https_handle_accept, this,
			    boost::asio::placeholders::error, con));
		}
	}
	catch (boost::system::error_code const &e) {
		throw e;
	}
}

void server::handle_accept(const boost::system::error_code& ec, connection* con)
{
	if (!acceptor_.is_open())
	{
	  return;
	}

	if (!ec)
	{
		if(config != nullptr)
		{
			if(!config->IsHttps())
			{
				std::shared_ptr<connection> con_shared = std::make_shared<connection>(std::move(socket_));
				con_shared->handlers_ = &handlers_;
				con_shared->start();
			}
			else if (con != nullptr)
			{
				con->handlers_ = &handlers_;
				con->start();
			}
		}
	}
	else if (ec) 
	{
		throw ec;//do_accept();
	}

	do_accept();
}

void server::https_handle_accept(const boost::system::error_code& ec, connection* con)
{
	if (!acceptor_.is_open())
	{
	  return;
	}

	if (ec != boost::asio::error::operation_aborted)
		do_accept();

	if (!ec)
	{
		//boost::asio::ip::tcp::no_delay option(true);
        	//con->ssl_socket_->lowest_layer().set_option(option);

		con->ssl_socket_->async_handshake(boost::asio::ssl::stream_base::server, 
					boost::bind(&server::handle_accept, this, boost::asio::placeholders::error, con));
	}
	else
	{
		throw ec; //do_accept();
	}
}

}
}
