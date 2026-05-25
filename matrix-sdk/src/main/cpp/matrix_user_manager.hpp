#pragma once
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct UserProfile {
    std::string userId; std::string displayName; std::string avatarUrl;
    uint64_t lastActive = 0; std::string presence; std::string statusMsg;
    bool loaded = false;
};

class MatrixUserManager {
public:
    MatrixUserManager(); ~MatrixUserManager();
    UserProfile* getOrCreateProfile(const std::string& userId);
    UserProfile* getProfile(const std::string& userId);
    void setProfile(const std::string& userId, const UserProfile& profile);
    void setPresence(const std::string& userId, const std::string& presence, const std::string& statusMsg = "");
    std::string getDisplayName(const std::string& userId) const;
    std::string getAvatarUrl(const std::string& userId) const;
    void clear();
    void onProfileUpdate(std::function<void(const UserProfile&)> cb);
private:
    std::map<std::string, UserProfile> m_profiles;
    std::function<void(const UserProfile&)> m_callback;
};
} // namespace matrix_sdk
