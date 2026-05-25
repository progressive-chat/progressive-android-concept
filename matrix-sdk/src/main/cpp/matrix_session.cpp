#include "matrix_session.hpp"
#include <fstream>
#include <android/log.h>

#define LOG_TAG "MatrixSession"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

MatrixSession::MatrixSession(const SessionData& data) : m_data(data) {}
MatrixSession::~MatrixSession() = default;

bool MatrixSession::saveToDisk(const std::string& path) {
    json j;
    j["user_id"] = m_data.userId;
    j["access_token"] = m_data.accessToken;
    j["device_id"] = m_data.deviceId;
    j["homeserver_url"] = m_data.homeserverUrl;
    j["identity_server_url"] = m_data.identityServerUrl;
    j["refresh_token"] = m_data.refreshToken;
    j["is_token_login"] = m_data.isTokenLogin;

    std::ofstream f(path);
    if (!f) { LOGE("Cannot write session: %s", path.c_str()); return false; }
    f << j.dump(2);
    return true;
}

std::unique_ptr<MatrixSession> MatrixSession::loadFromDisk(const std::string& path) {
    std::ifstream f(path);
    if (!f) return nullptr;
    json j = json::parse(f);
    SessionData data;
    data.userId = j.value("user_id", "");
    data.accessToken = j.value("access_token", "");
    data.deviceId = j.value("device_id", "");
    data.homeserverUrl = j.value("homeserver_url", "");
    data.identityServerUrl = j.value("identity_server_url", "");
    data.refreshToken = j.value("refresh_token", "");
    data.isTokenLogin = j.value("is_token_login", false);
    return std::make_unique<MatrixSession>(data);
}

std::vector<DeviceInfo> MatrixSession::getDevices() { return {}; }
bool MatrixSession::deleteDevice(const std::string&) { return false; }
bool MatrixSession::updateDeviceDisplayName(const std::string&, const std::string&) { return false; }
void MatrixSession::setPresence(const std::string&, const std::string&) {}
json MatrixSession::getPresence(const std::string&) { return json::object(); }
json MatrixSession::getProfile(const std::string&) { return json::object(); }
bool MatrixSession::setDisplayName(const std::string&) { return false; }
bool MatrixSession::setAvatarUrl(const std::string&) { return false; }
bool MatrixSession::uploadDeviceKeys(const json&, const json&) { return false; }
json MatrixSession::claimKeys(const std::vector<std::string>&) { return json::object(); }

} // namespace matrix_sdk
