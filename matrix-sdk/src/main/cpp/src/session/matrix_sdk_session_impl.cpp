#include "matrix_sdk/session/matrix_sdk_session.hpp"
#include "matrix_sdk/session/services/matrix_sdk_session_services.hpp"
#include "matrix_sdk/matrix_sdk_types.hpp"
#include "matrix_sdk/network/matrix_sdk_network.hpp"
#include "matrix_sdk/crypto/matrix_sdk_crypto.hpp"
#include "matrix_sdk/sync/matrix_sdk_sync_service.hpp"
#include <nlohmann/json.hpp>
#include <android/log.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <sstream>
#include <vector>

#define TAG "MatrixSession"

namespace matrix_sdk {

using json = nlohmann::json;

// ── SessionImpl ──
class SessionImpl : public Session,
                    public std::enable_shared_from_this<SessionImpl> {
public:
    explicit SessionImpl(const SessionParams& params)
        : params_(params)
        , alive_(false)
        , openable_(false)
        , connectionState_(ConnectionState::DISCONNECTED)
        , syncState_(SyncState::IDLE)
        , currentFilter_() {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Session created for user: %s",
            params.userId.c_str());
        validateParams();
    }

    ~SessionImpl() override {
        close();
    }

    // ── Identity ──
    std::string getUserId() const override {
        return params_.userId;
    }

    std::string getDeviceId() const override {
        return params_.deviceId;
    }

    std::string getAccessToken() const override {
        return params_.accessToken;
    }

    std::string getHomeServerUrl() const override {
        return params_.homeServerUrl;
    }

    SessionParams getSessionParams() const override {
        return params_;
    }

    // ── State ──
    bool isOpenable() const override {
        return openable_;
    }

    SessionState getState() const override {
        SessionState state;
        state.userId = params_.userId;
        state.deviceId = params_.deviceId;
        state.syncState = syncState_.load();
        state.connectionState = connectionState_.load();
        state.isOpenable = openable_;
        state.isEncryptionEnabled = cryptoService_ != nullptr;
        state.homeserverUrl = params_.homeServerUrl;
        state.identityServerUrl = params_.identityServerUrl;
        if (syncService_) {
            state.pendingRoomCount = syncService_->getPendingRoomCount();
        }
        return state;
    }

    SyncState getSyncState() const override {
        return syncState_.load();
    }

    // ── Lifecycle ──
    void open() override {
        if (alive_) {
            __android_log_print(ANDROID_LOG_WARN, TAG, "Session already open");
            return;
        }

        __android_log_print(ANDROID_LOG_INFO, TAG, "Opening session for %s", params_.userId.c_str());
        connectionState_ = ConnectionState::CONNECTING;
        notifyConnectionStateChanged(ConnectionState::CONNECTING);

        if (!openable_) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Session is not openable");
            connectionState_ = ConnectionState::DISCONNECTED;
            notifyConnectionStateChanged(ConnectionState::DISCONNECTED);
            MatrixError err;
            err.errorMessage = "Session is not openable - missing required parameters";
            notifyGlobalError(err);
            return;
        }

        alive_ = true;

        // Initialize and start sync service
        if (syncService_) {
            syncService_->start();
            syncState_ = syncService_->getState();

            // Start initial sync
            auto self = shared_from_this();
            syncService_->initialSync([this, self](bool success, const std::string& error) {
                if (success) {
                    connectionState_ = ConnectionState::CONNECTED;
                    notifyConnectionStateChanged(ConnectionState::CONNECTED);
                    syncState_ = SyncState::IDLE;
                    __android_log_print(ANDROID_LOG_INFO, TAG, "Initial sync completed successfully");
                } else {
                    __android_log_print(ANDROID_LOG_ERROR, TAG, "Initial sync failed: %s", error.c_str());
                    connectionState_ = ConnectionState::RECONNECTING;
                    notifyConnectionStateChanged(ConnectionState::RECONNECTING);

                    // Check for token errors
                    if (error.find("M_UNKNOWN_TOKEN") != std::string::npos ||
                        error.find("M_MISSING_TOKEN") != std::string::npos) {
                        notifyTokenInvalid();
                    }
                }
                notifySessionStateChanged(getState());
            });
        }

