#include "matrix_sdk/session/services/matrix_sdk_session_services.hpp"
#include "matrix_sdk/session/matrix_sdk_session.hpp"
#include "matrix_sdk/matrix_sdk_types.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <vector>

namespace matrix_sdk {

// ============================================================================
// HomeServerCapabilitiesServiceImpl
// ============================================================================
class HomeServerCapabilitiesServiceImpl : public HomeServerCapabilitiesService {
public:
    explicit HomeServerCapabilitiesServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {
        // Set sensible defaults before capabilities are fetched
        capabilities_["m.change_password"] = false;
        capabilities_["m.deactivate_account"] = false;
        capabilities_["m.max_upload_size"] = "10485760"; // 10MB default
        capabilities_["io.element.thread"] = false;
        capabilities_["m.space"] = false;
        capabilities_["m.poll"] = false;
        capabilities_["m.location"] = false;
        capabilities_["m.beacon"] = false;
        capabilities_["io.element.voice_broadcast"] = false;
        supportedVersions_ = {"v1.0", "v1.1", "v1.2", "v1.3", "v1.4", "v1.5", "v1.6"};
    }

    void fetchCapabilities(std::function<void(bool, const std::string&)> callback) override {
        // GET /_matrix/client/v3/capabilities
        // Expected JSON response format:
        // {
        //   "capabilities": {
        //     "m.change_password": { "enabled": true/false },
        //     "m.room_versions": { "default": "9", "available": {"1": "stable", ...} },
        //     ...
        //   }
        // }

        // In a real implementation, this would make an HTTP request and parse the response.
        // For now, we simulate a successful fetch with reasonable defaults.

        std::lock_guard<std::mutex> lock(mutex_);

        // Try to detect capabilities from homeserver version info
        std::string homeserverUrl = session_->getHomeServerUrl();
        if (!homeserverUrl.empty()) {
            isFetched_ = true;
        }

        // Populate default capabilities (will be overridden by actual API response)
        capabilities_["m.change_password"] = true;
        capabilities_["m.deactivate_account"] = false; // many servers disable this
        capabilities_["m.max_upload_size"] = "104857600"; // 100MB
        capabilities_["io.element.thread"] = true;
        capabilities_["m.space"] = true;
        capabilities_["m.poll"] = true;
        capabilities_["m.location"] = true;
        capabilities_["m.beacon"] = true;
        capabilities_["io.element.voice_broadcast"] = false;

        lastFetched_ = currentTimeMs();
        callback(true, "");
    }

    bool canChangePassword() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCapabilityBool("m.change_password", false);
    }

    bool canDeactivateAccount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCapabilityBool("m.deactivate_account", false);
    }

    std::string getMaxUploadSize() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCapabilityString("m.max_upload_size", "10485760");
    }

    bool supportsThreads() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCapabilityBool("io.element.thread", false);
    }

    bool supportsSpaces() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCapabilityBool("m.space", false);
    }

    bool supportsPoll() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCapabilityBool("m.poll", false);
    }

    bool supportsLocationSharing() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCapabilityBool("m.location", false);
    }

    bool supportsBeacon() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCapabilityBool("m.beacon", false);
    }

    bool supportsVoiceBroadcast() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCapabilityBool("io.element.voice_broadcast", false);
    }

    bool supportsVersions(const std::vector<std::string>& versions) const override {
        if (versions.empty()) return false;

        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& v : versions) {
            bool found = false;
            for (const auto& sv : supportedVersions_) {
                if (sv == v) { found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    }

    std::vector<std::string> getSupportedVersions() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return supportedVersions_;
    }

    // Internal helpers
    void parseCapabilitiesResponse(const json& response) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!response.contains("capabilities")) return;

        const auto& caps = response["capabilities"];
        for (const auto& [key, value] : caps.items()) {
            if (value.is_object() && value.contains("enabled")) {
                capabilities_[key] = value["enabled"].get<bool>();
            } else if (key == "m.room_versions") {
                if (value.contains("available")) {
                    supportedVersions_.clear();
                    for (const auto& [ver, status] : value["available"].items()) {
                        supportedVersions_.push_back(ver);
                    }
                }
            } else if (value.is_string()) {
                capabilities_[key] = value.get<std::string>();
            }
        }
        isFetched_ = true;
        lastFetched_ = currentTimeMs();
    }

