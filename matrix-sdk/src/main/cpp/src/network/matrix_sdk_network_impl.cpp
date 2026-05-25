#include "matrix_sdk/network/matrix_sdk_network.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

namespace matrix_sdk {

using json = nlohmann::json;

// ── TokenStoreImpl ──
class TokenStoreImpl : public TokenStore {
public:
    std::string getAccessToken() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return accessToken_;
    }

    std::string getRefreshToken() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return refreshToken_;
    }

    void setTokens(const std::string& accessToken, const std::string& refreshToken) override {
        std::lock_guard<std::mutex> lock(mutex_);
        accessToken_ = accessToken;
        refreshToken_ = refreshToken;
    }

    void clearTokens() override {
        std::lock_guard<std::mutex> lock(mutex_);
        accessToken_.clear();
        refreshToken_.clear();
    }

    bool hasValidToken() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return !accessToken_.empty();
    }

private:
    mutable std::mutex mutex_;
    std::string accessToken_;
    std::string refreshToken_;
};

// ── RateLimiterImpl ──
class RateLimiterImpl : public RateLimiter {
public:
    bool canExecute(const std::string& endpoint) const override {
        auto it = delayed_.find(endpoint);
        if (it == delayed_.end()) return true;
        auto now = std::chrono::steady_clock::now();
        return now >= it->second;
    }

    void recordRequest(const std::string& endpoint, int retryAfterMs) override {
        if (retryAfterMs > 0) {
            auto retryTime = std::chrono::steady_clock::now() +
                std::chrono::milliseconds(retryAfterMs);
            delayed_[endpoint] = retryTime;
        }
    }

    int getRetryAfterMs(const std::string& endpoint) const override {
        auto it = delayed_.find(endpoint);
        if (it == delayed_.end()) return 0;
        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(it->second - now).count();
        return diff > 0 ? static_cast<int>(diff) : 0;
    }

    void clear() override {
        delayed_.clear();
    }

private:
    std::map<std::string, std::chrono::steady_clock::time_point> delayed_;
};

// ── ApiEndpointBuilderImpl ──
class ApiEndpointBuilderImpl : public ApiEndpointBuilder {
public:
    explicit ApiEndpointBuilderImpl(const std::string& baseUrl) : baseUrl_(baseUrl) {}

    std::string makeUrl(const std::string& path) const {
        std::string url = baseUrl_;
        if (!url.empty() && url.back() == '/') url.pop_back();
        return url + path;
    }

    // Auth
    std::string login() const override { return makeUrl("/_matrix/client/r0/login"); }
    std::string logout() const override { return makeUrl("/_matrix/client/r0/logout"); }
    std::string registerAccount(const std::string& kind) const override {
        return makeUrl("/_matrix/client/r0/register" + (kind.empty() ? "" : "?kind=" + kind));
    }
    std::string refreshToken() const override { return makeUrl("/_matrix/client/r0/refresh"); }
    std::string changePassword() const override { return makeUrl("/_matrix/client/r0/account/password"); }
    std::string deactivateAccount() const override { return makeUrl("/_matrix/client/r0/account/deactivate"); }

    // Sync
    std::string sync(const std::string& filter, const std::string& since, int timeout, bool fullState) const override {
        std::string url = makeUrl("/_matrix/client/r0/sync?timeout=" + std::to_string(timeout));
        if (!filter.empty()) url += "&filter=" + filter;
        if (!since.empty()) url += "&since=" + since;
        if (fullState) url += "&full_state=true";
        return url;
    }
    std::string filter(const std::string& userId) const override {
        return makeUrl("/_matrix/client/r0/user/" + userId + "/filter");
    }
    std::string filterById(const std::string& userId, const std::string& filterId) const override {
        return makeUrl("/_matrix/client/r0/user/" + userId + "/filter/" + filterId);
    }