        notifySessionStateChanged(getState());
    }

    void close() override {
        if (!alive_) return;

        __android_log_print(ANDROID_LOG_INFO, TAG, "Closing session for %s", params_.userId.c_str());

        alive_ = false;
        connectionState_ = ConnectionState::DISCONNECTED;
        syncState_ = SyncState::TERMINATED;

        // Stop sync service
        if (syncService_) {
            syncService_->stop();
        }

        // Close crypto service
        if (cryptoService_) {
            cryptoService_->close();
        }

        notifyConnectionStateChanged(ConnectionState::DISCONNECTED);
        notifySessionStateChanged(getState());
    }

    void requireBackgroundSync() override {
        if (syncService_) {
            syncService_->enableBackgroundSync(true);
            __android_log_print(ANDROID_LOG_INFO, TAG, "Background sync enabled");
        }
    }

    void pauseSync() override {
        if (syncService_) {
            syncService_->pause();
            syncState_ = SyncState::PAUSED;
            notifySyncStateChanged(SyncState::PAUSED);
        }
    }

    void resumeSync() override {
        if (syncService_) {
            syncService_->resume();
            syncState_ = syncService_->getState();
            notifySyncStateChanged(syncState_);
        }
    }

    bool isAlive() const override {
        return alive_;
    }

    // ── Service Accessors ──
    std::shared_ptr<CryptoService> cryptoService() override {
        return cryptoService_;
    }

    std::shared_ptr<SessionAccountDataService> accountDataService() override {
        return accountDataService_;
    }

    std::shared_ptr<AccountService> accountService() override {
        return accountService_;
    }

    std::shared_ptr<SyncService> syncService() override {
        return syncService_;
    }

    std::shared_ptr<RoomService> roomService() override {
        return roomService_;
    }

    std::shared_ptr<UserService> userService() override {
        return userService_;
    }

    std::shared_ptr<EventService> eventService() override {
        return eventService_;
    }

    std::shared_ptr<PushRuleService> pushRuleService() override {
        return pushRuleService_;
    }

    std::shared_ptr<ProfileService> profileService() override {
        return profileService_;
    }

    std::shared_ptr<PresenceService> presenceService() override {
        return presenceService_;
    }

    std::shared_ptr<MediaService> mediaService() override {
        return mediaService_;
    }

    std::shared_ptr<FileService> fileService() override {
        return fileService_;
    }

    std::shared_ptr<ContentScannerService> contentScannerService() override {
        return contentScannerService_;
    }

    std::shared_ptr<ContentUrlResolver> contentUrlResolver() override {
        return contentUrlResolver_;
    }

    std::shared_ptr<ContentUploadStateTracker> contentUploadStateTracker() override {
        return contentUploadStateTracker_;
    }

    std::shared_ptr<ContentDownloadStateTracker> contentDownloadStateTracker() override {
        return contentDownloadStateTracker_;
    }

    std::shared_ptr<HomeServerCapabilitiesService> homeServerCapabilitiesService() override {
        return homeServerCapabilitiesService_;
    }

    std::shared_ptr<IdentityService> identityService() override {
        return identityService_;
    }

    std::shared_ptr<IntegrationManagerService> integrationManagerService() override {
        return integrationManagerService_;
    }

    std::shared_ptr<OpenIdService> openIdService() override {
        return openIdService_;
    }

    std::shared_ptr<PermalinkService> permalinkService() override {
        return permalinkService_;
    }

    std::shared_ptr<PushersService> pushersService() override {
        return pushersService_;
    }

    std::shared_ptr<SearchService> searchService() override {
        return searchService_;
    }

    std::shared_ptr<SharedSecretStorageService> sharedSecretStorageService() override {
        return sharedSecretStorageService_;
    }

    std::shared_ptr<SignOutService> signOutService() override {
        return signOutService_;
    }

    std::shared_ptr<SpaceService> spaceService() override {
        return spaceService_;
    }

    std::shared_ptr<TermsService> termsService() override {
        return termsService_;
    }

    std::shared_ptr<ThirdPartyService> thirdPartyService() override {
        return thirdPartyService_;
    }

    std::shared_ptr<TypingUsersTracker> typingUsersTracker() override {
        return typingUsersTracker_;
    }

    std::shared_ptr<WidgetService> widgetService() override {
        return widgetService_;
    }

    std::shared_ptr<CallSignalingService> callSignalingService() override {
        return callSignalingService_;
    }

    // ── Service Setters (for factory/configuration) ──
    void setCryptoService(std::shared_ptr<CryptoService> svc) {
        cryptoService_ = std::move(svc);
    }

    void setAccountDataService(std::shared_ptr<SessionAccountDataService> svc) {
        accountDataService_ = std::move(svc);
    }

    void setAccountService(std::shared_ptr<AccountService> svc) {
        accountService_ = std::move(svc);
    }

    void setSyncService(std::shared_ptr<SyncService> svc) {
        syncService_ = std::move(svc);
        // Wire sync callbacks
        if (syncService_) {
            auto self = shared_from_this();
            syncService_->setSyncCallback([this, self](const SyncResponse& response) {
                onSyncResponse(response);
            });
            syncService_->setErrorCallback([this, self](const MatrixError& error) {
                onSyncError(error);
            });
        }
    }

    void setRoomService(std::shared_ptr<RoomService> svc) {
        roomService_ = std::move(svc);
    }

    void setUserService(std::shared_ptr<UserService> svc) {
        userService_ = std::move(svc);
    }

    void setEventService(std::shared_ptr<EventService> svc) {
        eventService_ = std::move(svc);
    }

    void setPushRuleService(std::shared_ptr<PushRuleService> svc) {
        pushRuleService_ = std::move(svc);
    }

    void setProfileService(std::shared_ptr<ProfileService> svc) {
        profileService_ = std::move(svc);
    }

    void setPresenceService(std::shared_ptr<PresenceService> svc) {
        presenceService_ = std::move(svc);
    }

    void setMediaService(std::shared_ptr<MediaService> svc) {
        mediaService_ = std::move(svc);
    }

    void setFileService(std::shared_ptr<FileService> svc) {
        fileService_ = std::move(svc);
    }

    void setContentScannerService(std::shared_ptr<ContentScannerService> svc) {
        contentScannerService_ = std::move(svc);
    }

    void setContentUrlResolver(std::shared_ptr<ContentUrlResolver> svc) {
        contentUrlResolver_ = std::move(svc);
    }

    void setContentUploadStateTracker(std::shared_ptr<ContentUploadStateTracker> svc) {
        contentUploadStateTracker_ = std::move(svc);
    }

    void setContentDownloadStateTracker(std::shared_ptr<ContentDownloadStateTracker> svc) {
        contentDownloadStateTracker_ = std::move(svc);
    }

    void setHomeServerCapabilitiesService(std::shared_ptr<HomeServerCapabilitiesService> svc) {
        homeServerCapabilitiesService_ = std::move(svc);
    }

    void setIdentityService(std::shared_ptr<IdentityService> svc) {
        identityService_ = std::move(svc);
    }

    void setIntegrationManagerService(std::shared_ptr<IntegrationManagerService> svc) {
        integrationManagerService_ = std::move(svc);
    }

    void setOpenIdService(std::shared_ptr<OpenIdService> svc) {
        openIdService_ = std::move(svc);
    }

    void setPermalinkService(std::shared_ptr<PermalinkService> svc) {
        permalinkService_ = std::move(svc);
    }

    void setPushersService(std::shared_ptr<PushersService> svc) {
        pushersService_ = std::move(svc);
    }

    void setSearchService(std::shared_ptr<SearchService> svc) {
        searchService_ = std::move(svc);
    }

    void setSharedSecretStorageService(std::shared_ptr<SharedSecretStorageService> svc) {
        sharedSecretStorageService_ = std::move(svc);
    }

    void setSignOutService(std::shared_ptr<SignOutService> svc) {
        signOutService_ = std::move(svc);
    }

    void setSpaceService(std::shared_ptr<SpaceService> svc) {
        spaceService_ = std::move(svc);
    }

    void setTermsService(std::shared_ptr<TermsService> svc) {
        termsService_ = std::move(svc);
    }

    void setThirdPartyService(std::shared_ptr<ThirdPartyService> svc) {
        thirdPartyService_ = std::move(svc);
    }

    void setTypingUsersTracker(std::shared_ptr<TypingUsersTracker> svc) {
        typingUsersTracker_ = std::move(svc);
    }

    void setWidgetService(std::shared_ptr<WidgetService> svc) {
        widgetService_ = std::move(svc);
    }

    void setCallSignalingService(std::shared_ptr<CallSignalingService> svc) {
        callSignalingService_ = std::move(svc);
    }

    // ── Listeners ──
    void addListener(std::shared_ptr<SessionListener> listener) override {
        if (!listener) return;
        std::lock_guard<std::mutex> lock(listenerMutex_);
        listeners_.push_back(listener);
        __android_log_print(ANDROID_LOG_VERBOSE, TAG, "Listener added, total: %zu", listeners_.size());
    }

    void removeListener(std::shared_ptr<SessionListener> listener) override {
        if (!listener) return;
        std::lock_guard<std::mutex> lock(listenerMutex_);
        auto it = std::find_if(listeners_.begin(), listeners_.end(),
            [&](const std::weak_ptr<SessionListener>& wp) {
                auto sp = wp.lock();
                return sp && sp == listener;
            });
        if (it != listeners_.end()) {
            listeners_.erase(it);
        }
    }

    // ── Refresh Token ──
    void refreshToken(const std::string& refreshToken,
            std::function<void(bool, const std::string&)> callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Refreshing access token...");

        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client");
            return;
        }

        json body;
        body["refresh_token"] = refreshToken;
        body["grant_type"] = "refresh_token";

        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = params_.homeServerUrl + "/_matrix/client/r0/refresh",
            .timeoutMs = 15000
        };
        request.body = body.dump();
        request.contentType = "application/json";

        auto self = shared_from_this();
        httpClient_->execute(request, [this, self, callback](const HttpResponse& response) {
            if (!response.isSuccess()) {
                if (response.httpCode == 401 || response.httpCode == 403) {
                    notifyTokenInvalid();
                }
                if (callback) callback(false, "HTTP " + std::to_string(response.httpCode));
                return;
            }
            try {
                auto j = json::parse(response.body);
                std::string newAccessToken = j.value("access_token", "");
                std::string newRefreshToken = j.value("refresh_token", "");

                if (!newAccessToken.empty()) {
                    params_.accessToken = newAccessToken;
                    __android_log_print(ANDROID_LOG_INFO, TAG, "Token refreshed successfully");
                }
                if (!newRefreshToken.empty()) {
                    params_.refreshToken = newRefreshToken;
                }

                if (callback) callback(true, "");
            } catch (const std::exception& e) {
                if (callback) callback(false, std::string("Parse error: ") + e.what());
            }
        });
    }

    // ── Filter ──
    void setFilter(const SyncFilter& filter) override {
        currentFilter_ = filter;
        if (syncService_) {
            syncService_->setFilter(filter);
        }
        __android_log_print(ANDROID_LOG_INFO, TAG, "Filter updated");
    }

    SyncFilter getCurrentFilter() const override {
        return currentFilter_;
    }

    // ── Internal helpers ──
    void setHttpClient(std::shared_ptr<HttpClient> httpClient) {
        httpClient_ = std::move(httpClient);
    }

