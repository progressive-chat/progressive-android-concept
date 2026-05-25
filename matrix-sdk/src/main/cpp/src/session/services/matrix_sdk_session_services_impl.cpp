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
#include <fstream>

namespace matrix_sdk {

// ============================================================================
// AccountServiceImpl
// ============================================================================
class AccountServiceImpl : public AccountService {
public:
    explicit AccountServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void changePassword(const std::string& oldPassword, const std::string& newPassword,
                        SimpleCallback callback) override {
        if (oldPassword.empty()) {
            callback(false, "Old password is required");
            return;
        }
        if (newPassword.empty()) {
            callback(false, "New password is required");
            return;
        }
        if (newPassword.length() < 8) {
            callback(false, "New password must be at least 8 characters");
            return;
        }
        if (oldPassword == newPassword) {
            callback(false, "New password must differ from old password");
            return;
        }

        json body;
        body["auth"] = {{"type", "m.login.password"},
                         {"identifier", {{"type", "m.id.user"},
                                         {"user", session_->getUserId()}}},
                         {"password", oldPassword}};
        body["new_password"] = newPassword;

        // POST /_matrix/client/v3/account/password
        callback(true, "");
    }

    void deactivateAccount(const std::string& authJson, SimpleCallback callback) override {
        if (authJson.empty()) {
            callback(false, "Authentication data is required");
            return;
        }

        json auth;
        try {
            auth = json::parse(authJson);
        } catch (const json::parse_error&) {
            callback(false, "Invalid authentication JSON");
            return;
        }

        if (!auth.contains("type")) {
            callback(false, "Authentication type is required");
            return;
        }

        // POST /_matrix/client/v3/account/deactivate
        // Body contains the auth block, optional erase flag
        callback(true, "");
    }

    void changeDisplayName(const std::string& displayName, SimpleCallback callback) override {
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
        {
            std::lock_guard<std::mutex> lock(mutex_);
            displayName_ = displayName;
        }

        // PUT /_matrix/client/v3/profile/{userId}/displayname
        callback(true, "");
    }

    void changeAvatar(const std::string& mxcUrl, SimpleCallback callback) override {
        if (mxcUrl.empty()) {
            callback(false, "Avatar MXC URL is required");
            return;
        }

        // Validate MXC URL format: mxc://server-name/media-id
        static const std::regex mxcRegex(R"(^mxc://[^/]+/[^/]+$)");
        if (!std::regex_match(mxcUrl, mxcRegex)) {
            callback(false, "Invalid MXC URL format");
            return;
        }

        json body;
        body["avatar_url"] = mxcUrl;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            avatarUrl_ = mxcUrl;
        }

        // PUT /_matrix/client/v3/profile/{userId}/avatar_url
        callback(true, "");
    }

    void getThreePids(std::function<void(bool, const std::string&, const std::vector<std::string>&)> callback) override {
        // GET /_matrix/client/v3/account/3pid
        // Returns list of associated threepids
        std::lock_guard<std::mutex> lock(mutex_);
        callback(true, "", threePids_);
    }

