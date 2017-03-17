#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <string.h>
#include <boost/foreach.hpp>

#include "config.h"

const int DEFAULT_PORT = 80;

// Important Token Declarations
const char* PORT_TOKEN = "port";
const char* PATH_TOKEN = "path";
const char* FILE_HANDLER_ROOT_TOKEN = "root";
const char* DEFAULT_TOKEN = "default";
const char* PROXY_PORT_TOKEN = "proxy_port";
const char* PROXY_HOST_TOKEN = "host";
const char* HTTPS_TOKEN = "https";
const char* CERT_FILE_TOKEN = "certification_file_path";
const char* KEY_FILE_TOKEN = "key_file_path";
const char* THREAD_POOL_SIZE_TOKEN = "thread_pool_size";
const char* TMP_DH_FILE_TOKEN = "tmp_dh_file_path";

ServerConfig::ServerConfig(const std::string& configFilePath) {
	NginxConfigParser config_parser;
	parsedConfig = new NginxConfig();
	config_parser.Parse(configFilePath.c_str(), parsedConfig);

	ParseStatements();
}

int ServerConfig::GetPortNo() {
	return portNo;
}

int ServerConfig::GetThreadPoolSize() {
	if(threadPoolSize <= 0)
		return 1;

	return threadPoolSize;
}

bool ServerConfig::IsHttps() {
	return https;
}

std::string ServerConfig::GetCertFilePath() {
	return certFilePath;
}

std::string ServerConfig::GetKeyFilePath() {
	return keyFilePath;
}

std::string ServerConfig::GetTmpDhFilePath() {
	return tmpDhFilePath;
}

boost::unordered_map<std::string, Path*>& ServerConfig::GetPaths() {
	return paths;
}

bool ServerConfig::ParseStatements() {
	
	portNo = 0;
	for(const auto& statement : parsedConfig->statements_)
	{
		if(statement && statement->tokens_.size() >= 1)
			ParseStatement(statement);
	}

	if(portNo == 0)
	{
		//Setting standard port if not specified.
		portNo = 80;
		std::cerr << "Using port 80 as no port was specified in config" << std::endl;
	}
	if(paths.empty())
	{
		std::cerr << "No Handler Paths specified" << std::endl;
	}

	return true;
}

bool ServerConfig::ParseStatement(std::shared_ptr<NginxConfigStatement> statement, Path* lastPath) {
	
	if(statement->tokens_[0].compare(PORT_TOKEN) == 0)
	{
		int portNoRead = std::stoi(statement->tokens_[1]);
		
		if(portNoRead > 0 && portNoRead <= 65535)
		{
			portNo = portNoRead;
		}
		else
		{
			//Setting standard port if given port out of range.
			portNo = 80;
			std::cerr << "Given Port Number does not exist, using port 80 instead." << std::endl;
		}
		return true;
	} 
	else if(statement->tokens_[0].compare(THREAD_POOL_SIZE_TOKEN) == 0)
	{
		int threadPoolSizeRead = std::stoi(statement->tokens_[1]);

		if(threadPoolSizeRead > 0 && threadPoolSizeRead <= 32)
		{
			threadPoolSize = threadPoolSizeRead;
		}
		else
		{
			threadPoolSize = 1;
		}

		return true;
	} 
	else if(statement->tokens_[0].compare(HTTPS_TOKEN) == 0)
	{
		if(statement->tokens_[1].compare("on") == 0)
			https = true;
		else if (statement->tokens_[1].compare("off") == 0)
			https = false;
		else
		{
			std::cerr << "No or invalid HTTPS condition specified." << std::endl;
		}

		if(statement->child_block_ != nullptr)
		{
			for (const auto& fileHandlerStatement : statement->child_block_->statements_)
			{
				ParseStatement(fileHandlerStatement);
			}
		}

		return true;
	}
	else if(statement->tokens_[0].compare(DEFAULT_TOKEN) == 0)
	{
		Path* new_path = new Path("", statement->tokens_[1]);
		defaultpath = std::make_pair(statement->tokens_[1], new_path);
		if(statement->child_block_ != nullptr)
		{
			new_path->child_block_ = &(*statement->child_block_);
			for (const auto& fileHandlerStatement : statement->child_block_->statements_)
			{
				ParseStatement(fileHandlerStatement, new_path);
			}
		}
		
		return true;
	}
	else if(statement->tokens_[0].compare(PATH_TOKEN) == 0)
	{
		Path* new_path = new Path(statement->tokens_[1], statement->tokens_[2]);
		if(paths.find(statement->tokens_[1]) != paths.end()) 
		{
			std::cerr << "UNRECOVERABLE ERROR: Cannot re-specify mapping to " << statement->tokens_[1] << std::endl;
			exit(0);
		}
		paths[statement->tokens_[1]] = new_path;

		if(statement->child_block_ != nullptr)
		{
			new_path->child_block_ = &(*statement->child_block_);
			for (const auto& fileHandlerStatement : statement->child_block_->statements_)
			{
				ParseStatement(fileHandlerStatement, new_path);
			}
		}
		
		return true;
	}
	else if (statement->tokens_[0].compare(FILE_HANDLER_ROOT_TOKEN) == 0)
	{
		PathOption* new_option = new PathOption(statement->tokens_[0], statement->tokens_[1]);
		lastPath->options[statement->tokens_[0]] = new_option;
		return true;
	}
	else if (statement->tokens_[0].compare(PROXY_PORT_TOKEN) == 0)
	{
		PathOption* new_option = new PathOption(statement->tokens_[0], statement->tokens_[1]);
		lastPath->options[statement->tokens_[0]] = new_option;
		return true;
	}
	else if (statement->tokens_[0].compare(PROXY_HOST_TOKEN) == 0)
	{
		PathOption* new_option = new PathOption(statement->tokens_[0], statement->tokens_[1]);
		lastPath->options[statement->tokens_[0]] = new_option;
		return true;
	}
	else if (statement->tokens_[0].compare(CERT_FILE_TOKEN) == 0)
	{
		certFilePath = statement->tokens_[1];
		return true;
	}
	else if (statement->tokens_[0].compare(KEY_FILE_TOKEN) == 0)
	{
		keyFilePath = statement->tokens_[1];
		return true;
	}
	else if (statement->tokens_[0].compare(TMP_DH_FILE_TOKEN) == 0)
	{
		tmpDhFilePath = statement->tokens_[1];
		return true;
	}
	return false;
}

ServerConfig::~ServerConfig() {
	typedef std::pair<std::string, Path*> map_val_type;
	typedef std::pair<std::string, PathOption*> option_map_val_type;

	BOOST_FOREACH(map_val_type path, paths) 
	{
		BOOST_FOREACH(option_map_val_type option, std::get<1>(path)->options) 
		{
			delete(std::get<1>(option));
		}
		delete(std::get<1>(path));
	}
}

std::pair<std::string, Path*>& ServerConfig::GetDefault() {
	return defaultpath;
}

std::string ServerConfig::ToString() {
	std::string config_output;
	config_output.append("Server is running on Port: ");
	config_output.append(std::to_string(GetPortNo()));

	return config_output;
}