private:
    void validateParams() {
        openable_ = !params_.userId.empty() &&
                    !params_.homeServerUrl.empty() &&
                    !params_.accessToken.empty();
        if (!openable_) {
            __android_log_print(ANDROID_LOG_WARN, TAG,
                "Session not openable: userId=%s, homeServer=%s, hasToken=%s",
                params_.userId.c_str(),
                params_.homeServerUrl.c_str(),
                params_.accessToken.empty() ? "no" : "yes");
        }
    }

    // ── Sync Callbacks ──
    void onSyncResponse(const SyncResponse& response) {
        __android_log_print(ANDROID_LOG_VERBOSE, TAG, "Sync response received, nextBatch: %s",
            response.nextBatch.c_str());

        syncState_ = SyncState::IDLE;

        // Process joined rooms
        for (const auto& [roomId, roomJson] : response.joinedRooms) {
            processRoomSync(roomId, roomJson);
        }

        // Process invited rooms
        for (const auto& [roomId, roomJson] : response.invitedRooms) {
            processInvitedRoom(roomId, roomJson);
        }

        // Process left rooms
        for (const auto& [roomId, roomJson] : response.leftRooms) {
            processLeftRoom(roomId, roomJson);
        }

        // Process to-device events (crypto)
        if (cryptoService_ && !response.toDeviceJson.empty()) {
            try {
                auto toDevice = json::parse(response.toDeviceJson);
                if (toDevice.contains("events")) {
                    for (const auto& event : toDevice["events"]) {
                        std::string sender = event.value("sender", "");
                        std::string type = event.value("type", "");
                        if (type == "m.room.encrypted" || type == "m.room_key" ||
                            type == "m.room_key_request" || type.starts_with("m.key.verification")) {
                            cryptoService_->onLiveEvent(sender, event.dump());
                        }
                    }
                }
            } catch (const std::exception& e) {
                __android_log_print(ANDROID_LOG_ERROR, TAG, "To-device parse error: %s", e.what());
            }
        }

        // Update connection state
        if (connectionState_ != ConnectionState::CONNECTED) {
            connectionState_ = ConnectionState::CONNECTED;
            notifyConnectionStateChanged(ConnectionState::CONNECTED);
        }

        notifySyncStateChanged(SyncState::IDLE);
    }

    void onSyncError(const MatrixError& error) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Sync error: %s (code: %d)",
            error.errorMessage.c_str(), error.httpCode);

        connectionState_ = ConnectionState::RECONNECTING;
        notifyConnectionStateChanged(ConnectionState::RECONNECTING);

        if (error.isTokenError) {
            notifyTokenInvalid();
        }

        notifyGlobalError(error);
    }

    void processRoomSync(const std::string& roomId, const std::string& roomJson) {
        try {
            auto room = json::parse(roomJson);
            int64_t eventCount = 0;

            // Process timeline events
            if (room.contains("timeline") && room["timeline"].contains("events")) {
                eventCount = room["timeline"]["events"].size();

                // Notify crypto service of events in encrypted rooms
                if (cryptoService_ && cryptoService_->isRoomEncrypted(roomId)) {
                    for (const auto& event : room["timeline"]["events"]) {
                        cryptoService_->onLiveEvent(roomId, event.dump());
                    }
                }
            }

            // Process state events
            if (room.contains("state") && room["state"].contains("events")) {
                for (const auto& event : room["state"]["events"]) {
                    std::string type = event.value("type", "");
                    if (type == "m.room.encryption") {
                        // Mark room as encrypted
                        if (cryptoService_) {
                            cryptoService_->setRoomEncrypted(roomId, true);
                        }
                    }
                }
            }

            if (eventCount > 0) {
                __android_log_print(ANDROID_LOG_VERBOSE, TAG,
                    "Room %s: %lld events", roomId.c_str(), (long long)eventCount);
            }
        } catch (const std::exception& e) {
            __android_log_print(ANDROID_LOG_ERROR, TAG,
                "Room sync parse error for %s: %s", roomId.c_str(), e.what());
        }
    }

    void processInvitedRoom(const std::string& roomId, const std::string& roomJson) {
        __android_log_print(ANDROID_LOG_VERBOSE, TAG, "Invited room: %s", roomId.c_str());
    }

    void processLeftRoom(const std::string& roomId, const std::string& roomJson) {
        __android_log_print(ANDROID_LOG_VERBOSE, TAG, "Left room: %s", roomId.c_str());
    }

    // ── Listener Notifications ──
    void notifySyncStateChanged(SyncState state) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        syncState_ = state;
        for (auto it = listeners_.begin(); it != listeners_.end(); ) {
            auto listener = it->lock();
            if (listener) {
                listener->onSyncStateChanged(state);
                ++it;
            } else {
                it = listeners_.erase(it);
            }
        }
    }

    void notifyConnectionStateChanged(ConnectionState state) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        for (auto it = listeners_.begin(); it != listeners_.end(); ) {
            auto listener = it->lock();
            if (listener) {
                listener->onConnectionStateChanged(state);
                ++it;
            } else {
                it = listeners_.erase(it);
            }
        }
    }

    void notifyGlobalError(const MatrixError& error) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        for (auto it = listeners_.begin(); it != listeners_.end(); ) {
            auto listener = it->lock();
            if (listener) {
                listener->onGlobalError(error);
                ++it;
            } else {
                it = listeners_.erase(it);
            }
        }
    }

    void notifySessionStateChanged(const SessionState& state) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        for (auto it = listeners_.begin(); it != listeners_.end(); ) {
            auto listener = it->lock();
            if (listener) {
                listener->onSessionStateChanged(state);
                ++it;
            } else {
                it = listeners_.erase(it);
            }
        }
    }

    void notifyTokenInvalid() {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        params_.isTokenValid = false;
        for (auto it = listeners_.begin(); it != listeners_.end(); ) {
            auto listener = it->lock();
            if (listener) {
                listener->onTokenInvalid();
                ++it;
            } else {
                it = listeners_.erase(it);
            }
        }
    }

    SessionParams params_;
    std::shared_ptr<HttpClient> httpClient_;

    std::atomic<bool> alive_;
    bool openable_;
    std::atomic<ConnectionState> connectionState_;
    std::atomic<SyncState> syncState_;

    SyncFilter currentFilter_;

    // Core services
    std::shared_ptr<CryptoService> cryptoService_;
    std::shared_ptr<SyncService> syncService_;

    // Service instances
    std::shared_ptr<SessionAccountDataService> accountDataService_;
    std::shared_ptr<AccountService> accountService_;
    std::shared_ptr<RoomService> roomService_;
    std::shared_ptr<UserService> userService_;
    std::shared_ptr<EventService> eventService_;
    std::shared_ptr<PushRuleService> pushRuleService_;
    std::shared_ptr<ProfileService> profileService_;
    std::shared_ptr<PresenceService> presenceService_;
    std::shared_ptr<MediaService> mediaService_;
    std::shared_ptr<FileService> fileService_;
    std::shared_ptr<ContentScannerService> contentScannerService_;
    std::shared_ptr<ContentUrlResolver> contentUrlResolver_;
    std::shared_ptr<ContentUploadStateTracker> contentUploadStateTracker_;
    std::shared_ptr<ContentDownloadStateTracker> contentDownloadStateTracker_;
    std::shared_ptr<HomeServerCapabilitiesService> homeServerCapabilitiesService_;
    std::shared_ptr<IdentityService> identityService_;
    std::shared_ptr<IntegrationManagerService> integrationManagerService_;
    std::shared_ptr<OpenIdService> openIdService_;
    std::shared_ptr<PermalinkService> permalinkService_;
    std::shared_ptr<PushersService> pushersService_;
    std::shared_ptr<SearchService> searchService_;
    std::shared_ptr<SharedSecretStorageService> sharedSecretStorageService_;
    std::shared_ptr<SignOutService> signOutService_;
    std::shared_ptr<SpaceService> spaceService_;
    std::shared_ptr<TermsService> termsService_;
    std::shared_ptr<ThirdPartyService> thirdPartyService_;
    std::shared_ptr<TypingUsersTracker> typingUsersTracker_;
    std::shared_ptr<WidgetService> widgetService_;
    std::shared_ptr<CallSignalingService> callSignalingService_;

    // Listeners
    mutable std::mutex listenerMutex_;
    std::vector<std::weak_ptr<SessionListener>> listeners_;
};

