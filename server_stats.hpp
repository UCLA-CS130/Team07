#ifndef SERVER_STATS_H
#define SERVER_STATS_H

#include <boost/thread/mutex.hpp>
#include "request_handler.hpp"

namespace http {
    namespace server {

        class ServerStats {
            public:
                static ServerStats& getInstance();

                void insertHandler(const std::string& prefix, const std::string& handler_name);
                void insertRequest(const std::string& url, Response::ResponseCode response);
                int getNumRequests();
                void clearAllEntries();

                std::vector<std::pair<std::string, std::string>> getHandlers() const;
                std::vector<std::pair<std::string, Response::ResponseCode>> getRequests() const;

                boost::mutex sync_mutex;

            private:
                ServerStats() {} 

                std::vector<std::pair<std::string, std::string>> handlers_;        
                std::vector<std::pair<std::string, Response::ResponseCode>> requests_;
        };
    }
}

#endif // SERVER_STATS_H
