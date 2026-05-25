#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct SSOProvider { std::string id; std::string name; std::string iconUrl; std::string brand; };

class SSOManager {
public:
    SSOManager(); ~SSOManager();
    void setHomeserver(const std::string& url);
    std::vector<SSOProvider> getProviders();
    std::string getLoginUrl(const std::string& providerId, const std::string& redirectUrl);
    std::string extractLoginToken(const std::string& redirectUrl);
    bool isSSOAvailable();
private:
    std::string m_homeserver;
};
} // namespace matrix_sdk
