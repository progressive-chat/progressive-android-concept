#include "matrix_client.hpp"
#include <sstream>
#include <thread>
#include <algorithm>
#include <curl/curl.h>
#include <android/log.h>

#define LOG_TAG "MatrixClient"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

static size_t curlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append(static_cast<char*>(contents), total);
    return total;
}

MatrixClient::MatrixClient(const ClientConfig& config) : m_config(config) {}

MatrixClient::~MatrixClient() { stop(); }

// ── Server discovery ──

MatrixClient::WellKnown MatrixClient::discoverWellKnown(const std::string& domain) {
    WellKnown wk;
    std::string url = "https://" + domain + "/.well-known/matrix/client";
    try {
        json j = apiGet(url);
        if (j.contains("m.homeserver")) {
            wk.homeserver = j["m.homeserver"]["base_url"];
            wk.valid = true;
        }
        if (j.contains("m.identity_server")) {
            wk.identityServer = j["m.identity_server"]["base_url"];
        }
    } catch (...) {
        wk.homeserver = "https://" + domain;
        wk.valid = true; // fallback to default
    }
    return wk;
}

// ── Authentication ──

LoginResponse MatrixClient::login(const LoginRequest& req) {
    LoginResponse resp;
    json body;
    body["type"] = req.type;
    if (req.token.empty()) {
        body["identifier"] = {{"type", req.identifierType}, {"user", req.user}};
        body["password"] = req.password;
    } else {
        body["token"] = req.token;
    }
    if (!req.deviceId.empty()) body["device_id"] = req.deviceId;
    if (!req.initialDeviceDisplayName.empty())
        body["initial_device_display_name"] = req.initialDeviceDisplayName;

    try {
        json j = apiPost("/_matrix/client/v3/login", body);
        m_userId = j["user_id"];
        m_accessToken = j["access_token"];
        m_deviceId = j["device_id"];
        resp.userId = m_userId;
        resp.accessToken = m_accessToken;
        resp.deviceId = m_deviceId;
        resp.homeServer = m_config.homeserverUrl;
        LOGI("Login success: %s", m_userId.c_str());
    } catch (const std::exception& e) {
        LOGE("Login failed: %s", e.what());
    }
    return resp;
}

RegisterResponse MatrixClient::registerUser(const RegisterRequest& req) {
    RegisterResponse resp;
    json body;
    body["username"] = req.username;
    body["password"] = req.password;
    body["auth"] = {{"type", req.authType}};
    if (!req.initialDeviceDisplayName.empty())
        body["initial_device_display_name"] = req.initialDeviceDisplayName;
    body["inhibit_login"] = req.inhibitLogin;

    try {
        json j = apiPost("/_matrix/client/v3/register", body);
        if (j.contains("user_id")) {
            m_userId = j["user_id"];
            m_accessToken = j["access_token"];
            m_deviceId = j["device_id"];
            resp.userId = m_userId;
            resp.accessToken = m_accessToken;
            resp.deviceId = m_deviceId;
            resp.homeServer = m_config.homeserverUrl;
            resp.success = true;
        }
    } catch (const std::exception& e) {
        LOGE("Registration failed: %s", e.what());
    }
    return resp;
}

bool MatrixClient::logout() {
    try {
        apiPost("/_matrix/client/v3/logout", json::object());
        m_accessToken.clear();
        m_userId.clear();
        return true;
    } catch (...) { return false; }
}

// ── Sync ──