    // Room
    std::string createRoom() const override { return makeUrl("/_matrix/client/r0/createRoom"); }
    std::string joinRoom(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/join");
    }
    std::string joinRoomByAlias(const std::string& alias) const override {
        return makeUrl("/_matrix/client/r0/join/" + alias);
    }
    std::string leaveRoom(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/leave");
    }
    std::string forgetRoom(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/forget");
    }
    std::string inviteToRoom(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/invite");
    }
    std::string kickFromRoom(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/kick");
    }
    std::string banFromRoom(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/ban");
    }
    std::string unbanFromRoom(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/unban");
    }
    std::string roomMembers(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/members");
    }
    std::string roomJoinedMembers(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/joined_members");
    }
    std::string roomMessages(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/messages");
    }
    std::string roomContext(const std::string& roomId, const std::string& eventId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/context/" + eventId);
    }
    std::string roomState(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/state");
    }
    std::string roomStateEvent(const std::string& roomId, const std::string& eventType) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/state/" + eventType);
    }
    std::string roomStateEventKey(const std::string& roomId, const std::string& eventType, const std::string& stateKey) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/state/" + eventType + "/" + stateKey);
    }
    std::string roomSendEvent(const std::string& roomId, const std::string& eventType, const std::string& txnId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/send/" + eventType + "/" + txnId);
    }
    std::string roomRedact(const std::string& roomId, const std::string& eventId, const std::string& txnId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/redact/" + eventId + "/" + txnId);
    }
    std::string roomTyping(const std::string& roomId, const std::string& userId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/typing/" + userId);
    }
    std::string roomReadMarkers(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/read_markers");
    }
    std::string roomReceipt(const std::string& roomId, const std::string& receiptType, const std::string& eventId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/receipt/" + receiptType + "/" + eventId);
    }
    std::string roomReport(const std::string& roomId, const std::string& eventId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/report/" + eventId);
    }
    std::string roomUpgrade(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/upgrade");
    }
    std::string roomAliases(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/aliases");
    }
    std::string roomEventRelations(const std::string& roomId, const std::string& eventId, const std::string& relType) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/relations/" + eventId + "/" + relType);
    }
    std::string roomThreads(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/threads");
    }

    // User
    std::string userProfile(const std::string& userId) const override {
        return makeUrl("/_matrix/client/r0/profile/" + userId);
    }
    std::string userDisplayName(const std::string& userId) const override {
        return makeUrl("/_matrix/client/r0/profile/" + userId + "/displayname");
    }
    std::string userAvatarUrl(const std::string& userId) const override {
        return makeUrl("/_matrix/client/r0/profile/" + userId + "/avatar_url");
    }
    std::string userSearch(const std::string& query, int limit) const override {
        return makeUrl("/_matrix/client/r0/user_directory/search");
    }
    std::string userPresence(const std::string& userId) const override {
        return makeUrl("/_matrix/client/r0/presence/" + userId + "/status");
    }
    std::string userSetPresence(const std::string& userId) const override {
        return makeUrl("/_matrix/client/r0/presence/" + userId + "/status");
    }

    // Device
    std::string devices() const override { return makeUrl("/_matrix/client/r0/devices"); }
    std::string device(const std::string& deviceId) const override {
        return makeUrl("/_matrix/client/r0/devices/" + deviceId);
    }
    std::string deleteDevice(const std::string& deviceId) const override {
        return makeUrl("/_matrix/client/r0/devices/" + deviceId);
    }
    std::string deleteDevices(const std::string& userId, const std::string& deviceId) const override {
        return makeUrl("/_matrix/client/r0/delete_devices");
    }

    // Crypto
    std::string uploadKeys() const override { return makeUrl("/_matrix/client/r0/keys/upload"); }
    std::string uploadDeviceSigningKeys() const override { return makeUrl("/_matrix/client/r0/keys/device_signing/upload"); }
    std::string uploadDeviceSigningKey(const std::string& keyId) const override {
        return makeUrl("/_matrix/client/r0/keys/device_signing/upload/" + keyId);
    }
    std::string uploadSignatures() const override { return makeUrl("/_matrix/client/r0/keys/signatures/upload"); }
    std::string queryKeys() const override { return makeUrl("/_matrix/client/r0/keys/query"); }
    std::string claimKeys() const override { return makeUrl("/_matrix/client/r0/keys/claim"); }
    std::string keyChanges(const std::string& from, const std::string& to) const override {
        return makeUrl("/_matrix/client/r0/keys/changes?from=" + from + "&to=" + to);
    }
    std::string sendToDevice(const std::string& eventType, const std::string& txnId) const override {
        return makeUrl("/_matrix/client/r0/sendToDevice/" + eventType + "/" + txnId);
    }

    // Key backup
    std::string roomKeysVersion(const std::string& version) const override {
        std::string base = makeUrl("/_matrix/client/r0/room_keys/version");
        return version.empty() ? base : base + "/" + version;
    }
    std::string roomKeysForRoom(const std::string& version, const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/room_keys/keys/" + roomId + "?version=" + version);
    }
    std::string roomKeysForSession(const std::string& version, const std::string& roomId,
        const std::string& sessionId) const override {
        return makeUrl("/_matrix/client/r0/room_keys/keys/" + roomId + "/" + sessionId + "?version=" + version);
    }
    std::string deleteRoomKeysVersion(const std::string& version) const override {
        return makeUrl("/_matrix/client/r0/room_keys/version/" + version);
    }

    // Push
    std::string pushRules() const override { return makeUrl("/_matrix/client/r0/pushrules/"); }
    std::string pushRule(const std::string& scope, const std::string& kind, const std::string& ruleId) const override {
        return makeUrl("/_matrix/client/r0/pushrules/" + scope + "/" + kind + "/" + ruleId);
    }
    std::string pushRuleActions(const std::string& scope, const std::string& kind,
        const std::string& ruleId) const override {
        return makeUrl("/_matrix/client/r0/pushrules/" + scope + "/" + kind + "/" + ruleId + "/actions");
    }
    std::string pushRuleEnable(const std::string& scope, const std::string& kind,
        const std::string& ruleId) const override {
        return makeUrl("/_matrix/client/r0/pushrules/" + scope + "/" + kind + "/" + ruleId + "/enabled");
    }
    std::string pushers() const override { return makeUrl("/_matrix/client/r0/pushers"); }

    // Media
    std::string mediaUpload() const override { return makeUrl("/_matrix/media/r0/upload"); }
    std::string mediaDownload(const std::string& serverName, const std::string& mediaId) const override {
        return makeUrl("/_matrix/media/r0/download/" + serverName + "/" + mediaId);
    }
    std::string mediaThumbnail(const std::string& serverName, const std::string& mediaId) const override {
        return makeUrl("/_matrix/media/r0/thumbnail/" + serverName + "/" + mediaId);
    }
    std::string mediaConfig() const override { return makeUrl("/_matrix/media/r0/config"); }
    std::string mediaPreview(const std::string& url) const override {
        return makeUrl("/_matrix/media/r0/preview_url?url=" + url);
    }

    // Directory
    std::string publicRooms() const override { return makeUrl("/_matrix/client/r0/publicRooms"); }
    std::string roomAliasResolve(const std::string& alias) const override {
        return makeUrl("/_matrix/client/r0/directory/room/" + alias);
    }
    std::string roomAliasCreate(const std::string& alias) const override {
        return makeUrl("/_matrix/client/r0/directory/room/" + alias);
    }
    std::string roomAliasDelete(const std::string& alias) const override {
        return makeUrl("/_matrix/client/r0/directory/room/" + alias);
    }

    // Search
    std::string search() const override { return makeUrl("/_matrix/client/r0/search"); }
    std::string searchUserDirectory() const override { return makeUrl("/_matrix/client/r0/user_directory/search"); }

    // Third party
    std::string thirdPartyProtocol(const std::string& protocol) const override {
        return makeUrl("/_matrix/client/r0/thirdparty/protocol/" + protocol);
    }
    std::string thirdPartyUser(const std::string& protocol, const std::string& fields) const override {
        return makeUrl("/_matrix/client/r0/thirdparty/user/" + protocol);
    }
    std::string thirdPartyLocation(const std::string& protocol, const std::string& fields) const override {
        return makeUrl("/_matrix/client/r0/thirdparty/location/" + protocol);
    }

    // Account
    std::string accountWhoami() const override { return makeUrl("/_matrix/client/r0/account/whoami"); }
    std::string accountThreePid() const override { return makeUrl("/_matrix/client/r0/account/3pid"); }
    std::string accountThreePidToken(const std::string& type) const override {
        return makeUrl("/_matrix/client/r0/account/3pid/" + type + "/requestToken");
    }
    std::string accountPassword() const override { return makeUrl("/_matrix/client/r0/account/password"); }
    std::string accountDeactivate() const override { return makeUrl("/_matrix/client/r0/account/deactivate"); }

    // Admin
    std::string adminWhois(const std::string& userId) const override {
        return makeUrl("/_matrix/client/r0/admin/whois/" + userId);
    }

    // OpenID
    std::string openIdToken() const override { return makeUrl("/_matrix/client/r0/user/openid/request_token"); }

    // Well-known
    std::string wellKnown(const std::string& domain) const override {
        return makeUrl("/.well-known/matrix/client");
    }
    std::string versions() const override { return makeUrl("/_matrix/client/versions"); }
    std::string capabilities() const override { return makeUrl("/_matrix/client/r0/capabilities"); }

    // Space
    std::string spaceHierarchy(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/v1/rooms/" + roomId + "/hierarchy");
    }

    // Widget
    std::string widgets(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/rooms/" + roomId + "/widgets");
    }

    // Knock
    std::string knockRoom(const std::string& roomId) const override {
        return makeUrl("/_matrix/client/r0/knock/" + roomId);
    }

    // Terms
    std::string terms(const std::string& baseUrl) const override { return baseUrl + "/_matrix/client/r0/terms"; }
    std::string agreeTerms(const std::string& baseUrl) const override { return baseUrl + "/_matrix/client/r0/terms"; }

    // Identity
    std::string identityLookup(const std::string& identityServer) const override {
        return identityServer + "/_matrix/identity/v2/lookup";
    }
    std::string identityBulkLookup(const std::string& identityServer) const override {
        return identityServer + "/_matrix/identity/v2/bulk_lookup";
    }
    std::string identityStoreInvite(const std::string& identityServer) const override {
        return identityServer + "/_matrix/identity/v2/store-invite";
    }

private:
    std::string baseUrl_;
};

} // namespace matrix_sdk