private:
    bool getCapabilityBool(const std::string& key, bool defaultValue) const {
        auto it = capabilities_.find(key);
        if (it != capabilities_.end()) {
            return it->second == "true" || it->second == "1" || it->second == "enabled";
        }
        return defaultValue;
    }

    std::string getCapabilityString(const std::string& key, const std::string& defaultValue) const {
        auto it = capabilities_.find(key);
        if (it != capabilities_.end()) {
            return it->second;
        }
        return defaultValue;
    }

    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::map<std::string, std::string> capabilities_;
    std::vector<std::string> supportedVersions_;
    bool isFetched_ = false;
    int64_t lastFetched_ = 0;
};

// ============================================================================
// IdentityServiceImpl
// ============================================================================
class IdentityServiceImpl : public IdentityService {
public:
    explicit IdentityServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void lookup(const std::string& address, const std::string& medium,
                std::function<void(bool, const std::string&, const std::string&)> callback) override {
        if (address.empty()) {
            callback(false, "Address is required", "");
            return;
        }
        if (medium != "email" && medium != "msisdn") {
            callback(false, "Unsupported medium: " + medium + ". Use 'email' or 'msisdn'", "");
            return;
        }

        std::string identityUrl = getIdentityServerUrl();
        if (identityUrl.empty()) {
            callback(false, "No identity server configured", "");
            return;
        }

        // GET /_matrix/identity/v2/lookup?medium={medium}&address={address}
        // In a real implementation, this queries the identity server.
        // Returns a Matrix user ID if found.

        // Simulate: try to find in local cache first
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::string cacheKey = medium + ":" + address;
            auto it = lookupCache_.find(cacheKey);
            if (it != lookupCache_.end()) {
                callback(true, it->second, "");
                return;
            }
        }

        callback(true, "", ""); // Success but no result found
    }

    void bulkLookup(const std::vector<std::string>& addresses, const std::string& medium,
                    std::function<void(bool, const std::string&, const std::map<std::string, std::string>&)> callback) override {
        if (addresses.empty()) {
            callback(false, "Addresses list is required", {});
            return;
        }
        if (medium != "email" && medium != "msisdn") {
            callback(false, "Unsupported medium: " + medium, {});
            return;
        }

        std::string identityUrl = getIdentityServerUrl();
        if (identityUrl.empty()) {
            callback(false, "No identity server configured", {});
            return;
        }

        // POST /_matrix/identity/v2/bulk_lookup
        // Body: { "threepids": [["medium", "address"], ...] }

        std::map<std::string, std::string> results;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& addr : addresses) {
                std::string cacheKey = medium + ":" + addr;
                auto it = lookupCache_.find(cacheKey);
                if (it != lookupCache_.end()) {
                    results[addr] = it->second;
                }
            }
        }

        callback(true, "", results);
    }

    void storeInvite(const std::string& roomId, const std::string& eventId,
                     const std::string& address, const std::string& medium,
                     const std::string& roomName, SimpleCallback callback) override {
        if (address.empty()) {
            callback(false, "Address is required");
            return;
        }
        if (medium != "email" && medium != "msisdn") {
            callback(false, "Unsupported medium: " + medium);
            return;
        }

        json body;
        body["address"] = address;
        body["medium"] = medium;
        body["room_id"] = roomId;
        body["room_name"] = roomName;
        body["sender"] = session_->getUserId();

        // POST /_matrix/identity/v2/store-invite
        // Stores an invite mapping so lookups return the correct room
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::string cacheKey = medium + ":" + address;
            storedInvites_[cacheKey] = {
                {"roomId", roomId},
                {"eventId", eventId},
                {"roomName", roomName}
            };
        }

        callback(true, "");
    }

    std::string getIdentityServerUrl() const override {
        // The identity server URL could come from:
        // 1. Well-known discovery
        // 2. Session params
        // 3. .well-known/matrix/client response
        if (!identityServerUrl_.empty()) {
            return identityServerUrl_;
        }
        return "";
    }

    // Internal helpers
    void setIdentityServerUrl(const std::string& url) {
        std::lock_guard<std::mutex> lock(mutex_);
        identityServerUrl_ = url;
    }

    void addToCache(const std::string& address, const std::string& medium,
                    const std::string& userId) {
        std::lock_guard<std::mutex> lock(mutex_);
        lookupCache_[medium + ":" + address] = userId;
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::string identityServerUrl_;
    std::unordered_map<std::string, std::string> lookupCache_; // "medium:address" -> userId
    std::unordered_map<std::string, std::map<std::string, std::string>> storedInvites_;
};