// ── SessionFactoryImpl ──
class SessionFactoryImpl : public SessionFactory {
public:
    explicit SessionFactoryImpl(std::shared_ptr<HttpClient> httpClient)
        : httpClient_(std::move(httpClient)) {
    }

    std::shared_ptr<Session> createSession(const SessionParams& params) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Creating session for %s", params.userId.c_str());

        auto session = std::make_shared<SessionImpl>(params);
        session->setHttpClient(httpClient_);

        // Create and wire core services
        auto crypto = createCryptoService(httpClient_);
        auto sync = createSyncService(httpClient_);

        // Configure crypto service
        crypto->setDeviceId(params.deviceId);
        crypto->setCurrentUserId(params.userId);
        crypto->setHomeserverUrl(params.homeServerUrl);

        // Configure sync service
        sync->setUserId(params.userId);
        sync->setAccessToken(params.accessToken);
        sync->setHomeserverUrl(params.homeServerUrl);
        sync->setNextBatchToken("");
        sync->setCryptoService(crypto);

        // Initialize crypto
        crypto->initialize([crypto](bool success, const std::string& error) {
            if (success) {
                __android_log_print(ANDROID_LOG_INFO, TAG, "Crypto initialized in session factory");
            } else {
                __android_log_print(ANDROID_LOG_ERROR, TAG, "Crypto init failed: %s", error.c_str());
            }
        });

