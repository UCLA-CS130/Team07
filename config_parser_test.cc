#include "gtest/gtest.h"
#include "config_parser.h"

TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}

// TESTING ToString function of NginxConfigStatement

TEST(NginxConfigTest, SimpleTest) {
	NginxConfigStatement statement;
	statement.tokens_.push_back("foo");
	statement.tokens_.push_back("bar");
	EXPECT_EQ("foo bar;\n", statement.ToString(0));
}

TEST(NginxConfigTest, BlockTest) {
	NginxConfigStatement statement;
	statement.tokens_.push_back("{");
	statement.tokens_.push_back("foo");
	statement.tokens_.push_back("bar");
	statement.tokens_.push_back("}");
	EXPECT_EQ("{ foo bar };\n", statement.ToString(0));
}

TEST(NginxConfigTest, EmptyTest) {
	NginxConfigStatement statement;
	EXPECT_EQ(";\n", statement.ToString(0));
}

// TESTING Fixture for NginxConfigs in string format

class NginxStringConfigTest : public ::testing::Test {
protected:
	bool ParseString(const std::string config_string) 
	{
		std::stringstream config_stream(config_string);
		return parser_.Parse(&config_stream, &out_config_);
	}
	NginxConfigParser parser_;
	NginxConfig out_config_;
};


TEST_F(NginxStringConfigTest, SimpleTest) {
	EXPECT_TRUE(ParseString("server { port: 8080; }"));
}

TEST_F(NginxStringConfigTest, SimpleWrongTest) {
	EXPECT_FALSE(ParseString("{ }"));
}

TEST_F(NginxStringConfigTest, MissingSemicolon) {
	EXPECT_FALSE(ParseString("server { port: 8080 }"));
}

TEST_F(NginxStringConfigTest, UnbalancedCurlyBraces) {
	EXPECT_FALSE(ParseString("server { port: 8080; "));
}

TEST_F(NginxStringConfigTest, UnbalancedCurlyBracesOpening) {
	EXPECT_FALSE(ParseString("server port: 8080 }"));
}

TEST_F(NginxStringConfigTest, UnbalancedCurlyBracesNested) {
	EXPECT_FALSE(ParseString("server { Location {false; port: 8080 }"));
}

TEST_F(NginxStringConfigTest, BlockAdvanced) {
	EXPECT_TRUE(ParseString("server { port { 8080 ;}}"));
}

TEST_F(NginxStringConfigTest, Commenttest) {
	EXPECT_TRUE(ParseString("server { port { 8080 ;}} # comment 1"));
	EXPECT_EQ(1, out_config_.statements_.size());
}

TEST_F(NginxStringConfigTest, Statementstest) {
	EXPECT_TRUE(ParseString("server { port { 8080 ;}}"));
	EXPECT_EQ(1, out_config_.statements_.size());
	EXPECT_EQ("server",out_config_.statements_[0]->tokens_[0]);
	
	EXPECT_EQ("port",out_config_.statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(0));
	
	EXPECT_EQ("8080",out_config_.statements_.at(0)->child_block_->statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(0));
	
	
}