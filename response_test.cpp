#include "gtest/gtest.h"
#include "response.hpp"


TEST(ResponseTest, StatusCheck) {
    http::server::Response resp_;
    resp_.SetStatus(http::server::Response::ok);
  	EXPECT_EQ(resp_.getResponseCode(), 200);
}

TEST(ResponseTest, HeaderStruct) {
	http::server::Response resp_;
	resp_.SetStatus(http::server::Response::ok);
	resp_.SetBody("Test content");
	http::server::header head_;
	head_.name = "Content-Length";
	head_.value = std::to_string(12);
	resp_.AddHeader(head_.name, head_.value);

	std::string body = "HTTP/1.1 200 OK\r\n";
	body += "Content-Length: 12\r\n";
	body += "Test content";

	EXPECT_EQ(resp_.ToString(), body);

}

TEST(ResponseTest, HeaderVector) {
	http::server::Response resp_;
	resp_.SetStatus(http::server::Response::ok);
	resp_.SetBody("Test content");
	http::server::header head_;
	head_.name = "Content-Length";
	head_.value = std::to_string(12);
	resp_.AddHeader(head_.name, head_.value);


	head_.name = "Content-Type";
	head_.value = "text/plain";
	resp_.AddHeader(head_.name, head_.value);

	std::string body = "HTTP/1.1 200 OK\r\n";
	body += "Content-Length: 12\r\n";
	body += "Content-Type: text/plain\r\n";
	body += "Test content";
	EXPECT_EQ(resp_.ToString(), body);

}

class ResponseHeaderTest : public ::testing::Test {
	protected:
		bool AnalyzeHeader(std::string rcontent) {
			resp_.SetStatus(http::server::Response::not_found);
			resp_.SetBody(rcontent);
			buffers = resp_.to_buffers();
			std::string body = "HTTP/1.1 404 Not Found\r\n";
			body += rcontent;
			if (resp_.ToString() == body)
			{
				return true;
			}
			return false;	

		}
		void loadHeader() {
			resp_.SetStatus(http::server::Response::not_found);
			resp_.SetBody("Test content");
			head1_.name = "Content-Length";
			head1_.value = std::to_string(12);
			resp_.AddHeader(head1_.name, head1_.value);

			head2_.name = "Content-Type";
			head2_.value = "text/plain";
			resp_.AddHeader(head2_.name, head2_.value);

			buffers = resp_.to_buffers();
		}
		http::server::Response resp_;
		http::server::header head1_, head2_;
		std::vector<boost::asio::const_buffer> buffers;
};

TEST_F(ResponseHeaderTest, HeaderParsing) {
	loadHeader();
	std::string body = "HTTP/1.1 404 Not Found\r\n";
	body += "Content-Length: 12\r\n";
	body += "Content-Type: text/plain\r\n";
	body += "Test content";
  	EXPECT_EQ(body, resp_.ToString());
}

TEST_F(ResponseHeaderTest, ContentPassedInBuffer) {
  	EXPECT_TRUE(AnalyzeHeader("Test Content"));
}