// ============================================================================
// IntegrationManagerServiceImpl
// ============================================================================
class IntegrationManagerServiceImpl : public IntegrationManagerService {
public:
    explicit IntegrationManagerServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {
        // Default integration manager URL (Element's scalar instance)
        integrationManagerUrl_ = "https://scalar.vector.im/api";
    }

    void getIntegrationManagers(
        std::function<void(bool, const std::string&, const std::vector<std::string>&)> callback) override {
        // GET /_matrix/client/v3/user/{userId}/account_data/m.widgets
        // Or from .well-known integration managers

        std::vector<std::string> managers;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!integrationManagerUrl_.empty()) {
                managers.push_back(integrationManagerUrl_);
            }
        }

        if (managers.empty()) {
            callback(false, "No integration managers configured", {});
            return;
        }

        callback(true, "", managers);
    }

    void openIntegrationManager(const std::string& integrationId, const std::string& roomId,
                                const std::string& screen, const std::string& integrationUrl,
                                SimpleCallback callback) override {
        if (integrationUrl.empty()) {
            // Build the URL from the configured manager
            std::string baseUrl = getIntegrationManagerUrl();
            if (baseUrl.empty()) {
                callback(false, "No integration manager configured");
                return;
            }

            std::string fullUrl = baseUrl + "?scalar_token=TODO&room_id=" + roomId;
            if (!screen.empty()) {
                fullUrl += "&screen=" + screen;
            }
            if (!integrationId.empty()) {
                fullUrl += "&integ_id=" + integrationId;
            }

            // Open widget with this URL
            // In real implementation, this would create/display a widget
            callback(true, fullUrl);
            return;
        }

        // Use the provided URL directly
        static const std::regex urlRegex(R"(^https?://)");
        if (!std::regex_search(integrationUrl, urlRegex)) {
            callback(false, "Integration URL must be HTTPS");
            return;
        }

        callback(true, integrationUrl);
    }

    std::string getIntegrationManagerUrl() const override {
        return integrationManagerUrl_;
    }

    // Internal helpers
    void setIntegrationManagerUrl(const std::string& url) {
        std::lock_guard<std::mutex> lock(mutex_);
        integrationManagerUrl_ = url;
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::string integrationManagerUrl_;
};

// ============================================================================
// MediaServiceImpl
// ============================================================================
class MediaServiceImpl : public MediaService {
public:
    explicit MediaServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void uploadMedia(const std::string& localPath, const std::string& filename,
                     const std::string& mimeType, ProgressCallback progress,
                     MediaCallback callback) override {
        if (localPath.empty()) {
            callback(false, "Local file path is required", "", MediaInfo{});
            return;
        }
        if (filename.empty()) {
            callback(false, "Filename is required", "", MediaInfo{});
            return;
        }

        // Check file exists (in real impl this would use stat/fstat)
        if (localPath.find("..") != std::string::npos) {
            callback(false, "Invalid file path: path traversal detected", "", MediaInfo{});
            return;
        }

        std::string uploadId = generateUploadId();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            UploadInfo info;
            info.localPath = localPath;
            info.filename = filename;
            info.mimeType = mimeType;
            info.status = "pending";
            info.startedAt = currentTimeMs();
            uploads_[uploadId] = info;
        }

        if (progress) progress(0, 0);

        // In a real impl: POST /_matrix/media/v3/upload with multipart form data
        // Simulate a successful upload with a generated MXC URL
        std::string mediaId = generateMediaId();
        std::string homeserver = parseHomeserverDomain(session_->getHomeServerUrl());
        std::string mxcUrl = "mxc://" + homeserver + "/" + mediaId;

