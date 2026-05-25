#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct OIDCConfig { std::string issuer; std::string clientId; std::string redirectUri; std::vector<std::string> scopes; };
struct OIDCTokenResponse { std::string accessToken; std::string refreshToken; std::string idToken; int expiresIn = 0; bool success = false; };

class OIDCManager {
public:
    OIDCManager(); ~OIDCManager();
    void setConfig(const OIDCConfig& config);
    std::string getAuthorizationUrl();
    OIDCTokenResponse exchangeCode(const std::string& code, const std::string& codeVerifier);
    OIDCTokenResponse refreshTokens(const std::string& refreshToken);
    bool isConfigured() const;
private:
    OIDCConfig m_config; std::string m_codeVerifier;
    std::string generateCodeVerifier();
    std::string generateCodeChallenge(const std::string& verifier);
};
} // namespace matrix_sdk
