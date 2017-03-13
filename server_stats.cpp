#include "server_stats.hpp"

namespace http {
    namespace server {

        ServerStats& ServerStats::getInstance() {
            static ServerStats instance;
            return instance;
        }

        void ServerStats::insertHandler(const std::string& prefix, const std::string& handler_name) {
            std::pair<std::string, std::string> handler(prefix, handler_name);

            //Checks to see that no duplicate entries are being added
            for (unsigned int i=0; i < handlers_.size(); i++)
            {
                if (handlers_[i].first == prefix)
                {
                    return;
                }
            }

            handlers_.push_back(handler);
        }

        void ServerStats::insertRequest(const std::string& url, Response::ResponseCode response) {
            std::pair<std::string, Response::ResponseCode> req(url, response);
            requests_.push_back(req);
        }

        int ServerStats::getNumRequests() {
            return requests_.size();
        }

        void ServerStats::clearAllEntries() {
            handlers_.clear();
            requests_.clear();
        }

        std::vector<std::pair<std::string, std::string>> ServerStats::getHandlers() const {
            return handlers_;
        }

        std::vector<std::pair<std::string, Response::ResponseCode>> ServerStats::getRequests() const {
            return requests_;
        }

    }
}
