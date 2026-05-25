#include "matrix_sso.hpp"
#include <sstream>
#include <android/log.h>

#define LOG_TAG "SSOManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

SSOManager::SSOManager() = default;
SSOManager::~SSOManager() = default;

void SSOManager::setHomeserver(const std::string& url) { m_homeserver = url; }

std::vector<SSOProvider> SSOManager::getProviders() {
    // Would call GET /_matrix/client/v3/login to get flows
    return {
        {"google", "Google", "", "google"},
        {"github", "GitHub", "", "github"},
        {"apple", "Apple", "", "apple"},
        {"facebook", "Facebook", "", "facebook"},
        {"twitter", "Twitter", "", "twitter"},
        {"gitlab", "GitLab", "", "gitlab"},
    };
}

std::string SSOManager::getLoginUrl(const std::string& providerId, const std::string& redirectUrl) {
    std::ostringstream os;
    os << m_homeserver << "/_matrix/client/v3/login/sso/redirect/"
       << providerId << "?redirectUrl=" << redirectUrl;
    return os.str();
}

std::string SSOManager::extractLoginToken(const std::string& redirectUrl) {
    size_t pos = redirectUrl.find("loginToken=");
    if (pos != std::string::npos) {
        pos += 11;
        size_t end = redirectUrl.find('&', pos);
        return end != std::string::npos
            ? redirectUrl.substr(pos, end - pos)
            : redirectUrl.substr(pos);
    }
    return "";
}

bool SSOManager::isSSOAvailable() {
    return !m_homeserver.empty();
}

} // namespace matrix_sdk