    void addThreePid(const std::string& address, const std::string& medium, SimpleCallback callback) override {
        if (address.empty()) {
            callback(false, "Address is required");
            return;
        }
        if (medium != "email" && medium != "msisdn") {
            callback(false, "Medium must be 'email' or 'msisdn'");
            return;
        }

        // POST /_matrix/client/v3/account/3pid/email/requestToken or /msisdn/requestToken
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pendingPidRequests_[address] = medium;
        }
        callback(true, "");
    }

    void deleteThreePid(const std::string& address, const std::string& medium, SimpleCallback callback) override {
        if (address.empty()) {
            callback(false, "Address is required");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = std::find(threePids_.begin(), threePids_.end(),
                                medium + ":" + address);
            if (it != threePids_.end()) {
                threePids_.erase(it);
            } else {
                callback(false, "Threepid not found");
                return;
            }
        }

        // POST /_matrix/client/v3/account/3pid/delete
        callback(true, "");
    }

    void requestTokenEmail(const std::string& email, const std::string& clientSecret,
                           int sendAttempt, SimpleCallback callback) override {
        if (email.empty()) {
            callback(false, "Email address is required");
            return;
        }
        if (clientSecret.empty()) {
            callback(false, "Client secret is required");
            return;
        }
        if (sendAttempt <= 0) {
            callback(false, "Send attempt must be positive");
            return;
        }

        // POST /_matrix/client/v3/register/email/requestToken
        json body;
        body["client_secret"] = clientSecret;
        body["email"] = email;
        body["send_attempt"] = sendAttempt;
        callback(true, "");
    }

    void requestTokenSms(const std::string& phone, const std::string& clientSecret,
                         int sendAttempt, SimpleCallback callback) override {
        if (phone.empty()) {
            callback(false, "Phone number is required");
            return;
        }
        if (clientSecret.empty()) {
            callback(false, "Client secret is required");
            return;
        }
        if (sendAttempt <= 0) {
            callback(false, "Send attempt must be positive");
            return;
        }

        // POST /_matrix/client/v3/register/msisdn/requestToken
        json body;
        body["client_secret"] = clientSecret;
        body["phone_number"] = phone;
        body["send_attempt"] = sendAttempt;
        callback(true, "");
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::string displayName_;
    std::string avatarUrl_;
    std::vector<std::string> threePids_;
    std::unordered_map<std::string, std::string> pendingPidRequests_;
};

// ============================================================================
// SessionAccountDataServiceImpl
// ============================================================================
class SessionAccountDataServiceImpl : public SessionAccountDataService {
public:
    explicit SessionAccountDataServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void setAccountData(const std::string& type, const std::string& content,
                        SimpleCallback callback) override {
        if (type.empty()) {
            callback(false, "Account data type is required");
            return;
        }
        if (content.empty()) {
            callback(false, "Account data content is required");
            return;
        }

        // Validate content is valid JSON
        try {
            json::parse(content);
        } catch (const json::parse_error&) {
            callback(false, "Account data content must be valid JSON");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            accountData_[type] = content;
            accountDataTimestamps_[type] = currentTimeMs();
        }

        // PUT /_matrix/client/v3/user/{userId}/account_data/{type}
        callback(true, "");
    }

    std::string getAccountData(const std::string& type) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = accountData_.find(type);
        if (it != accountData_.end()) {
            return it->second;
        }
        return "";
    }

    std::map<std::string, std::string> getAllAccountData() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return accountData_;
    }

    void updateDirectRooms(const std::map<std::string, std::vector<std::string>>& directRooms,
                           SimpleCallback callback) override {
        if (directRooms.empty()) {
            callback(false, "Direct rooms map cannot be empty");
            return;
        }

        json content;
        for (const auto& [userId, roomIds] : directRooms) {
            if (userId.empty()) {
                callback(false, "User ID cannot be empty");
                return;
            }
            content[userId] = roomIds;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            directRooms_ = directRooms;
        }

        // PUT /_matrix/client/v3/user/{userId}/account_data/m.direct
        callback(true, "");
    }

    std::map<std::string, std::vector<std::string>> getDirectRooms() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return directRooms_;
    }

    std::string getGlobalAccountData(const std::string& type) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = globalAccountData_.find(type);
        if (it != globalAccountData_.end()) {
            return it->second;
        }
        return "";
    }

    void setGlobalAccountData(const std::string& type, const std::string& content,
                              SimpleCallback callback) override {
        if (type.empty()) {
            callback(false, "Global account data type is required");
            return;
        }

        try {
            json::parse(content);
        } catch (const json::parse_error&) {
            callback(false, "Global account data content must be valid JSON");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            globalAccountData_[type] = content;
        }

        // PUT /_matrix/client/v3/user/{userId}/account_data/{type}
        // Note: global account data uses the same endpoint but different scope
        callback(true, "");
    }

private:
    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::map<std::string, std::string> accountData_;
    std::map<std::string, int64_t> accountDataTimestamps_;
    std::map<std::string, std::vector<std::string>> directRooms_;
    std::map<std::string, std::string> globalAccountData_;
};

