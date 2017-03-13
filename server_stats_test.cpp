#include "gtest/gtest.h"
#include "request.hpp"
#include "response.hpp"
#include "server_stats.hpp"
#include "request_handler.hpp"
#include <string>
#include <limits.h>
#include <unistd.h>

namespace http {
	namespace server {

		TEST(ServerStatsTest, InsertHandler) {
			ServerStats::getInstance().clearAllEntries();
			ServerStats::getInstance().insertHandler("/echo", "echo_handler");
			ServerStats::getInstance().insertHandler("/static1", "file_handler");
			ServerStats::getInstance().insertHandler("/static2", "file_handler");
			ServerStats::getInstance().insertHandler("/Status", "StatusHandler");

			std::vector<std::pair<std::string, std::string>> hand = ServerStats::getInstance().getHandlers();

			EXPECT_EQ(hand.size(), 4);
			EXPECT_EQ(hand[0].first, "/echo");
			EXPECT_EQ(hand[0].second, "echo_handler");
			EXPECT_EQ(hand[1].first, "/static1");
			EXPECT_EQ(hand[1].second, "file_handler");
			EXPECT_EQ(hand[2].first, "/static2");
			EXPECT_EQ(hand[2].second, "file_handler");
			EXPECT_EQ(hand[3].first, "/Status");
			EXPECT_EQ(hand[3].second, "StatusHandler");
			
		}

		TEST(ServerStatsTest, InsertRequest) {
			ServerStats::getInstance().insertRequest("/echo", Response::ResponseCode::ok);
			ServerStats::getInstance().insertRequest("/static1", Response::ResponseCode::ok);
			ServerStats::getInstance().insertRequest("/static2", Response::ResponseCode::not_found);
			ServerStats::getInstance().insertRequest("/Status", Response::ResponseCode::ok);

			std::vector<std::pair<std::string, Response::ResponseCode>> req = ServerStats::getInstance().getRequests();
			int totalNum = ServerStats::getInstance().getNumRequests();

			EXPECT_EQ(totalNum, 4);
			EXPECT_EQ(req[0].first, "/echo");
			EXPECT_EQ(req[0].second, Response::ResponseCode::ok);
			EXPECT_EQ(req[1].first, "/static1");
			EXPECT_EQ(req[1].second, Response::ResponseCode::ok);
			EXPECT_EQ(req[2].first, "/static2");
			EXPECT_EQ(req[2].second, Response::ResponseCode::not_found);
			EXPECT_EQ(req[3].first, "/Status");
			EXPECT_EQ(req[3].second, Response::ResponseCode::ok);
		}
	
	}
}