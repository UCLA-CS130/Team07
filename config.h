// An nginx config file in memory.

#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>

#include "config_parser.h"

extern const int DEFAULT_PORT;

extern const char* PORT_TOKEN;
extern const char* PATH_TOKEN;
extern const char* FILE_HANDLER_ROOT_TOKEN;
extern const char* PROXY_PORT_TOKEN;
extern const char* PROXY_HOST_TOKEN;

class InvalidConfigException: public std::runtime_error
{
public:
  InvalidConfigException(const std::string msg) : runtime_error(msg) {}
};

class PortRangeException: public std::range_error
{
public:
  PortRangeException(const std::string msg) : range_error(msg) {}
};

struct PathOption{
	PathOption(){}
	PathOption(const std::string& token_, const std::string& value_) : token(token_), value(value_){}

	std::string token;
	std::string value;
};

struct Path{
	Path(){}
	Path(const std::string& token_, const std::string& handler_name_) : token(token_), handler_name(handler_name_){}

	std::string token;
	std::string handler_name;
	boost::unordered_map<std::string, PathOption*> options;

	NginxConfig* child_block_;
};

//TODO: check for duplicate paths and match longest path


// The in-memory representation of the entire config.
class ServerConfig {
 private:
	int portNo;
	boost::unordered_map<std::string, Path*> paths;
	bool ParseStatements();
	bool ParseStatement(std::shared_ptr<NginxConfigStatement> statement, Path* lastPath = nullptr);
	NginxConfig* parsedConfig;
	std::pair<std::string, Path*> defaultpath;

 public:
	ServerConfig(const std::string& configFilePath);
	int GetPortNo();
	std::string ToString();
	~ServerConfig();
	std::pair<std::string, Path*>& GetDefault();
	boost::unordered_map<std::string, Path*>& GetPaths();

};

#endif //  CONFIG_H