// ============================================================================
// CallSignalingServiceImpl
// ============================================================================
class CallSignalingServiceImpl : public CallSignalingService {
public:
    explicit CallSignalingServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void placeCall(const std::string& roomId, const std::string& callId,
                   SimpleCallback callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required");
            return;
        }
        if (callId.empty()) {
            callback(false, "Call ID is required");
            return;
        }

        json content;
        content["call_id"] = callId;
        content["offer"] = "";
    // SDP offer would be set by the calling application
    content["version"] = "1";
    content["lifetime"] = 60000; // 60 seconds default

    std::string eventType = "m.call.invite";

    {
        std::lock_guard<std::mutex> lock(mutex_);
        activeCalls_[callId] = {
            {"roomId", roomId},
            {"state", "invite"},
            {"timestamp", currentTimeMs()},
            {"callerId", session_->getUserId()}
        };
    }

    // PUT /_matrix/client/v3/rooms/{roomId}/send/{eventType}/{txnId}
    callback(true, "");
}

void answerCall(const std::string& roomId, const std::string& callId,
                SimpleCallback callback) override {
    if (roomId.empty()) {
        callback(false, "Room ID is required");
        return;
    }
    if (callId.empty()) {
        callback(false, "Call ID is required");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = activeCalls_.find(callId);
    if (it == activeCalls_.end()) {
        callback(false, "Call not found");
        return;
    }
    if (it->second["state"] != "invite") {
        callback(false, "Call is not in invitable state");
        return;
    }

    it->second["state"] = "connected";
    it->second["answererId"] = session_->getUserId();

    // PUT /_matrix/client/v3/rooms/{roomId}/send/m.call.answer/{txnId}
    callback(true, "");
}

void hangupCall(const std::string& roomId, const std::string& callId,
                SimpleCallback callback) override {
    if (roomId.empty()) {
        callback(false, "Room ID is required");
        return;
    }
    if (callId.empty()) {
        callback(false, "Call ID is required");
        return;
    }

    json content;
    content["call_id"] = callId;
    content["version"] = "1";

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = activeCalls_.find(callId);
        if (it != activeCalls_.end()) {
            it->second["state"] = "ended";
            it->second["reason"] = "user_hangup";
        }
    }

    // PUT /_matrix/client/v3/rooms/{roomId}/send/m.call.hangup/{txnId}
    callback(true, "");
}

void sendCandidates(const std::string& roomId, const std::string& callId,
                    const std::string& candidates, SimpleCallback callback) override {
    if (roomId.empty()) {
        callback(false, "Room ID is required");
        return;
    }
    if (callId.empty()) {
        callback(false, "Call ID is required");
        return;
    }
    if (candidates.empty()) {
        callback(false, "Candidates payload is required");
        return;
    }

    // Validate candidates is valid JSON
    try {
        json::parse(candidates);
    } catch (const json::parse_error&) {
        callback(false, "Candidates must be valid JSON");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (activeCalls_.find(callId) == activeCalls_.end()) {
            callback(false, "Call not found");
            return;
        }
    }

    // PUT /_matrix/client/v3/rooms/{roomId}/send/m.call.candidates/{txnId}
    callback(true, "");
}

private:
    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::map<std::string, std::string>> activeCalls_;
};

// ============================================================================
// ContentUploadStateTrackerImpl
// ============================================================================
class ContentUploadStateTrackerImpl : public ContentUploadStateTracker {
public:
    explicit ContentUploadStateTrackerImpl() {}

    void trackUpload(const std::string& uploadId, ProgressCallback progress) override {
        if (uploadId.empty()) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (uploads_.find(uploadId) != uploads_.end()) {
            return; // Already tracking this upload
        }

        UploadInfo info;
        info.callback = progress;
        info.progress = 0;
        info.total = 0;
        info.status = "pending";
        info.startedAt = currentTimeMs();
        uploads_[uploadId] = info;
    }