        MediaInfo mediaInfo;
        mediaInfo.mimeType = mimeType;
        mediaInfo.size = 0; // Would come from stat

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = uploads_.find(uploadId);
            if (it != uploads_.end()) {
                it->second.status = "completed";
                it->second.mxcUrl = mxcUrl;
                it->second.mediaInfo = mediaInfo;
            }
            mxcCache_[mxcUrl] = localPath;
        }

        if (progress) progress(100, 100);

        callback(true, mxcUrl, "", mediaInfo);
    }

    void downloadMedia(const std::string& mxcUrl, ProgressCallback progress,
                       SimpleCallback callback) override {
        if (mxcUrl.empty()) {
            callback(false, "MXC URL is required");
            return;
        }

        // Validate MXC URL
        static const std::regex mxcRegex(R"(^mxc://[^/]+/[^/]+$)");
        if (!std::regex_match(mxcUrl, mxcRegex)) {
            callback(false, "Invalid MXC URL format");
            return;
        }

        // Check if already cached
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = cache_.find(mxcUrl);
            if (it != cache_.end()) {
                callback(true, it->second);
                return;
            }
        }

        if (progress) progress(0, 100);

        std::string downloadId = generateDownloadId();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            DownloadInfo info;
            info.mxcUrl = mxcUrl;
            info.status = "downloading";
            info.startedAt = currentTimeMs();
            downloads_[downloadId] = info;
        }

        // Build download URL
        auto parsed = parseMxc(mxcUrl);
        std::string downloadUrl = session_->getHomeServerUrl() +
            "/_matrix/media/v3/download/" + parsed.first + "/" + parsed.second;

        // Simulate: cache locally
        std::string cachePath = buildCachePath(mxcUrl);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            downloads_.erase(downloadId);
            cache_[mxcUrl] = cachePath;
        }

        if (progress) progress(100, 100);

        callback(true, cachePath);
    }

    std::string getCachedPath(const std::string& mxcUrl) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(mxcUrl);
        if (it != cache_.end()) {
            return it->second;
        }
        return "";
    }

    void getThumbnail(const std::string& mxcUrl, const ThumbnailSize& size,
                      std::function<void(bool, const std::string&, const std::string&)> callback) override {
        if (mxcUrl.empty()) {
            callback(false, "MXC URL is required", "");
            return;
        }
        if (size.width <= 0 || size.height <= 0) {
            callback(false, "Thumbnail dimensions must be positive", "");
            return;
        }

        // Check for cached thumbnail
        std::string cacheKey = mxcUrl + "_" + size.toQueryString();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = cache_.find(cacheKey);
            if (it != cache_.end()) {
                callback(true, it->second, "");
                return;
            }
        }

        // GET /_matrix/media/v3/thumbnail/{serverName}/{mediaId}?width=...&height=...
        auto parsed = parseMxc(mxcUrl);
        std::string thumbUrl = session_->getHomeServerUrl() +
            "/_matrix/media/v3/thumbnail/" + parsed.first + "/" + parsed.second +
            "?width=" + std::to_string(size.width) +
            "&height=" + std::to_string(size.height) +
            (size.method.empty() ? "" : "&method=" + size.method);

        // Simulate: store thumbnail path
        std::string thumbPath = buildCachePath(cacheKey);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            cache_[cacheKey] = thumbPath;
        }

        callback(true, thumbPath, "");
    }

    void cancelUpload(const std::string& uploadId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = uploads_.find(uploadId);
        if (it != uploads_.end()) {
            it->second.status = "cancelled";
            uploads_.erase(it);
        }
    }

    void cancelDownload(const std::string& downloadId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = downloads_.find(downloadId);
        if (it != downloads_.end()) {
            it->second.status = "cancelled";
            downloads_.erase(it);
        }
    }

    // Internal helpers
    void clearCache() {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
        mxcCache_.clear();
    }

    size_t getCacheSize() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }

private:
    struct UploadInfo {
        std::string localPath;
        std::string filename;
        std::string mimeType;
        std::string status;
        std::string mxcUrl;
        MediaInfo mediaInfo;
        int64_t startedAt = 0;
    };

    struct DownloadInfo {
        std::string mxcUrl;
        std::string status;
        int64_t startedAt = 0;
    };

    static std::string generateUploadId() {
        static std::atomic<uint64_t> counter{0};
        return "up_" + std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()) +
            "_" + std::to_string(++counter);
    }

    static std::string generateDownloadId() {
        static std::atomic<uint64_t> counter{0};
        return "dl_" + std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()) +
            "_" + std::to_string(++counter);
    }

    static std::string generateMediaId() {
        static std::atomic<uint64_t> counter{0};
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "media_" + std::to_string(now) + "_" + std::to_string(++counter);
    }

    static std::string parseHomeserverDomain(const std::string& url) {
        static const std::regex domainRegex(R"(https?://([^/:]+))");
        std::smatch match;
        if (std::regex_search(url, match, domainRegex)) {
            return match[1].str();
        }
        return "matrix.org";
    }

    static std::pair<std::string, std::string> parseMxc(const std::string& mxcUrl) {
        static const std::regex mxcRegex(R"(^mxc://([^/]+)/(.+)$)");
        std::smatch match;
        if (std::regex_match(mxcUrl, match, mxcRegex)) {
            return {match[1].str(), match[2].str()};
        }
        return {"", ""};
    }

    std::string buildCachePath(const std::string& key) const {
        std::size_t hash = std::hash<std::string>{}(key);
        return "/tmp/matrix-media-cache/" + std::to_string(hash);
    }

    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::string> cache_;           // key -> local path
    std::unordered_map<std::string, std::string> mxcCache_;        // mxc -> local path
    std::unordered_map<std::string, UploadInfo> uploads_;
    std::unordered_map<std::string, DownloadInfo> downloads_;
};