        // Wire services to session
        session->setCryptoService(std::move(crypto));
        session->setSyncService(std::move(sync));

        return session;
    }

    std::shared_ptr<Session> restoreSession(const std::string& storedData) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Restoring session from stored data");
        try {
            auto j = json::parse(storedData);
            SessionParams params;
            params.userId = j.value("userId", "");
            params.accessToken = j.value("accessToken", "");
            params.deviceId = j.value("deviceId", "");
            params.homeServerUrl = j.value("homeServerUrl", "");
            params.identityServerUrl = j.value("identityServerUrl", "");
            params.refreshToken = j.value("refreshToken", "");
            params.tokenExpiryTime = j.value("tokenExpiryTime", 0);
            params.isTokenValid = j.value("isTokenValid", false);

            auto session = createSession(params);

            // Restore next batch token from stored sync position
            if (j.contains("nextBatchToken")) {
                auto sync = session->syncService();
                if (sync) {
                    sync->setNextBatchToken(j["nextBatchToken"].get<std::string>());
                }
            }

            // Restore filter
            if (j.contains("filterId")) {
                auto sync = session->syncService();
                if (sync) {
                    sync->setFilterId(j["filterId"].get<std::string>());
                }
            }

            return session;
        } catch (const std::exception& e) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Session restore failed: %s", e.what());
            return nullptr;
        }
    }

    std::string serializeSession(const std::shared_ptr<Session>& session) override {
        if (!session) return "{}";

        json j;
        j["userId"] = session->getUserId();
        j["deviceId"] = session->getDeviceId();
        j["accessToken"] = session->getAccessToken();
        j["homeServerUrl"] = session->getHomeServerUrl();

        auto params = session->getSessionParams();
        j["identityServerUrl"] = params.identityServerUrl;
        j["refreshToken"] = params.refreshToken;
        j["tokenExpiryTime"] = params.tokenExpiryTime;
        j["isTokenValid"] = params.isTokenValid;

        auto sync = session->syncService();
        if (sync) {
            j["nextBatchToken"] = sync->getCurrentToken();
            j["filterId"] = sync->getFilterId();
        }

        return j.dump();
    }

private:
    std::shared_ptr<HttpClient> httpClient_;
};

} // namespace matrix_sdk