    void untrackUpload(const std::string& uploadId) override {
        if (uploadId.empty()) return;

        std::lock_guard<std::mutex> lock(mutex_);
        uploads_.erase(uploadId);
    }

    int getProgress(const std::string& uploadId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = uploads_.find(uploadId);
        if (it != uploads_.end()) {
            if (it->second.total > 0) {
                return (it->second.progress * 100) / it->second.total;
            }
            return it->second.progress;
        }
        return -1;
    }

    int getActiveUploads() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        int count = 0;
        for (const auto& [id, info] : uploads_) {
            if (info.status == "pending" || info.status == "uploading") {
                count++;
            }
        }
        return count;
    }

    // Internal helpers
    void updateProgress(const std::string& uploadId, int progress, int total) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = uploads_.find(uploadId);
        if (it != uploads_.end()) {
            it->second.progress = progress;
            it->second.total = total;
            it->second.status = progress >= total ? "completed" : "uploading";
            if (it->second.callback) {
                it->second.callback(progress, total);
            }
        }
    }

    void markCompleted(const std::string& uploadId, const std::string& mxcUrl) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = uploads_.find(uploadId);
        if (it != uploads_.end()) {
            it->second.status = "completed";
            it->second.mxcUrl = mxcUrl;
            it->second.completedAt = currentTimeMs();
        }
    }

    void markFailed(const std::string& uploadId, const std::string& error) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = uploads_.find(uploadId);
        if (it != uploads_.end()) {
            it->second.status = "failed";
            it->second.error = error;
        }
    }

    std::string getMxcUrl(const std::string& uploadId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = uploads_.find(uploadId);
        if (it != uploads_.end()) {
            return it->second.mxcUrl;
        }
        return "";
    }

private:
    struct UploadInfo {
        ProgressCallback callback;
        int progress = 0;
        int total = 0;
        std::string status;
        std::string mxcUrl;
        std::string error;
        int64_t startedAt = 0;
        int64_t completedAt = 0;
    };

    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    mutable std::mutex mutex_;
    std::unordered_map<std::string, UploadInfo> uploads_;
};

// ============================================================================
// ContentUrlResolverImpl
// ============================================================================
class ContentUrlResolverImpl : public ContentUrlResolver {
public:
    explicit ContentUrlResolverImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    std::string resolveUrl(const std::string& mxcUrl) const override {
        if (mxcUrl.empty()) return "";
        auto parsed = parseMxc(mxcUrl);
        if (parsed.first.empty() || parsed.second.empty()) return "";

        std::string baseUrl = getHomeServerContentUrl();
        return baseUrl + "/_matrix/media/v3/download/" +
               parsed.first + "/" + parsed.second;
    }

    std::string resolveThumbnailUrl(const std::string& mxcUrl,
                                     const ThumbnailSize& size) const override {
        if (mxcUrl.empty()) return "";
        auto parsed = parseMxc(mxcUrl);
        if (parsed.first.empty() || parsed.second.empty()) return "";

        std::string baseUrl = getHomeServerContentUrl();
        std::string url = baseUrl + "/_matrix/media/v3/thumbnail/" +
                          parsed.first + "/" + parsed.second;
        url += "?width=" + std::to_string(size.width) +
               "&height=" + std::to_string(size.height);
        if (!size.method.empty()) {
            url += "&method=" + size.method;
        }
        return url;
    }

    std::string resolveDownloadUrl(const std::string& mxcUrl) const override {
        return resolveUrl(mxcUrl);
    }

    std::string resolveUploadUrl() const override {
        std::string baseUrl = getHomeServerContentUrl();
        return baseUrl + "/_matrix/media/v3/upload";
    }

    std::string getHomeServerContentUrl() const override {
        std::string homeserverUrl = session_->getHomeServerUrl();
        // Strip trailing slash
        while (!homeserverUrl.empty() && homeserverUrl.back() == '/') {
            homeserverUrl.pop_back();
        }
        return homeserverUrl;
    }

private:
    // Returns {server_name, media_id}
    static std::pair<std::string, std::string> parseMxc(const std::string& mxcUrl) {
        static const std::regex mxcRegex(R"(^mxc://([^/]+)/(.+)$)");
        std::smatch match;
        if (std::regex_match(mxcUrl, match, mxcRegex)) {
            return {match[1].str(), match[2].str()};
        }
        return {"", ""};
    }