// ============================================================================
// OpenIdServiceImpl
// ============================================================================
class OpenIdServiceImpl : public OpenIdService {
public:
    explicit OpenIdServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void getOpenIdToken(std::function<void(bool, const std::string&, const OpenIdToken&)> callback) override {
        // POST /_matrix/client/v3/user/{userId}/openid/request_token
        // Body: {} (empty JSON object)

        // Check if we have a valid cached token
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (isTokenValid(currentToken_)) {
                callback(true, "", currentToken_);
                return;
            }
        }

        // Generate a fresh token (in real impl this comes from server)
        OpenIdToken token;
        token.accessToken = generateTokenString();
        token.tokenType = "Bearer";
        token.matrixServerName = parseHomeserverDomain(session_->getHomeServerUrl());
        token.expiresIn = 3600; // 1 hour

        {
            std::lock_guard<std::mutex> lock(mutex_);
            currentToken_ = token;
            tokenIssuedAt_ = currentTimeMs();
        }

        callback(true, "", token);
    }

    bool isTokenValid(const OpenIdToken& token) const override {
        if (token.accessToken.empty()) return false;

        std::lock_guard<std::mutex> lock(mutex_);
        // Check if token is the current one and hasn't expired
        if (token.accessToken == currentToken_.accessToken) {
            int64_t elapsed = currentTimeMs() - tokenIssuedAt_;
            return elapsed < (token.expiresIn * 1000);
        }

        // For external tokens, just check the access token is non-empty
        // and the token type is set
        return !token.tokenType.empty() && !token.matrixServerName.empty();
    }

    // Internal helpers
    void invalidateCurrentToken() {
        std::lock_guard<std::mutex> lock(mutex_);
        currentToken_ = OpenIdToken{};
        tokenIssuedAt_ = 0;
    }

private:
    static std::string generateTokenString() {
        static std::atomic<uint64_t> counter{0};
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "oid_" + std::to_string(now) + "_" + std::to_string(++counter);
    }

    static std::string parseHomeserverDomain(const std::string& url) {
        static const std::regex domainRegex(R"(https?://([^/:]+))");
        std::smatch match;
        if (std::regex_search(url, match, domainRegex)) {
            return match[1].str();
        }
        return "matrix.org";
    }

    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    OpenIdToken currentToken_;
    int64_t tokenIssuedAt_ = 0;
};

// ============================================================================
// PermalinkServiceImpl
// ============================================================================
class PermalinkServiceImpl : public PermalinkService {
public:
    explicit PermalinkServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    std::string createPermalink(const std::string& roomId, const std::string& eventId) const override {
        if (roomId.empty()) return "";

        std::string baseUrl = getPermalinkBaseUrl();
        std::string permalink = baseUrl + "/#/" + roomId;
        if (!eventId.empty()) {
            permalink += "/" + eventId;
        }
        return permalink;
    }

    std::string createRoomLink(const std::string& roomId,
                                const std::vector<std::string>& viaServers) const override {
        if (roomId.empty()) return "";

        std::string baseUrl = getPermalinkBaseUrl();
        std::string link = baseUrl + "/#/" + roomId;

        if (!viaServers.empty()) {
            link += "?";
            for (size_t i = 0; i < viaServers.size(); ++i) {
                if (i > 0) link += "&";
                link += "via=" + viaServers[i];
            }
        }
        return link;
    }

