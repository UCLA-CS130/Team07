// Based Off http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/http/server/request_handler.cpp
#include "file_handler.hpp"
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>
#include "cpp-markdown/markdown.h"

namespace http {
namespace server {

const char* FILE_HANDLER_ROOT_TOKEN = "root";

bool file_handler::url_decode(const std::string& in, std::string& out)
{
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i)
	{
	  if (in[i] == '%')
	  {
	    if (i + 3 <= in.size())
	    {
	      	int value = 0;
	      	std::istringstream is(in.substr(i + 1, 2));
	      	if (is >> std::hex >> value)
	      	{
			out += static_cast<char>(value);
			i += 2;
	      	}
		else
		{
	  		return false;
		}
		}
		else
		{
			return false;
		}
		}
		else if (in[i] == '+')
		{
			out += ' ';
		}
		else
		{
			out += in[i];
		}
	}
	return true;
}

RequestHandler::Status file_handler::HandleRequest(const Request &request, Response* response){
	std::string encoded_request_path = request.uri();   
	encoded_request_path.erase(encoded_request_path.begin(), encoded_request_path.begin() + uri_prefix_.length());   
	//boost::filesystem::path path{request.uri()};

	//This is to start after the doc_root-specifying token - only with the file name
	/*auto pathIt = path.begin();
	if(++(++path.begin())!=path.end())
	pathIt = ++(++(path.begin()));
	if(pathIt != path.end() && !path.filename().empty())
	{
		//TODO: check errors/long paths
		//request_.uri = "/" + path.filename().string();
		while(pathIt != path.end())
		{
			if(!pathIt->string().empty())
				encoded_request_path += "/" + pathIt->string();
			++pathIt;
		}
	}*/

	std::string request_path;

	if (!url_decode(encoded_request_path, request_path))
	{
		*response = Response::stock_response(Response::bad_request);
		return RequestHandler::bad_request;
	}
	if (request_path.empty() || request_path[0] != '/'
	|| request_path.find("..") != std::string::npos)
	{
		*response = Response::stock_response(Response::bad_request);
		return RequestHandler::bad_request;
	}

	if (request_path[request_path.size() - 1] == '/')
	{
		request_path += "index.html";
	}

	// Determine the file extension.
	std::size_t last_slash_pos = request_path.find_last_of("/");
	std::size_t last_dot_pos = request_path.find_last_of(".");
	std::string extension;
 	if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
	{
		extension = request_path.substr(last_dot_pos + 1);
	}

	std::string full_path = docroot + request_path;
	std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
	if (!is)
	{
		*response = Response::stock_response(Response::not_found);
		return RequestHandler::not_found;
	}

	response->SetStatus(Response::ok);

	std::string body;
	char buf[512];
	while (is.read(buf, sizeof(buf)).gcount() > 0)
		body.append(buf, is.gcount());

	
	
	if(extension == "gif") {
		extension = "image/gif";
	}
	else if (extension == "htm") {
		extension = "text/html";
	}
	else if (extension == "html") {
		extension = "text/html";
	}
	else if (extension == "jpg") {
		extension = "image/jpg";
	}
	else if (extension == "png") {
		extension = "image/png";
	}
	else if (extension == "md") {
		extension = "text/markdown";
	}
	else {
		extension = "text/plain";
	}

	if (extension == "text/markdown") {
		markdown::Document doc;
		doc.read(body);
		std::ostringstream stream;
		doc.write(stream);

		std::string markd = stream.str();

		response->SetBody(markd);

		response->AddHeader("Content-Type", "text/html");

		response->AddHeader("Content-Length", std::to_string(markd.size()));

	} else {
		response->SetBody(body);

		response->AddHeader("Content-Length", std::to_string(body.size()));

		response->AddHeader("Content-Type", extension);
	}	

	return RequestHandler::OK;
}


RequestHandler::Status file_handler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	uri_prefix_ = uri_prefix;

	for(const auto& statement : config.statements_)
	{
		if(statement && statement->tokens_.size() >= 1)
			if (statement->tokens_[0].compare(FILE_HANDLER_ROOT_TOKEN) == 0)
				docroot = statement->tokens_[1];
	}

	if(!docroot.empty())
		return RequestHandler::OK;
	else
		return RequestHandler::docroot_missing;
}

}
}
