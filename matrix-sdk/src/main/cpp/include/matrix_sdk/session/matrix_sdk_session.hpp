#ifndef MATRIX_SDK_SESSION_HPP
#define MATRIX_SDK_SESSION_HPP

#include "matrix_sdk_types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <optional>

namespace matrix_sdk {

// Forward declarations for all services
class AccountService;
class SessionAccountDataService;
class CallSignalingService;
class ContentUploadStateTracker;
class ContentUrlResolver;
class ContentScannerService;
class CryptoService;
class EventService;
class ContentDownloadStateTracker;
class FileService;
class HomeServerCapabilitiesService;
class IdentityService;
class IntegrationManagerService;
class MediaService;
class OpenIdService;
class PermalinkService;
class PresenceService;
class ProfileService;
class PushersService;
class PushRuleService;
class RoomDirectoryService;
class RoomService;
class SearchService;
class SharedSecretStorageService;
class SignOutService;
class SpaceService;
class SyncService;
class TermsService;
class ThirdPartyService;
class TypingUsersTracker;
class UserService;
class WidgetService;
class RoomSummary;

// ── Session State ──
struct SessionState {
    std::string userId;
    std::string deviceId;
    SyncState syncState = SyncState::IDLE;
    ConnectionState connectionState = ConnectionState::DISCONNECTED;
    bool isOpenable = false;
    bool isEncryptionEnabled = false;
    int pendingRoomCount = 0;
    std::string homeserverUrl;
    std::string identityServerUrl;
};

// ── Session Listener ──
class SessionListener {
public:
    virtual ~SessionListener() = default;
    virtual void onSyncStateChanged(SyncState state) {}
    virtual void onConnectionStateChanged(ConnectionState state) {}
    virtual void onGlobalError(const MatrixError& error) {}
    virtual void onSessionStateChanged(const SessionState& state) {}
    virtual void onTokenInvalid() {}
};

// ── Session ──
class Session {
public:
    virtual ~Session() = default;

    // ── Identity ──
    virtual std::string getUserId() const = 0;
    virtual std::string getDeviceId() const = 0;
    virtual std::string getAccessToken() const = 0;
    virtual std::string getHomeServerUrl() const = 0;
    virtual SessionParams getSessionParams() const = 0;

    // ── State ──
    virtual bool isOpenable() const = 0;
    virtual SessionState getState() const = 0;
    virtual SyncState getSyncState() const = 0;

    // ── Lifecycle ──
    virtual void open() = 0;
    virtual void close() = 0;
    virtual void requireBackgroundSync() = 0;
    virtual void pauseSync() = 0;
    virtual void resumeSync() = 0;
    virtual bool isAlive() const = 0;

    // ── Services ──
    virtual std::shared_ptr<CryptoService> cryptoService() = 0;
    virtual std::shared_ptr<SessionAccountDataService> accountDataService() = 0;
    virtual std::shared_ptr<AccountService> accountService() = 0;
    virtual std::shared_ptr<SyncService> syncService() = 0;
    virtual std::shared_ptr<RoomService> roomService() = 0;
    virtual std::shared_ptr<UserService> userService() = 0;
    virtual std::shared_ptr<EventService> eventService() = 0;
    virtual std::shared_ptr<PushRuleService> pushRuleService() = 0;
    virtual std::shared_ptr<ProfileService> profileService() = 0;
    virtual std::shared_ptr<PresenceService> presenceService() = 0;
    virtual std::shared_ptr<MediaService> mediaService() = 0;
    virtual std::shared_ptr<FileService> fileService() = 0;
    virtual std::shared_ptr<ContentScannerService> contentScannerService() = 0;
    virtual std::shared_ptr<ContentUrlResolver> contentUrlResolver() = 0;
    virtual std::shared_ptr<ContentUploadStateTracker> contentUploadStateTracker() = 0;
    virtual std::shared_ptr<ContentDownloadStateTracker> contentDownloadStateTracker() = 0;
    virtual std::shared_ptr<HomeServerCapabilitiesService> homeServerCapabilitiesService() = 0;
    virtual std::shared_ptr<IdentityService> identityService() = 0;
    virtual std::shared_ptr<IntegrationManagerService> integrationManagerService() = 0;
    virtual std::shared_ptr<OpenIdService> openIdService() = 0;
    virtual std::shared_ptr<PermalinkService> permalinkService() = 0;
    virtual std::shared_ptr<PushersService> pushersService() = 0;
    virtual std::shared_ptr<SearchService> searchService() = 0;
    virtual std::shared_ptr<SharedSecretStorageService> sharedSecretStorageService() = 0;
    virtual std::shared_ptr<SignOutService> signOutService() = 0;
    virtual std::shared_ptr<SpaceService> spaceService() = 0;
    virtual std::shared_ptr<TermsService> termsService() = 0;
    virtual std::shared_ptr<ThirdPartyService> thirdPartyService() = 0;
    virtual std::shared_ptr<TypingUsersTracker> typingUsersTracker() = 0;
    virtual std::shared_ptr<WidgetService> widgetService() = 0;
    virtual std::shared_ptr<CallSignalingService> callSignalingService() = 0;

    // ── Listeners ──
    virtual void addListener(std::shared_ptr<SessionListener> listener) = 0;
    virtual void removeListener(std::shared_ptr<SessionListener> listener) = 0;

    // ── Refresh ──
    virtual void refreshToken(const std::string& refreshToken,
        std::function<void(bool, const std::string&)> callback) = 0;

    // ── Filter ──
    virtual void setFilter(const SyncFilter& filter) = 0;
    virtual SyncFilter getCurrentFilter() const = 0;
};

// ── Session Factory ──
class SessionFactory {
public:
    virtual ~SessionFactory() = default;
    virtual std::shared_ptr<Session> createSession(const SessionParams& params) = 0;
    virtual std::shared_ptr<Session> restoreSession(const std::string& storedData) = 0;
    virtual std::string serializeSession(const std::shared_ptr<Session>& session) = 0;
};

} // namespace matrix_sdk

#endif