    std::string createUserLink(const std::string& userId) const override {
        if (userId.empty()) return "";

        std::string baseUrl = getPermalinkBaseUrl();
        return baseUrl + "/#/" + userId;
    }

    bool isPermalink(const std::string& url) const override {
        if (url.empty()) return false;

        std::string baseUrl = getPermalinkBaseUrl();
        // Check if URL starts with matrix.to or element permalink
        if (url.find("matrix.to/#/") != std::string::npos) return true;
        if (url.find("element://") == 0) return true;

        // Check for matrix: URI scheme
        if (url.find("matrix:") == 0) return true;

        // Check if it starts with our permalink base
        if (url.find(baseUrl + "/#/") == 0) return true;

        return false;
    }

    bool parsePermalink(const std::string& url, std::string& roomId, std::string& eventId) const override {
        if (!isPermalink(url)) return false;

        // Extract the part after /#/
        auto hashPos = url.find("/#/");
        if (hashPos == std::string::npos) {
            // Try matrix: URI scheme
            std::string identifier;
            if (url.find("matrix:roomid/") == 0) {
                identifier = url.substr(std::string("matrix:roomid/").length());
            } else if (url.find("matrix:u/") == 0) {
                identifier = url.substr(std::string("matrix:u/").length());
            } else if (url.find("matrix:r/") == 0) {
                identifier = url.substr(std::string("matrix:r/").length());
            } else {
                return false;
            }

            // Remove query params
            auto qPos = identifier.find('?');
            if (qPos != std::string::npos) {
                identifier = identifier.substr(0, qPos);
            }

            if (!identifier.empty() && identifier[0] == '!') {
                roomId = identifier;
            } else if (!identifier.empty() && identifier[0] == '@') {
                roomId = identifier; // Store user ID in roomId
            }
            return true;
        }

        std::string path = url.substr(hashPos + 3);

        // Remove query params
        auto qPos = path.find('?');
        if (qPos != std::string::npos) {
            path = path.substr(0, qPos);
        }

        // Split by '/'
        std::istringstream iss(path);
        std::string segment;
        std::vector<std::string> segments;
        while (std::getline(iss, segment, '/')) {
            if (!segment.empty()) segments.push_back(segment);
        }

        if (segments.empty()) return false;

        if (segments[0][0] == '@') {
            // User permalink
            roomId = segments[0];
            return true;
        }

        if (segments[0][0] == '!' || segments[0][0] == '#') {
            roomId = segments[0];
            if (segments.size() >= 2 && segments[1][0] == '$') {
                eventId = segments[1];
            }
            return true;
        }

        return false;
    }

    // Internal helpers
    void setPermalinkBaseUrl(const std::string& url) {
        permalinkBaseUrl_ = url;
    }

private:
    std::string getPermalinkBaseUrl() const {
        if (!permalinkBaseUrl_.empty()) {
            return permalinkBaseUrl_;
        }
        // Default to matrix.to
        return "https://matrix.to";
    }

    std::shared_ptr<Session> session_;
    std::string permalinkBaseUrl_;
};

// ============================================================================
// PresenceServiceImpl
// ============================================================================
class PresenceServiceImpl : public PresenceService {
public:
    explicit PresenceServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {
        // Initialize own presence
        ownPresence_["presence"] = "online";
        ownPresence_["status_msg"] = "";
        ownPresence_["last_active_ago"] = 0;
    }

    void setPresence(const std::string& presence, const std::string& statusMsg,
                     SimpleCallback callback) override {
        static const std::set<std::string> validPresences =
            {"online", "offline", "unavailable", "invisible"};

        if (validPresences.find(presence) == validPresences.end()) {
            callback(false, "Invalid presence state: " + presence +
                     ". Must be one of: online, offline, unavailable, invisible");
            return;
        }

        if (statusMsg.length() > 500) {
            callback(false, "Status message must be at most 500 characters");
            return;
        }

        json body;
        body["presence"] = presence;
        if (!statusMsg.empty()) {
            body["status_msg"] = statusMsg;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            ownPresence_["presence"] = presence;
            ownPresence_["status_msg"] = statusMsg;
            if (presence == "online") {
                ownPresence_["last_active_ago"] = 0;
                ownPresence_["currently_active"] = true;
            }
        }

        // PUT /_matrix/client/v3/presence/{userId}/status
        callback(true, "");
    }

