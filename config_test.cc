#include "gtest/gtest.h"
#include <string.h>
#include "config.h"

TEST(ServerConfigTest, SimpleConfig) {
  ServerConfig config("test_config_1");
  EXPECT_EQ(config.GetPortNo(), 12345);
  std::cout << config.ToString() << "     " << "Server is running on Port: 12345";
  EXPECT_EQ(0, config.ToString().compare(std::string{"Server is running on Port: 12345"}));
}

TEST(ServerConfigTest, PortTooLargeConfig) {
  try
  {
  	ServerConfig config("test_config_2");
  }
  catch (PortRangeException &e)
  {
	 EXPECT_EQ(0, strcmp(e.what(), "Port number out of range."));
  }
}

TEST(ServerConfigTest, NoPortConfig) {
  try
  {
  	ServerConfig config("test_config_3");
  }
  catch (InvalidConfigException &e)
  {
	 EXPECT_EQ(0, strcmp(e.what(), "Port number missing in config file."));
  }
}

TEST(ServerConfigTest, EmptyConfig) {
  try
  {
  	ServerConfig config("test_config_4");
  }
  catch (InvalidConfigException &e)
  {
	 EXPECT_EQ(0, strcmp(e.what(), "Port number missing in config file."));
  }
}