void MatrixClient::startSync(const std::string& since, int timeout, const std::string& filter) {
    if (!m_running) { m_running = true; LOGI("Sync started"); }
    std::map<std::string, std::string> params;
    if (!since.empty()) params["since"] = since;
    else if (!m_nextBatch.empty()) params["since"] = m_nextBatch;
    params["timeout"] = std::to_string(timeout);
    if (!filter.empty()) params["filter"] = filter;

    try {
        json j = apiGet("/_matrix/client/v3/sync", params);
        m_nextBatch = j.value("next_batch", m_nextBatch);
        if (m_eventCallback && j.contains("rooms")) {
            auto& rooms = j["rooms"];
            for (auto& [membership, roomMap] : rooms.items()) {
                for (auto& [roomId, roomData] : roomMap.items()) {
                    if (roomData.contains("timeline") &&
                        roomData["timeline"].contains("events")) {
                        for (auto& event : roomData["timeline"]["events"]) {
                            m_eventCallback(roomId, event);
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        LOGE("Sync error: %s", e.what());
        if (m_errorCallback) m_errorCallback(e.what());
    }
}

void MatrixClient::stopSync() { m_running = false; }

// ── Room operations ──

std::string MatrixClient::createRoom(const std::string& name, bool isDM,
                                      const std::vector<std::string>& invite) {
    json body;
    if (!name.empty()) body["name"] = name;
    if (isDM) body["is_direct"] = true;
    if (!invite.empty()) body["invite"] = invite;
    if (isDM) body["preset"] = "trusted_private_chat";
    else body["preset"] = "private_chat";

    try {
        json j = apiPost("/_matrix/client/v3/createRoom", body);
        return j["room_id"];
    } catch (...) { return ""; }
}

bool MatrixClient::joinRoom(const std::string& roomIdOrAlias) {
    std::string path = "/_matrix/client/v3/join/" + roomIdOrAlias;
    try { apiPost(path, json::object()); return true; }
    catch (...) { return false; }
}

bool MatrixClient::leaveRoom(const std::string& roomId) {
    try { apiPost("/_matrix/client/v3/rooms/" + roomId + "/leave", json::object()); return true; }
    catch (...) { return false; }
}

bool MatrixClient::inviteUser(const std::string& roomId, const std::string& userId) {
    json body{{"user_id", userId}};
    try { apiPost("/_matrix/client/v3/rooms/" + roomId + "/invite", body); return true; }
    catch (...) { return false; }
}

bool MatrixClient::kickUser(const std::string& roomId, const std::string& userId,
                              const std::string& reason) {
    json body{{"user_id", userId}};
    if (!reason.empty()) body["reason"] = reason;
    try { apiPost("/_matrix/client/v3/rooms/" + roomId + "/kick", body); return true; }
    catch (...) { return false; }
}

// ── Messages ──

std::string MatrixClient::sendMessage(const std::string& roomId, const std::string& body,
                                       const std::string& msgType) {
    std::string txnId = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    json content{{"msgtype", msgType}, {"body", body}};
    std::string path = "/_matrix/client/v3/rooms/" + roomId + "/send/m.room.message/" + txnId;
    try {
        json j = apiPut(path, content);
        return j["event_id"];
    } catch (...) { return ""; }
}

bool MatrixClient::redactEvent(const std::string& roomId, const std::string& eventId,
                                 const std::string& reason) {
    std::string txnId = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    json body;
    if (!reason.empty()) body["reason"] = reason;
    try { apiPut("/_matrix/client/v3/rooms/" + roomId + "/redact/" + eventId + "/" + txnId, body); return true; }
    catch (...) { return false; }
}

// ── Media ──

json MatrixClient::getUserProfile(const std::string& userId) {
    try { return apiGet("/_matrix/client/v3/profile/" + userId); }
    catch (...) { return json::object(); }
}

// ── HTTP helpers ──

std::string MatrixClient::buildUrl(const std::string& path,
                                     const std::map<std::string, std::string>& params) {
    std::ostringstream url;
    url << m_config.homeserverUrl << path;
    if (!params.empty()) {
        url << "?";
        bool first = true;
        for (auto& [k, v] : params) {
            if (!first) url << "&";
            url << k << "=" << v;
            first = false;
        }
    }
    return url.str();
}

json MatrixClient::apiGet(const std::string& path,
                           const std::map<std::string, std::string>& params) {
    return apiPost(path, json::object()); // simplified; real impl uses curl
}

json MatrixClient::apiPost(const std::string& path, const json& body) {
    // Real implementation uses curl_easy with headers:
    // Authorization: Bearer <token>
    // Content-Type: application/json
    // This is a skeleton that would use libcurl in production
    json result;
    return result;
}

json MatrixClient::apiPut(const std::string& path, const json& body) {
    return apiPost(path, body);
}

bool MatrixClient::start() { m_running = true; return true; }
void MatrixClient::stop() { m_running = false; }
bool MatrixClient::refreshToken() { return false; }
void MatrixClient::setSyncCallback(std::function<void(const json&)> cb) {}

std::string MatrixClient::sendFormattedMessage(const std::string&, const std::string&,
                                                const std::string&, const std::string&) { return ""; }
bool MatrixClient::editMessage(const std::string&, const std::string&, const std::string&) { return false; }
std::string MatrixClient::uploadMedia(const std::string&, const std::string&) { return ""; }
std::string MatrixClient::getMediaUrl(const std::string& mxcUri) { return mxcUri; }
bool MatrixClient::downloadMedia(const std::string&, const std::string&) { return false; }
bool MatrixClient::setDisplayName(const std::string&) { return false; }
bool MatrixClient::setAvatarUrl(const std::string&) { return false; }
json MatrixClient::getRoomState(const std::string&) { return json::object(); }
bool MatrixClient::setRoomName(const std::string&, const std::string&) { return false; }
bool MatrixClient::setRoomTopic(const std::string&, const std::string&) { return false; }
json MatrixClient::getRoomMembers(const std::string&) { return json::object(); }
json MatrixClient::getRoomMessages(const std::string&, const std::string&, int, const std::string&) { return json::object(); }
json MatrixClient::getContextForEvent(const std::string&, const std::string&, int) { return json::object(); }
bool MatrixClient::sendTyping(const std::string&, bool, int) { return false; }
bool MatrixClient::sendReadReceipt(const std::string&, const std::string&) { return false; }
bool MatrixClient::setPresence(const std::string&, const std::string&) { return false; }
json MatrixClient::searchMessages(const std::string&, const std::string&, int) { return json::object(); }
json MatrixClient::getPublicRooms(const std::string&, int) { return json::object(); }
bool MatrixClient::setAccountData(const std::string&, const json&) { return false; }
json MatrixClient::getAccountData(const std::string&) { return json::object(); }
int MatrixClient::getNotificationCount() { return 0; }
json MatrixClient::getPushRules() { return json::object(); }
bool MatrixClient::setPushRule(const std::string&, const std::string&, const std::string&, const json&) { return false; }
json MatrixClient::getDevices() { return json::object(); }
bool MatrixClient::deleteDevice(const std::string&) { return false; }
bool MatrixClient::updateDeviceDisplayName(const std::string&, const std::string&) { return false; }
json MatrixClient::getContentConfig() { return json::object(); }
bool MatrixClient::uploadDeviceKeys(const json&) { return false; }
json MatrixClient::claimOneTimeKeys(const std::string&, const std::string&, const std::string&) { return json::object(); }
bool MatrixClient::sendToDevice(const std::string&, const std::string&, const json&) { return false; }
json MatrixClient::getKeysChanges(const std::string&, const std::string&) { return json::object(); }
json MatrixClient::handleResponse(const std::string&, int) { return json::object(); }

} // namespace matrix_sdk

// ── Extended MatrixClient API implementations ──

namespace matrix_sdk {

// Room aliases
std::string MatrixClient::resolveRoomAlias(const std::string& alias) {
    if (alias.empty() || alias[0] != '#') return alias;
    try {
        json j = apiGet("/_matrix/client/v3/directory/room/" + alias);
        return j.value("room_id", "");
    } catch (...) { return alias; }
}

std::string MatrixClient::createRoomAlias(const std::string& roomId, const std::string& alias) {
    json body{{"room_id", roomId}};
    try { apiPut("/_matrix/client/v3/directory/room/" + alias, body); return alias; }
    catch (...) { return ""; }
}

bool MatrixClient::deleteRoomAlias(const std::string& alias) {
    try { apiDelete("/_matrix/client/v3/directory/room/" + alias); return true; }
    catch (...) { return false; }
}

std::vector<std::string> MatrixClient::getRoomAliases(const std::string& roomId) {
    try {
        json j = apiGet("/_matrix/client/v3/rooms/" + roomId + "/aliases");
        if (j.contains("aliases")) {
            std::vector<std::string> result;
            for (auto& a : j["aliases"]) result.push_back(a);
            return result;
        }
    } catch (...) {}
    return {};
}

// Room directory
json MatrixClient::getRoomDirectory(const std::string& roomId) {
    try { return apiGet("/_matrix/client/v3/directory/room/" + roomId); }
    catch (...) { return json::object(); }
}

// Joined rooms
std::vector<std::string> MatrixClient::getJoinedRooms() {
    try {
        json j = apiGet("/_matrix/client/v3/joined_rooms");
        if (j.contains("joined_rooms")) {
            std::vector<std::string> result;
            for (auto& r : j["joined_rooms"]) result.push_back(r);
            return result;
        }
    } catch (...) {}
    return {};
}

// Room tags
json MatrixClient::getRoomTags(const std::string& roomId) {
    try { return apiGet("/_matrix/client/v3/user/" + m_userId + "/rooms/" + roomId + "/tags"); }
    catch (...) { return json::object(); }
}

bool MatrixClient::setRoomTag(const std::string& roomId, const std::string& tag,
                                const json& order) {
    json body;
    if (!order.is_null()) body = order;
    try {
        apiPut("/_matrix/client/v3/user/" + m_userId + "/rooms/" + roomId + "/tags/" + tag, body);
        return true;
    } catch (...) { return false; }
}

bool MatrixClient::deleteRoomTag(const std::string& roomId, const std::string& tag) {
    try {
        apiDelete("/_matrix/client/v3/user/" + m_userId + "/rooms/" + roomId + "/tags/" + tag);
        return true;
    } catch (...) { return false; }
}

// Room state
bool MatrixClient::sendStateEvent(const std::string& roomId, const std::string& eventType,
                                   const std::string& stateKey, const json& content) {
    std::string path = "/_matrix/client/v3/rooms/" + roomId + "/state/" + eventType;
    if (!stateKey.empty()) path += "/" + stateKey;
    try { apiPut(path, content); return true; }
    catch (...) { return false; }
}

json MatrixClient::getStateEvent(const std::string& roomId, const std::string& eventType,
                                  const std::string& stateKey) {
    std::string path = "/_matrix/client/v3/rooms/" + roomId + "/state/" + eventType;
    if (!stateKey.empty()) path += "/" + stateKey;
    try { return apiGet(path); }
    catch (...) { return json::object(); }
}

// Event relations
json MatrixClient::getEventRelations(const std::string& roomId, const std::string& eventId,
                                       const std::string& relType, const std::string& from, int limit) {
    std::string path = "/_matrix/client/v3/rooms/" + roomId + "/relations/" + eventId;
    if (!relType.empty()) path += "/" + relType;
    std::map<std::string, std::string> params;
    if (!from.empty()) params["from"] = from;
    if (limit > 0) params["limit"] = std::to_string(limit);
    try { return apiGet(path, params); }
    catch (...) { return json::object(); }
}

// Threads
json MatrixClient::getThreads(const std::string& roomId, const std::string& from, int limit) {
    std::map<std::string, std::string> params;
    if (!from.empty()) params["from"] = from;
    if (limit > 0) params["limit"] = std::to_string(limit);
    try { return apiGet("/_matrix/client/v3/rooms/" + roomId + "/threads", params); }
    catch (...) { return json::object(); }
}

// Room upgrade
std::string MatrixClient::upgradeRoom(const std::string& roomId, const std::string& newVersion) {
    json body{{"new_version", newVersion}};
    try {
        json j = apiPost("/_matrix/client/v3/rooms/" + roomId + "/upgrade", body);
        return j.value("replacement_room", "");
    } catch (...) { return ""; }
}

// Reporting
bool MatrixClient::reportEvent(const std::string& roomId, const std::string& eventId,
                                 int score, const std::string& reason) {
    json body{{"score", score}, {"reason", reason}};
    try {
        apiPost("/_matrix/client/v3/rooms/" + roomId + "/report/" + eventId, body);
        return true;
    } catch (...) { return false; }
}

// Ignore/unignore users
bool MatrixClient::ignoreUser(const std::string& userId) {
    json data = getAccountData("m.ignored_user_list");
    if (data.is_null()) data["ignored_users"] = json::object();
    data["ignored_users"][userId] = json::object();
    return setAccountData("m.ignored_user_list", data);
}

bool MatrixClient::unignoreUser(const std::string& userId) {
    json data = getAccountData("m.ignored_user_list");
    if (data.contains("ignored_users")) {
        data["ignored_users"].erase(userId);
    }
    return setAccountData("m.ignored_user_list", data);
}

std::vector<std::string> MatrixClient::getIgnoredUsers() {
    json data = getAccountData("m.ignored_user_list");
    std::vector<std::string> result;
    if (data.contains("ignored_users")) {
        for (auto& [userId, _] : data["ignored_users"].items()) {
            result.push_back(userId);
        }
    }
    return result;
}

// Direct messages
bool MatrixClient::setDirectMessage(const std::string& roomId, const std::string& userId) {
    json data = getAccountData("m.direct");
    if (data.is_null()) data = json::object();
    if (!data.contains(userId)) data[userId] = json::array();
    bool found = false;
    for (auto& r : data[userId]) {
        if (r == roomId) { found = true; break; }
    }
    if (!found) data[userId].push_back(roomId);
    return setAccountData("m.direct", data);
}

std::map<std::string, std::vector<std::string>> MatrixClient::getDirectMessages() {
    json data = getAccountData("m.direct");
    std::map<std::string, std::vector<std::string>> result;
    for (auto& [userId, rooms] : data.items()) {
        for (auto& room : rooms) {
            result[userId].push_back(room);
        }
    }
    return result;
}

// Read markers
bool MatrixClient::setReadMarkers(const std::string& roomId, const std::string& fullyRead,
                                    const std::string& readReceipt) {
    json body;
    if (!fullyRead.empty()) body["m.fully_read"] = fullyRead;
    if (!readReceipt.empty()) body["m.read"] = readReceipt;
    try {
        apiPost("/_matrix/client/v3/rooms/" + roomId + "/read_markers", body);
        return true;
    } catch (...) { return false; }
}

// OpenID token (for Jitsi, etc.)
std::string MatrixClient::getOpenIdToken() {
    try {
        json j = apiPost("/_matrix/client/v3/user/" + m_userId + "/openid/request_token",
                         json::object());
        return j.value("access_token", "");
    } catch (...) { return ""; }
}

// Server capabilities
json MatrixClient::getCapabilities() {
    try { return apiGet("/_matrix/client/v3/capabilities"); }
    catch (...) { return json::object(); }
}

// Server version
json MatrixClient::getServerVersion() {
    try { return apiGet("/_matrix/client/versions"); }
    catch (...) { return json::object(); }
}

// Session validation
bool MatrixClient::validateSession() {
    try {
        json j = apiGet("/_matrix/client/v3/account/whoami");
        return j.contains("user_id") && j["user_id"] == m_userId;
    } catch (...) { return false; }
}

// Change password
bool MatrixClient::changePassword(const std::string& oldPassword,
                                    const std::string& newPassword) {
    json body{{"new_password", newPassword}};
    // Would need to handle UIA (User-Interactive Auth) for this
    try {
        apiPost("/_matrix/client/v3/account/password", body);
        return true;
    } catch (...) { return false; }
}

// Account deactivation
bool MatrixClient::deactivateAccount(const std::string& auth) {
    json body;
    if (!auth.empty()) body["auth"] = {{"type", "m.login.password"}, {"user", m_userId},
                                        {"password", auth}};
    try {
        apiPost("/_matrix/client/v3/account/deactivate", body);
        return true;
    } catch (...) { return false; }
}

// Three-PID management
bool MatrixClient::add3Pid(const std::string& address, const std::string& medium) {
    json body{{"three_pid_creds", {{"client_secret", generateTxnId()},
                                     {"id_server", m_config.identityServerUrl},
                                     {"sid", ""}}}};
    try { apiPost("/_matrix/client/v3/account/3pid", body); return true; }
    catch (...) { return false; }
}

bool MatrixClient::delete3Pid(const std::string& address, const std::string& medium) {
    json body{{"address", address}, {"medium", medium}};
    try { apiPost("/_matrix/client/v3/account/3pid/delete", body); return true; }
    catch (...) { return false; }
}

json MatrixClient::get3Pids() {
    try { return apiGet("/_matrix/client/v3/account/3pid"); }
    catch (...) { return json::object(); }
}

static std::string generateTxnId() {
    static std::atomic<uint64_t> counter{0};
    return std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count())
        + "_" + std::to_string(counter.fetch_add(1));
}

} // namespace matrix_sdk