    std::string getPresence(const std::string& userId) const override {
        if (userId.empty()) return "offline";

        std::lock_guard<std::mutex> lock(mutex_);
        if (userId == session_->getUserId()) {
            return ownPresence_["presence"];
        }

        auto it = userPresences_.find(userId);
        if (it != userPresences_.end()) {
            if (it->second.find("presence") != it->second.end()) {
                return it->second["presence"];
            }
        }
        return "offline";
    }

    std::string getStatusMessage(const std::string& userId) const override {
        if (userId.empty()) return "";

        std::lock_guard<std::mutex> lock(mutex_);
        if (userId == session_->getUserId()) {
            auto it = ownPresence_.find("status_msg");
            if (it != ownPresence_.end()) return it->second;
            return "";
        }

        auto it = userPresences_.find(userId);
        if (it != userPresences_.end()) {
            auto sit = it->second.find("status_msg");
            if (sit != it->second.end()) return sit->second;
        }
        return "";
    }

    bool isUserOnline(const std::string& userId) const override {
        std::string presence = getPresence(userId);
        return presence == "online" || presence == "unavailable";
    }

    int64_t getLastActiveAgo(const std::string& userId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (userId == session_->getUserId()) {
            auto it = ownPresence_.find("last_active_ago");
            if (it != ownPresence_.end()) {
                try { return std::stoll(it->second); }
                catch (...) { return 0; }
            }
            return 0;
        }

        auto it = userPresences_.find(userId);
        if (it != userPresences_.end()) {
            auto lait = it->second.find("last_active_ago");
            if (lait != it->second.end()) {
                try { return std::stoll(lait->second); }
                catch (...) { return -1; }
            }
        }
        return -1;
    }

    // Internal helpers - called from sync/events
    void updateUserPresence(const std::string& userId, const std::string& presence,
                            const std::string& statusMsg, int64_t lastActiveAgo) {
        if (userId.empty()) return;

        std::lock_guard<std::mutex> lock(mutex_);
        if (userId == session_->getUserId()) {
            ownPresence_["presence"] = presence;
            ownPresence_["status_msg"] = statusMsg;
            ownPresence_["last_active_ago"] = std::to_string(lastActiveAgo);
        } else {
            userPresences_[userId]["presence"] = presence;
            userPresences_[userId]["status_msg"] = statusMsg;
            userPresences_[userId]["last_active_ago"] = std::to_string(lastActiveAgo);
            userPresences_[userId]["updated_at"] = std::to_string(currentTimeMs());
        }
    }

private:
    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::map<std::string, std::string> ownPresence_;
    std::unordered_map<std::string, std::map<std::string, std::string>> userPresences_;
};

