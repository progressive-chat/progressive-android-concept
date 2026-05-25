#ifndef MATRIX_SDK_NETWORK_HPP
#define MATRIX_SDK_NETWORK_HPP

#include "matrix_sdk_types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace matrix_sdk {

// ── HTTP Method ──
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS
};

// ── HTTP Request ──
struct HttpRequest {
    HttpMethod method = HttpMethod::GET;
    std::string url;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queryParams;
    int timeoutMs = 30000;
    int maxRetries = 3;
    bool retryOnConnectionError = true;
    bool retryOnRateLimit = true;
    std::string canonicalUrl;
};

// ── HTTP Response ──
struct HttpResponse {
    int httpCode = 0;
    std::string body;
    std::string errorBody;
    std::string errorMessage;
    std::map<std::string, std::string> headers;
    int64_t requestDurationMs = 0;
    int retryAfterMs = 0;
    bool isSuccess() const { return httpCode >= 200 && httpCode < 300; }
    bool isRateLimited() const { return httpCode == 429; }
    bool isAuthError() const { return httpCode == 401 || httpCode == 403; }
};

// ── Network Callback ──
using NetworkCallback = std::function<void(const HttpResponse& response)>;
using NetworkErrorCallback = std::function<void(const MatrixError& error)>;

// ── Token Store ──
class TokenStore {
public:
    virtual ~TokenStore() = default;
    virtual std::string getAccessToken() const = 0;
    virtual std::string getRefreshToken() const = 0;
    virtual void setTokens(const std::string& accessToken, const std::string& refreshToken) = 0;
    virtual void clearTokens() = 0;
    virtual bool hasValidToken() const = 0;
};

// ── HTTP Client ──
class HttpClient {
public:
    virtual ~HttpClient() = default;

    virtual void execute(const HttpRequest& request, NetworkCallback callback) = 0;
    virtual void executeWithRetry(const HttpRequest& request, int maxRetries, NetworkCallback callback) = 0;
    virtual void cancelAll() = 0;
    virtual void cancel(const std::string& requestId) = 0;

    virtual void setBaseUrl(const std::string& baseUrl) = 0;
    virtual std::string getBaseUrl() const = 0;

    virtual void setTokenStore(std::shared_ptr<TokenStore> store) = 0;
    virtual void setUserAgent(const std::string& userAgent) = 0;

    virtual void addHeader(const std::string& key, const std::string& value) = 0;
    virtual void removeHeader(const std::string& key) = 0;

    virtual void setConnectTimeout(int timeoutMs) = 0;
    virtual void setReadTimeout(int timeoutMs) = 0;
    virtual void setWriteTimeout(int timeoutMs) = 0;

    virtual int getActiveRequestCount() const = 0;
    virtual bool isOnline() const = 0;
};

// ── Request Signer ──
class RequestSigner {
public:
    virtual ~RequestSigner() = default;
    virtual void sign(HttpRequest& request) = 0;
    virtual bool canSign() const = 0;
};

// ── Network Monitor ──
class NetworkMonitor {
public:
    using ConnectivityCallback = std::function<void(bool isConnected)>;

    virtual ~NetworkMonitor() = default;
    virtual bool isConnected() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void setCallback(ConnectivityCallback callback) = 0;
};

// ── Rate Limiter ──
class RateLimiter {
public:
    virtual ~RateLimiter() = default;

    virtual bool canExecute(const std::string& endpoint) const = 0;
    virtual void recordRequest(const std::string& endpoint, int retryAfterMs) = 0;
    virtual int getRetryAfterMs(const std::string& endpoint) const = 0;
    virtual void clear() = 0;
};

// ── API Endpoint Builder ──
class ApiEndpointBuilder {
public:
    virtual ~ApiEndpointBuilder() = default;

    // Auth endpoints
    virtual std::string login() const = 0;
    virtual std::string logout() const = 0;
    virtual std::string registerAccount(const std::string& kind) const = 0;
    virtual std::string refreshToken() const = 0;
    virtual std::string changePassword() const = 0;
    virtual std::string deactivateAccount() const = 0;

    // Sync endpoints
    virtual std::string sync(const std::string& filter, const std::string& since, int timeout, bool fullState) const = 0;
    virtual std::string filter(const std::string& userId) const = 0;
    virtual std::string filterById(const std::string& userId, const std::string& filterId) const = 0;

    // Room endpoints
    virtual std::string createRoom() const = 0;
    virtual std::string joinRoom(const std::string& roomId) const = 0;
    virtual std::string joinRoomByAlias(const std::string& alias) const = 0;
    virtual std::string leaveRoom(const std::string& roomId) const = 0;
    virtual std::string forgetRoom(const std::string& roomId) const = 0;
    virtual std::string inviteToRoom(const std::string& roomId) const = 0;
    virtual std::string kickFromRoom(const std::string& roomId) const = 0;
    virtual std::string banFromRoom(const std::string& roomId) const = 0;
    virtual std::string unbanFromRoom(const std::string& roomId) const = 0;
    virtual std::string roomMembers(const std::string& roomId) const = 0;
    virtual std::string roomJoinedMembers(const std::string& roomId) const = 0;
    virtual std::string roomMessages(const std::string& roomId) const = 0;
    virtual std::string roomContext(const std::string& roomId, const std::string& eventId) const = 0;
    virtual std::string roomState(const std::string& roomId) const = 0;
    virtual std::string roomStateEvent(const std::string& roomId, const std::string& eventType) const = 0;
    virtual std::string roomStateEventKey(const std::string& roomId, const std::string& eventType, const std::string& stateKey) const = 0;
    virtual std::string roomSendEvent(const std::string& roomId, const std::string& eventType, const std::string& txnId) const = 0;
    virtual std::string roomRedact(const std::string& roomId, const std::string& eventId, const std::string& txnId) const = 0;
    virtual std::string roomTyping(const std::string& roomId, const std::string& userId) const = 0;
    virtual std::string roomReadMarkers(const std::string& roomId) const = 0;
    virtual std::string roomReceipt(const std::string& roomId, const std::string& receiptType, const std::string& eventId) const = 0;
    virtual std::string roomReport(const std::string& roomId, const std::string& eventId) const = 0;
    virtual std::string roomUpgrade(const std::string& roomId) const = 0;
    virtual std::string roomAliases(const std::string& roomId) const = 0;
    virtual std::string roomEventRelations(const std::string& roomId, const std::string& eventId, const std::string& relType) const = 0;
    virtual std::string roomThreads(const std::string& roomId) const = 0;