    std::shared_ptr<Session> session_;
};

// ============================================================================
// ContentScannerServiceImpl
// ============================================================================
class ContentScannerServiceImpl : public ContentScannerService {
public:
    explicit ContentScannerServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)), scannerSupported_(false) {}

    void scanContent(const std::string& mxcUrl, SimpleCallback callback) override {
        if (mxcUrl.empty()) {
            callback(false, "MXC URL is required");
            return;
        }

        if (!isContentScannerSupported()) {
            callback(false, "Content scanner is not supported by this homeserver");
            return;
        }

        // Validate MXC format
        static const std::regex mxcRegex(R"(^mxc://[^/]+/[^/]+$)");
        if (!std::regex_match(mxcUrl, mxcRegex)) {
            callback(false, "Invalid MXC URL format");
            return;
        }

        std::string scanId = generateScanId();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            ScanInfo info;
            info.mxcUrl = mxcUrl;
            info.status = "pending";
            info.startedAt = currentTimeMs();
            scans_[scanId] = info;
        }

        // POST /_matrix/client/v3/content_scan/{serverName}/{mediaId}
        callback(true, scanId);
    }

    void getScanResult(const std::string& scanId,
                       std::function<void(bool, const std::string&, const std::string&)> callback) override {
        if (scanId.empty()) {
            callback(false, "Scan ID is required", "");
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = scans_.find(scanId);
        if (it == scans_.end()) {
            callback(false, "Scan not found", "");
            return;
        }

        if (it->second.status == "pending") {
            callback(false, "Scan is still in progress", "pending");
        } else if (it->second.status == "completed") {
            callback(true, it->second.result, it->second.status);
        } else if (it->second.status == "failed") {
            callback(false, it->second.error, "failed");
        } else {
            callback(false, "Unknown scan status: " + it->second.status, "");
        }
    }

    bool isContentScannerSupported() const override {
        return scannerSupported_;
    }

    // Internal helpers
    void setScannerSupported(bool supported) {
        scannerSupported_ = supported;
    }

    void updateScanResult(const std::string& scanId, const std::string& result) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = scans_.find(scanId);
        if (it != scans_.end()) {
            it->second.status = "completed";
            it->second.result = result;
            it->second.completedAt = currentTimeMs();
        }
    }

private:
    struct ScanInfo {
        std::string mxcUrl;
        std::string status;
        std::string result;
        std::string error;
        int64_t startedAt = 0;
        int64_t completedAt = 0;
    };

    static std::string generateScanId() {
        static std::atomic<uint64_t> counter{0};
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "scan_" + std::to_string(now) + "_" + std::to_string(++counter);
    }

    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::atomic<bool> scannerSupported_;
    std::unordered_map<std::string, ScanInfo> scans_;
};

// ============================================================================
// ContentDownloadStateTrackerImpl
// ============================================================================
class ContentDownloadStateTrackerImpl : public ContentDownloadStateTracker {
public:
    explicit ContentDownloadStateTrackerImpl() {}

    void trackDownload(const std::string& downloadId, ProgressCallback progress) override {
        if (downloadId.empty()) return;

        std::lock_guard<std::mutex> lock(mutex_);
        if (downloads_.find(downloadId) != downloads_.end()) {
            return; // Already tracking
        }

        DownloadInfo info;
        info.callback = progress;
        info.progress = 0;
        info.total = 0;
        info.status = "pending";
        info.startedAt = currentTimeMs();
        downloads_[downloadId] = info;
    }

    void untrackDownload(const std::string& downloadId) override {
        if (downloadId.empty()) return;

        std::lock_guard<std::mutex> lock(mutex_);
        downloads_.erase(downloadId);
    }

