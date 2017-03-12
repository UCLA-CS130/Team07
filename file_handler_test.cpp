#include "gtest/gtest.h"
#include "request.hpp"
#include "response.hpp"
#include "file_handler.hpp"
#include "config_parser.h"
#include <string>
#include <limits.h>
#include <unistd.h>
namespace http {
namespace server {


TEST(SimpleRequestTest, Docroot) {
	
	Response resp;
	//Setting up nested part of config block that specifies mapping
	NginxConfig* parsedConfig = new NginxConfig();
	NginxConfigStatement *st = new NginxConfigStatement();
	st->tokens_.push_back("root");
	st->tokens_.push_back("files");
	auto pointer = std::shared_ptr<NginxConfigStatement>(st);
	parsedConfig->statements_.push_back(pointer);

	//Parsing Request
	std::unique_ptr<Request> req = Request::Parse("GET /static1/test.txt HTTP/1.1\r\n\r\n");
	file_handler *f= new file_handler();
	f->Init("/static1", *parsedConfig);
	std::cout << parsedConfig->statements_.size() <<std::endl;
	Request r = *req;
	f->HandleRequest(r, &resp);
	std::string test = "HTTP/1.1 200 OK\r\nContent-Length: 70\r\nContent-Type: text/plain\r\nJust a test file to check webserver response in the Integration Test.\n";
	EXPECT_EQ(resp.ToString(), test);
}

TEST(BadRequestTest, Docroot) {
	NginxConfig* parsedConfig = new NginxConfig();
	NginxConfigStatement *st = new NginxConfigStatement();
	st->tokens_.push_back("root");
	st->tokens_.push_back("files");
	auto pointer = std::shared_ptr<NginxConfigStatement>(st);
	parsedConfig->statements_.push_back(pointer);
	Response resp;
	std::unique_ptr<Request> req = Request::Parse("GET test.txt HTTP/1.1\r\n\r\n");
	file_handler *f = new file_handler();
	f->Init("", *parsedConfig);
	f->HandleRequest(*req, &resp);
	EXPECT_EQ(resp.getResponseCode(), 400);
}

TEST(FileNotFound, Docroot) {
	NginxConfig* parsedConfig = new NginxConfig();
	NginxConfigStatement *st = new NginxConfigStatement();
	st->tokens_.push_back("root");
	st->tokens_.push_back("files");
	auto pointer = std::shared_ptr<NginxConfigStatement>(st);
	parsedConfig->statements_.push_back(pointer);
	Response rep;
	std::unique_ptr<Request> req = Request::Parse("GET /abcd.txt HTTP/1.1\r\n\r\n");
	file_handler *f= new file_handler();
	f->Init("", *parsedConfig);
	f->HandleRequest(*req, &rep);
	EXPECT_EQ(rep.getResponseCode(), 404);
}

TEST(MarkdownTest, Docroot) {
	
	Response resp;
	//Setting up nested part of config block that specifies mapping
	NginxConfig* parsedConfig = new NginxConfig();
	NginxConfigStatement *st = new NginxConfigStatement();
	st->tokens_.push_back("root");
	st->tokens_.push_back("files1");
	auto pointer = std::shared_ptr<NginxConfigStatement>(st);
	parsedConfig->statements_.push_back(pointer);

	//Parsing Request
	std::unique_ptr<Request> req = Request::Parse("GET /static2/short_mark.md HTTP/1.1\r\n\r\n");
	file_handler *f= new file_handler();
	f->Init("/static2", *parsedConfig);
	std::cout << parsedConfig->statements_.size() <<std::endl;
	Request r = *req;
	f->HandleRequest(r, &resp);
	std::string content = "<h1>An h1 header</h1>\n<p>Paragraphs are separated by a blank line.</p>\n\n";
	content += "<p>2nd paragraph. <em>Italic</em>, <strong>bold</strong>, and <code>monospace</code>. Itemized lists look like:</p>\n\n\n";
	content += "<ul>\n<li>this one</li>\n<li>that one</li>\n<li>the other one</li>\n</ul>\n\n";
	std::string test = "HTTP/1.1 200 OK\r\nContent-Length: 261\r\nContent-Type: text/html\r\n";
	test += content;
	EXPECT_EQ(resp.ToString(), test);
}

}}
