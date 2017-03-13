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
#include <algorithm> 

#include "server.hpp"
//TODO: check necessary
#include "file_handler.hpp"
#include "echo_handler.hpp"
#include "server_stats.hpp"


namespace http {
namespace server { 

// HTTP RESPONSE/REQUEST WRITE/READ RELATED FUNCTIONS
connection::connection(boost::asio::ip::tcp::socket socket, boost::asio::io_service* io_service, boost::asio::ssl::context* context)
	//TODO: fix nullptrs
	: socket_(std::move(socket))
{
	//paths_ = nullptr;
}

//connection::connection(boost::asio::ip::tcp::socket socket, Path* paths): socket_(std::move(socket)), paths_(paths)
//{}

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
}

void connection::do_read() {
	auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(buffer_),
	//TODO: inline specification
      [this, self](boost::system::error_code ec, std::size_t bytes)
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
      });
}

void connection::do_write() {
	auto self(shared_from_this());

	boost::asio::async_write(socket_, response_.to_buffers(),
	[this, self](boost::system::error_code ec, std::size_t)
	{
		if (!ec)
		{
			boost::system::error_code ignored_ec;
			stop();
		}
	});
}


// SERVER CONNECTION RELATED FUNCTIONS

server::server(const std::string& sconfig_path)
  : io_service_(), acceptor_(io_service_), socket_(io_service_), context_(io_service_, boost::asio::ssl::context::sslv23)
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
		context_.use_certificate_chain_file(config->GetCertFilePath());
		context_.use_private_key_file(config->GetKeyFilePath(), boost::asio::ssl::context::pem);
		context_.use_tmp_dh_file("dh512.pem");

		/*if(verifyFile.size() > 0) {
			context_.load_verify_file(verifyFile);
			
			context_.set_verify_mode(boost::asio::ssl::context::default_workarounds
			| boost::asio::ssl::context::no_sslv2
			| boost::asio::ssl::context::single_dh_use));
			set_session_id_context = true;
		}*/
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
	if(set_session_id_context) {
		session_id_context = std::to_string(config->GetPortNo()) + "localhost";
		SSL_CTX_set_session_id_context(context_.native_handle(), reinterpret_cast<const unsigned char*>(session_id_context.data()),
		std::min<size_t>(session_id_context.size(), SSL_MAX_SSL_SESSION_ID_LENGTH));
    	}
	
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
			[this](boost::system::error_code ec)
			{
				
				if (!acceptor_.is_open())
				{
				  return;
				}

				if (!ec)
				{
					if(config != nullptr)
					{
						std::shared_ptr<connection> con = std::make_shared<connection>(std::move(socket_), &io_service_, &context_);
						con->handlers_ = &handlers_;
						con->start();
					}
				} 
				else if (ec) 
				{
					throw ec;
				}

				do_accept();
			});
		}
		else
		{	
			auto socket = std::make_shared<HTTPS>(io_service_, context_);
			
			acceptor_.async_accept((*socket).lowest_layer(),
			  boost::bind(&server::https_handle_accept, this, socket, 
			    boost::asio::placeholders::error));
		}
	}
	catch (boost::system::error_code const &e) {
		throw e;
	}
}

void server::https_handle_accept(std::shared_ptr<HTTPS> socket, const boost::system::error_code& ec)
{
	if (!ec)
	{
		//Start new operation quickly if not done
		if (ec != boost::asio::error::operation_aborted)
			do_accept();

		boost::asio::ip::tcp::no_delay option(true);
                socket->lowest_layer().set_option(option);

		if(config != nullptr)
		{
			socket->async_handshake(boost::asio::ssl::stream_base::server, boost::bind(&server::https_handle_handshake, this, boost::asio::placeholders::error));
		}

		do_accept();
	}
	else
	{
		throw ec;
	}
}

void server::https_handle_handshake(const boost::system::error_code& ec)
{
	if(!ec)
	{
		std::shared_ptr<connection> con = std::make_shared<connection>(std::move(socket_), &io_service_, &context_);
		con->handlers_ = &handlers_;
		con->start();
	}
	else if (ec) 
	{
		throw ec;
	}
}


}
}