    int getProgress(const std::string& downloadId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = downloads_.find(downloadId);
        if (it != downloads_.end()) {
            if (it->second.total > 0) {
                return (it->second.progress * 100) / it->second.total;
            }
            return it->second.progress;
        }
        return -1;
    }

    // Internal helpers
    void updateProgress(const std::string& downloadId, int progress, int total) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = downloads_.find(downloadId);
        if (it != downloads_.end()) {
            it->second.progress = progress;
            it->second.total = total;
            it->second.status = (total > 0 && progress >= total) ? "completed" : "downloading";
            if (it->second.callback) {
                it->second.callback(progress, total);
            }
        }
    }

    void markCompleted(const std::string& downloadId, const std::string& localPath) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = downloads_.find(downloadId);
        if (it != downloads_.end()) {
            it->second.status = "completed";
            it->second.localPath = localPath;
            it->second.completedAt = currentTimeMs();
        }
    }

    void markFailed(const std::string& downloadId, const std::string& error) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = downloads_.find(downloadId);
        if (it != downloads_.end()) {
            it->second.status = "failed";
            it->second.error = error;
        }
    }

    void markCancelled(const std::string& downloadId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = downloads_.find(downloadId);
        if (it != downloads_.end()) {
            it->second.status = "cancelled";
        }
    }

    std::string getStatus(const std::string& downloadId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = downloads_.find(downloadId);
        if (it != downloads_.end()) {
            return it->second.status;
        }
        return "";
    }

    bool isActive(const std::string& downloadId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = downloads_.find(downloadId);
        if (it != downloads_.end()) {
            return it->second.status == "pending" ||
                   it->second.status == "downloading";
        }
        return false;
    }

private:
    struct DownloadInfo {
        ProgressCallback callback;
        int progress = 0;
        int total = 0;
        std::string status;
        std::string localPath;
        std::string error;
        int64_t startedAt = 0;
        int64_t completedAt = 0;
    };

    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    mutable std::mutex mutex_;
    std::unordered_map<std::string, DownloadInfo> downloads_;
};

// ============================================================================
// FileServiceImpl
// ============================================================================
class FileServiceImpl : public FileService {
public:
    explicit FileServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void downloadFile(const std::string& mxcUrl, const std::string& filename,
                      const std::string& mimeType, ProgressCallback progress,
                      SimpleCallback callback) override {
        if (mxcUrl.empty()) {
            callback(false, "MXC URL is required");
            return;
        }
        if (filename.empty()) {
            callback(false, "Filename is required");
            return;
        }

        // Validate MXC URL format
        static const std::regex mxcRegex(R"(^mxc://[^/]+/[^/]+$)");
        if (!std::regex_match(mxcUrl, mxcRegex)) {
            callback(false, "Invalid MXC URL format");
            return;
        }

        // Validate filename - prevent path traversal
        if (filename.find('/') != std::string::npos ||
            filename.find("..") != std::string::npos) {
            callback(false, "Invalid filename: path traversal characters detected");
            return;
        }

        std::string downloadId = generateDownloadId();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (downloadedFiles_.find(mxcUrl) != downloadedFiles_.end()) {
                // File already downloaded
                callback(true, downloadedFiles_[mxcUrl]);
                return;
            }

            FileDownloadInfo info;
            info.mxcUrl = mxcUrl;
            info.filename = filename;
            info.mimeType = mimeType;
            info.status = "pending";
            info.startedAt = currentTimeMs();
            activeDownloads_[downloadId] = info;
        }

        if (progress) {
            // Register progress tracking
            progress(0, 100);
        }

        // GET /_matrix/media/v3/download/{serverName}/{mediaId}
        // In a real implementation, this would stream the file to local storage

        // Simulate completed download
        std::string localPath = buildLocalPath(mxcUrl, filename);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            activeDownloads_.erase(downloadId);
            downloadedFiles_[mxcUrl] = localPath;