    // User endpoints
    virtual std::string userProfile(const std::string& userId) const = 0;
    virtual std::string userDisplayName(const std::string& userId) const = 0;
    virtual std::string userAvatarUrl(const std::string& userId) const = 0;
    virtual std::string userSearch(const std::string& query, int limit) const = 0;
    virtual std::string userPresence(const std::string& userId) const = 0;
    virtual std::string userSetPresence(const std::string& userId) const = 0;

    // Device endpoints
    virtual std::string devices() const = 0;
    virtual std::string device(const std::string& deviceId) const = 0;
    virtual std::string deleteDevice(const std::string& deviceId) const = 0;
    virtual std::string deleteDevices(const std::string& userId, const std::string& deviceId) const = 0;

    // Crypto endpoints
    virtual std::string uploadKeys() const = 0;
    virtual std::string uploadDeviceSigningKeys() const = 0;
    virtual std::string uploadDeviceSigningKey(const std::string& keyId) const = 0;
    virtual std::string uploadSignatures() const = 0;
    virtual std::string queryKeys() const = 0;
    virtual std::string claimKeys() const = 0;
    virtual std::string keyChanges(const std::string& from, const std::string& to) const = 0;
    virtual std::string sendToDevice(const std::string& eventType, const std::string& txnId) const = 0;

    // Key backup endpoints
    virtual std::string roomKeysVersion(const std::string& version = "") const = 0;
    virtual std::string roomKeysForRoom(const std::string& version, const std::string& roomId) const = 0;
    virtual std::string roomKeysForSession(const std::string& version, const std::string& roomId, const std::string& sessionId) const = 0;
    virtual std::string deleteRoomKeysVersion(const std::string& version) const = 0;

    // Push endpoints
    virtual std::string pushRules() const = 0;
    virtual std::string pushRule(const std::string& scope, const std::string& kind, const std::string& ruleId) const = 0;
    virtual std::string pushRuleActions(const std::string& scope, const std::string& kind, const std::string& ruleId) const = 0;
    virtual std::string pushRuleEnable(const std::string& scope, const std::string& kind, const std::string& ruleId) const = 0;
    virtual std::string pushers() const = 0;

    // Media endpoints
    virtual std::string mediaUpload() const = 0;
    virtual std::string mediaDownload(const std::string& serverName, const std::string& mediaId) const = 0;
    virtual std::string mediaThumbnail(const std::string& serverName, const std::string& mediaId) const = 0;
    virtual std::string mediaConfig() const = 0;
    virtual std::string mediaPreview(const std::string& url) const = 0;

    // Directory endpoints
    virtual std::string publicRooms() const = 0;
    virtual std::string roomAliasResolve(const std::string& alias) const = 0;
    virtual std::string roomAliasCreate(const std::string& alias) const = 0;
    virtual std::string roomAliasDelete(const std::string& alias) const = 0;

    // Search endpoints
    virtual std::string search() const = 0;
    virtual std::string searchUserDirectory() const = 0;

    // Third party endpoints
    virtual std::string thirdPartyProtocol(const std::string& protocol) const = 0;
    virtual std::string thirdPartyUser(const std::string& protocol, const std::string& fields) const = 0;
    virtual std::string thirdPartyLocation(const std::string& protocol, const std::string& fields) const = 0;

    // Account endpoints
    virtual std::string accountWhoami() const = 0;
    virtual std::string accountThreePid() const = 0;
    virtual std::string accountThreePidToken(const std::string& type) const = 0;
    virtual std::string accountPassword() const = 0;
    virtual std::string accountDeactivate() const = 0;

    // Admin endpoints
    virtual std::string adminWhois(const std::string& userId) const = 0;

    // OpenID endpoints
    virtual std::string openIdToken() const = 0;

    // Well-known
    virtual std::string wellKnown(const std::string& domain) const = 0;
    virtual std::string versions() const = 0;
    virtual std::string capabilities() const = 0;

    // Space endpoints
    virtual std::string spaceHierarchy(const std::string& roomId) const = 0;

    // Widget endpoints
    virtual std::string widgets(const std::string& roomId) const = 0;

    // Knocking
    virtual std::string knockRoom(const std::string& roomId) const = 0;

    // Terms
    virtual std::string terms(const std::string& baseUrl) const = 0;
    virtual std::string agreeTerms(const std::string& baseUrl) const = 0;

    // Identity server
    virtual std::string identityLookup(const std::string& identityServer) const = 0;
    virtual std::string identityBulkLookup(const std::string& identityServer) const = 0;
    virtual std::string identityStoreInvite(const std::string& identityServer) const = 0;
};

} // namespace matrix_sdk

#endif