// ============================================================================
// ProfileServiceImpl
// ============================================================================
class ProfileServiceImpl : public ProfileService {
public:
    explicit ProfileServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)), ownProfileRefreshed_(false) {}

    void fetchProfile(const std::string& userId, ProfileCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required", UserProfile{});
            return;
        }

        // Check local cache first
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = profileCache_.find(userId);
            if (it != profileCache_.end()) {
                int64_t age = currentTimeMs() - it->second.lastUpdated;
                if (age < 300000) { // Cache valid for 5 minutes
                    callback(true, "", it->second);
                    return;
                }
            }
        }

        // GET /_matrix/client/v3/profile/{userId}
        // GET /_matrix/client/v3/profile/{userId}/displayname
        // GET /_matrix/client/v3/profile/{userId}/avatar_url
        // In a real impl, these are separate HTTP requests

        // Simulate profile fetch
        UserProfile profile;
        profile.userId = userId;
        profile.displayName = userId; // Default to userId if no display name
        profile.lastUpdated = currentTimeMs();

        // Parse userId to extract a display name guess
        if (userId.size() > 1 && userId[0] == '@') {
            auto colonPos = userId.find(':');
            if (colonPos != std::string::npos) {
                profile.displayName = userId.substr(1, colonPos - 1);
            }
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            profileCache_[userId] = profile;
        }

        callback(true, "", profile);
    }

    std::optional<UserProfile> getCachedProfile(const std::string& userId) const override {
        if (userId.empty()) return std::nullopt;

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = profileCache_.find(userId);
        if (it != profileCache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void setDisplayName(const std::string& displayName, SimpleCallback callback) override {
        if (displayName.empty()) {
            callback(false, "Display name cannot be empty");
            return;
        }
        if (displayName.length() > 256) {
            callback(false, "Display name must be at most 256 characters");
            return;
        }

        json body;
        body["displayname"] = displayName;

        // PUT /_matrix/client/v3/profile/{userId}/displayname
        std::string ownUserId = session_->getUserId();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = profileCache_.find(ownUserId);
            if (it != profileCache_.end()) {
                it->second.displayName = displayName;
                it->second.lastUpdated = currentTimeMs();
            } else {
                UserProfile profile;
                profile.userId = ownUserId;
                profile.displayName = displayName;
                profile.lastUpdated = currentTimeMs();
                profileCache_[ownUserId] = profile;
            }
        }

        callback(true, "");
    }

    void setAvatarUrl(const std::string& mxcUrl, SimpleCallback callback) override {
        if (mxcUrl.empty()) {
            callback(false, "Avatar MXC URL is required");
            return;
        }

        static const std::regex mxcRegex(R"(^mxc://[^/]+/[^/]+$)");
        if (!std::regex_match(mxcUrl, mxcRegex)) {
            callback(false, "Invalid MXC URL format");
            return;
        }

        json body;
        body["avatar_url"] = mxcUrl;

        // PUT /_matrix/client/v3/profile/{userId}/avatar_url
        std::string ownUserId = session_->getUserId();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = profileCache_.find(ownUserId);
            if (it != profileCache_.end()) {
                it->second.avatarUrl = mxcUrl;
                it->second.lastUpdated = currentTimeMs();
            } else {
                UserProfile profile;
                profile.userId = ownUserId;
                profile.avatarUrl = mxcUrl;
                profile.lastUpdated = currentTimeMs();
                profileCache_[ownUserId] = profile;
            }
        }

        callback(true, "");
    }

    void refreshProfile(SimpleCallback callback) override {
        // Re-fetch own profile from server
        std::string ownUserId = session_->getUserId();
        fetchProfile(ownUserId, [callback, this](bool success, const std::string& error, const UserProfile&) {
            if (success) {
                ownProfileRefreshed_ = true;
                callback(true, "");
            } else {
                callback(false, error);
            }
        });
    }

    // Internal helpers
    void updateProfileFromEvent(const std::string& userId, const UserProfile& profile) {
        std::lock_guard<std::mutex> lock(mutex_);
        profileCache_[userId] = profile;
        profileCache_[userId].lastUpdated = currentTimeMs();
    }

    void invalidateCache(const std::string& userId) {
        std::lock_guard<std::mutex> lock(mutex_);
        profileCache_.erase(userId);
    }

    void clearCache() {
        std::lock_guard<std::mutex> lock(mutex_);
        profileCache_.clear();
        ownProfileRefreshed_ = false;
    }

private:
    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, UserProfile> profileCache_; // userId -> profile
    bool ownProfileRefreshed_;
};

// ============================================================================
// Service Factory Functions
// ============================================================================

std::shared_ptr<HomeServerCapabilitiesService> createHomeServerCapabilitiesService(
    std::shared_ptr<Session> session) {
    return std::make_shared<HomeServerCapabilitiesServiceImpl>(session);
}

std::shared_ptr<IdentityService> createIdentityService(std::shared_ptr<Session> session) {
    return std::make_shared<IdentityServiceImpl>(session);
}

std::shared_ptr<IntegrationManagerService> createIntegrationManagerService(
    std::shared_ptr<Session> session) {
    return std::make_shared<IntegrationManagerServiceImpl>(session);
}

std::shared_ptr<MediaService> createMediaService(std::shared_ptr<Session> session) {
    return std::make_shared<MediaServiceImpl>(session);
}

std::shared_ptr<OpenIdService> createOpenIdService(std::shared_ptr<Session> session) {
    return std::make_shared<OpenIdServiceImpl>(session);
}

std::shared_ptr<PermalinkService> createPermalinkService(std::shared_ptr<Session> session) {
    return std::make_shared<PermalinkServiceImpl>(session);
}

std::shared_ptr<PresenceService> createPresenceService(std::shared_ptr<Session> session) {
    return std::make_shared<PresenceServiceImpl>(session);
}

std::shared_ptr<ProfileService> createProfileService(std::shared_ptr<Session> session) {
    return std::make_shared<ProfileServiceImpl>(session);
}

} // namespace matrix_sdk