            auto& finfo = fileInfo_[mxcUrl];
            finfo.mimeType = mimeType;
            finfo.filename = filename;
            finfo.downloadedAt = currentTimeMs();
        }

        if (progress) {
            progress(100, 100);
        }

        callback(true, localPath);
    }

    void cancelDownload(const std::string& downloadId) override {
        if (downloadId.empty()) return;

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = activeDownloads_.find(downloadId);
        if (it != activeDownloads_.end()) {
            it->second.status = "cancelled";
            activeDownloads_.erase(it);
        }
    }

    std::string getLocalFilePath(const std::string& mxcUrl) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = downloadedFiles_.find(mxcUrl);
        if (it != downloadedFiles_.end()) {
            return it->second;
        }
        return "";
    }

    bool isFileDownloaded(const std::string& mxcUrl) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return downloadedFiles_.find(mxcUrl) != downloadedFiles_.end();
    }

    int64_t getFileSize(const std::string& mxcUrl) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = fileInfo_.find(mxcUrl);
        if (it != fileInfo_.end()) {
            return it->second.fileSize;
        }
        return -1;
    }

    // Internal helpers
    void setCacheDir(const std::string& dir) {
        cacheDir_ = dir;
    }

    void clearCache() {
        std::lock_guard<std::mutex> lock(mutex_);
        downloadedFiles_.clear();
        fileInfo_.clear();
        activeDownloads_.clear();
    }

private:
    struct FileDownloadInfo {
        std::string mxcUrl;
        std::string filename;
        std::string mimeType;
        std::string status;
        int64_t startedAt = 0;
    };

    struct FileCacheInfo {
        std::string mimeType;
        std::string filename;
        int64_t fileSize = 0;
        int64_t downloadedAt = 0;
    };

    static std::string generateDownloadId() {
        static std::atomic<uint64_t> counter{0};
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "dl_" + std::to_string(now) + "_" + std::to_string(++counter);
    }

    std::string buildLocalPath(const std::string& mxcUrl, const std::string& filename) const {
        // Build a cache path: cacheDir/hash(mxcUrl)/filename
        std::size_t hash = std::hash<std::string>{}(mxcUrl);
        std::string dir = cacheDir_.empty() ? "/tmp/matrix-sdk-cache" : cacheDir_;
        std::string subdir = dir + "/" + std::to_string(hash);
        return subdir + "/" + filename;
    }

    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::string cacheDir_;
    std::unordered_map<std::string, std::string> downloadedFiles_;           // mxc -> localPath
    std::unordered_map<std::string, FileCacheInfo> fileInfo_;               // mxc -> info
    std::unordered_map<std::string, FileDownloadInfo> activeDownloads_;     // downloadId -> info
};

// ============================================================================
// Service Factory Functions
// ============================================================================

std::shared_ptr<AccountService> createAccountService(std::shared_ptr<Session> session) {
    return std::make_shared<AccountServiceImpl>(session);
}

std::shared_ptr<SessionAccountDataService> createSessionAccountDataService(std::shared_ptr<Session> session) {
    return std::make_shared<SessionAccountDataServiceImpl>(session);
}

std::shared_ptr<CallSignalingService> createCallSignalingService(std::shared_ptr<Session> session) {
    return std::make_shared<CallSignalingServiceImpl>(session);
}

std::shared_ptr<ContentUploadStateTracker> createContentUploadStateTracker() {
    return std::make_shared<ContentUploadStateTrackerImpl>();
}

std::shared_ptr<ContentUrlResolver> createContentUrlResolver(std::shared_ptr<Session> session) {
    return std::make_shared<ContentUrlResolverImpl>(session);
}

std::shared_ptr<ContentScannerService> createContentScannerService(std::shared_ptr<Session> session) {
    return std::make_shared<ContentScannerServiceImpl>(session);
}

std::shared_ptr<ContentDownloadStateTracker> createContentDownloadStateTracker() {
    return std::make_shared<ContentDownloadStateTrackerImpl>();
}

std::shared_ptr<FileService> createFileService(std::shared_ptr<Session> session) {
    return std::make_shared<FileServiceImpl>(session);
}

} // namespace matrix_sdk
