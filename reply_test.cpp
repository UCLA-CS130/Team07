#include "gtest/gtest.h"
#include "reply.hpp"

TEST(ReplyTest, StatusCheck) {
    http::server::reply reply_;
    reply_.status = http::server::reply::ok;
  	EXPECT_EQ(reply_.status, 200);
}

TEST(ReplyTest, HeaderStruct) {
	http::server::reply reply_;
	reply_.content = "Test content";
	http::server::header head_;
	head_.name = "Content-Length";
	head_.value = std::to_string(reply_.content.size());
	reply_.headers.push_back(head_);
	EXPECT_EQ(reply_.headers[0].name, "Content-Length");
	EXPECT_EQ(reply_.headers[0].value, "12");
}

TEST(ReplyTest, HeaderVector) {
	http::server::reply reply_;
	reply_.content = "Test content";
	http::server::header head_;
	head_.name = "Content-Length";
	head_.value = std::to_string(reply_.content.size());
	reply_.headers.push_back(head_);
	EXPECT_EQ(reply_.headers.size(), 1);

	head_.name = "Content-Type";
	head_.value = "text/plain";
	reply_.headers.push_back(head_);
	EXPECT_EQ(reply_.headers.size(), 2);
}

class ReplyHeaderTest : public ::testing::Test {
	protected:
		bool AnalyzeHeader(std::string rcontent) {
			reply_.status = http::server::reply::not_found;
			reply_.content = rcontent;
			buffers = reply_.to_buffers();
			if (reply_.headers.size() == 0)
			{
				return true;
			}
			return false;	
		}

		void loadHeader() {
			reply_.status = http::server::reply::ok;
			reply_.content = "Test content";
			head1_.name = "Content-Length";
			head1_.value = std::to_string(reply_.content.size());
			reply_.headers.push_back(head1_);

			head2_.name = "Content-Type";
			head2_.value = "text/plain";
			reply_.headers.push_back(head2_);

			buffers = reply_.to_buffers();
		}
		http::server::reply reply_;
		http::server::header head1_, head2_;
		std::vector<boost::asio::const_buffer> buffers;
};

TEST_F(ReplyHeaderTest, HeaderParsing) {
	loadHeader();
  	EXPECT_EQ(reply_.content, "Test content");
  	EXPECT_EQ(reply_.headers[0].name, "Content-Length");
  	EXPECT_EQ(reply_.headers[0].value, "12");
  	EXPECT_EQ(reply_.headers[1].name, "Content-Type");
  	EXPECT_EQ(reply_.headers[1].value,"text/plain");
}

TEST_F(ReplyHeaderTest, ContentPassedInBuffer) {
  	EXPECT_TRUE(AnalyzeHeader("Test Content"));
}
