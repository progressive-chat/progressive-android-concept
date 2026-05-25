#include "matrix_user_manager.hpp"
#include <android/log.h>
#define LOG_TAG "UserManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

MatrixUserManager::MatrixUserManager() = default;
MatrixUserManager::~MatrixUserManager() = default;

UserProfile* MatrixUserManager::getOrCreateProfile(const std::string& userId) {
    auto it = m_profiles.find(userId);
    if (it != m_profiles.end()) return &it->second;
    UserProfile p; p.userId = userId;
    auto [iter, inserted] = m_profiles.emplace(userId, p);
    return &iter->second;
}

UserProfile* MatrixUserManager::getProfile(const std::string& userId) {
    auto it = m_profiles.find(userId);
    return it != m_profiles.end() ? &it->second : nullptr;
}

void MatrixUserManager::setProfile(const std::string& userId, const UserProfile& profile) {
    m_profiles[userId] = profile;
    m_profiles[userId].loaded = true;
    if (m_callback) m_callback(m_profiles[userId]);
}

void MatrixUserManager::setPresence(const std::string& userId, const std::string& presence, const std::string& statusMsg) {
    auto* profile = getOrCreateProfile(userId);
    profile->presence = presence;
    profile->statusMsg = statusMsg;
    profile->lastActive = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string MatrixUserManager::getDisplayName(const std::string& userId) const {
    auto it = m_profiles.find(userId);
    return it != m_profiles.end() && !it->second.displayName.empty()
        ? it->second.displayName : userId;
}

std::string MatrixUserManager::getAvatarUrl(const std::string& userId) const {
    auto it = m_profiles.find(userId);
    return it != m_profiles.end() ? it->second.avatarUrl : "";
}

void MatrixUserManager::clear() { m_profiles.clear(); }
void MatrixUserManager::onProfileUpdate(std::function<void(const UserProfile&)> cb) {
    m_callback = std::move(cb);
}

} // namespace matrix_sdk
