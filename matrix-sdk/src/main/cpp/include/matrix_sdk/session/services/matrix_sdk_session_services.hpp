#ifndef MATRIX_SDK_SESSION_SERVICES_HPP
#define MATRIX_SDK_SESSION_SERVICES_HPP

#include "matrix_sdk_types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace matrix_sdk {

// ── Account Service ──
class AccountService {
public:
    virtual ~AccountService() = default;

    virtual void changePassword(const std::string& oldPassword, const std::string& newPassword, SimpleCallback callback) = 0;
    virtual void deactivateAccount(const std::string& authJson, SimpleCallback callback) = 0;
    virtual void changeDisplayName(const std::string& displayName, SimpleCallback callback) = 0;
    virtual void changeAvatar(const std::string& mxcUrl, SimpleCallback callback) = 0;
    virtual void getThreePids(std::function<void(bool, const std::string&, const std::vector<std::string>&)> callback) = 0;
    virtual void addThreePid(const std::string& address, const std::string& medium, SimpleCallback callback) = 0;
    virtual void deleteThreePid(const std::string& address, const std::string& medium, SimpleCallback callback) = 0;
    virtual void requestTokenEmail(const std::string& email, const std::string& clientSecret, int sendAttempt, SimpleCallback callback) = 0;
    virtual void requestTokenSms(const std::string& phone, const std::string& clientSecret, int sendAttempt, SimpleCallback callback) = 0;
};

// ── Session Account Data Service ──
class SessionAccountDataService {
public:
    virtual ~SessionAccountDataService() = default;

    virtual void setAccountData(const std::string& type, const std::string& content, SimpleCallback callback) = 0;
    virtual std::string getAccountData(const std::string& type) const = 0;
    virtual std::map<std::string, std::string> getAllAccountData() const = 0;
    virtual void updateDirectRooms(const std::map<std::string, std::vector<std::string>>& directRooms, SimpleCallback callback) = 0;
    virtual std::map<std::string, std::vector<std::string>> getDirectRooms() const = 0;
    virtual std::string getGlobalAccountData(const std::string& type) const = 0;
    virtual void setGlobalAccountData(const std::string& type, const std::string& content, SimpleCallback callback) = 0;
};

// ── Call Signaling Service ──
class CallSignalingService {
public:
    virtual ~CallSignalingService() = default;

    virtual void placeCall(const std::string& roomId, const std::string& callId, SimpleCallback callback) = 0;
    virtual void answerCall(const std::string& roomId, const std::string& callId, SimpleCallback callback) = 0;
    virtual void hangupCall(const std::string& roomId, const std::string& callId, SimpleCallback callback) = 0;
    virtual void sendCandidates(const std::string& roomId, const std::string& callId, const std::string& candidates, SimpleCallback callback) = 0;
};

// ── Content Upload State Tracker ──
class ContentUploadStateTracker {
public:
    virtual ~ContentUploadStateTracker() = default;

    virtual void trackUpload(const std::string& uploadId, ProgressCallback progress) = 0;
    virtual void untrackUpload(const std::string& uploadId) = 0;
    virtual int getProgress(const std::string& uploadId) const = 0;
    virtual int getActiveUploads() const = 0;
};

// ── Content URL Resolver ──
class ContentUrlResolver {
public:
    virtual ~ContentUrlResolver() = default;

    virtual std::string resolveUrl(const std::string& mxcUrl) const = 0;
    virtual std::string resolveThumbnailUrl(const std::string& mxcUrl, const ThumbnailSize& size) const = 0;
    virtual std::string resolveDownloadUrl(const std::string& mxcUrl) const = 0;
    virtual std::string resolveUploadUrl() const = 0;
    virtual std::string getHomeServerContentUrl() const = 0;
};

// ── Content Scanner Service ──
class ContentScannerService {
public:
    virtual ~ContentScannerService() = default;

    virtual void scanContent(const std::string& mxcUrl, SimpleCallback callback) = 0;
    virtual void getScanResult(const std::string& scanId,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual bool isContentScannerSupported() const = 0;
};

// ── Content Download State Tracker ──
class ContentDownloadStateTracker {
public:
    virtual ~ContentDownloadStateTracker() = default;

    virtual void trackDownload(const std::string& downloadId, ProgressCallback progress) = 0;
    virtual void untrackDownload(const std::string& downloadId) = 0;
    virtual int getProgress(const std::string& downloadId) const = 0;
};

// ── File Service ──
class FileService {
public:
    virtual ~FileService() = default;

    virtual void downloadFile(const std::string& mxcUrl, const std::string& filename,
        const std::string& mimeType, ProgressCallback progress, SimpleCallback callback) = 0;
    virtual void cancelDownload(const std::string& downloadId) = 0;
    virtual std::string getLocalFilePath(const std::string& mxcUrl) const = 0;
    virtual bool isFileDownloaded(const std::string& mxcUrl) const = 0;
    virtual int64_t getFileSize(const std::string& mxcUrl) const = 0;
};

// ── HomeServer Capabilities Service ──
class HomeServerCapabilitiesService {
public:
    virtual ~HomeServerCapabilitiesService() = default;

    virtual void fetchCapabilities(std::function<void(bool, const std::string&)> callback) = 0;
    virtual bool canChangePassword() const = 0;
    virtual bool canDeactivateAccount() const = 0;
    virtual std::string getMaxUploadSize() const = 0;
    virtual bool supportsThreads() const = 0;
    virtual bool supportsSpaces() const = 0;
    virtual bool supportsPoll() const = 0;
    virtual bool supportsLocationSharing() const = 0;
    virtual bool supportsBeacon() const = 0;
    virtual bool supportsVoiceBroadcast() const = 0;
    virtual bool supportsVersions(const std::vector<std::string>& versions) const = 0;
    virtual std::vector<std::string> getSupportedVersions() const = 0;
};

// ── Identity Service ──
class IdentityService {
public:
    virtual ~IdentityService() = default;

    virtual void lookup(const std::string& address, const std::string& medium,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual void bulkLookup(const std::vector<std::string>& addresses, const std::string& medium,
        std::function<void(bool, const std::string&, const std::map<std::string, std::string>&)> callback) = 0;
    virtual void storeInvite(const std::string& roomId, const std::string& eventId, const std::string& address,
        const std::string& medium, const std::string& roomName, SimpleCallback callback) = 0;
    virtual std::string getIdentityServerUrl() const = 0;
};

// ── Integration Manager Service ──
class IntegrationManagerService {
public:
    virtual ~IntegrationManagerService() = default;

    virtual void getIntegrationManagers(
        std::function<void(bool, const std::string&, const std::vector<std::string>&)> callback) = 0;
    virtual void openIntegrationManager(const std::string& integrationId, const std::string& roomId,
        const std::string& screen, const std::string& integrationUrl, SimpleCallback callback) = 0;
    virtual std::string getIntegrationManagerUrl() const = 0;
};

// ── Media Service ──
class MediaService {
public:
    using MediaCallback = std::function<void(bool, const std::string&, const std::string&, const MediaInfo&)>;

    virtual ~MediaService() = default;

    virtual void uploadMedia(const std::string& localPath, const std::string& filename,
        const std::string& mimeType, ProgressCallback progress, MediaCallback callback) = 0;
    virtual void downloadMedia(const std::string& mxcUrl, ProgressCallback progress, SimpleCallback callback) = 0;
    virtual std::string getCachedPath(const std::string& mxcUrl) const = 0;
    virtual void getThumbnail(const std::string& mxcUrl, const ThumbnailSize& size,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual void cancelUpload(const std::string& uploadId) = 0;
    virtual void cancelDownload(const std::string& downloadId) = 0;
};

// ── OpenId Service ──
class OpenIdService {
public:
    virtual ~OpenIdService() = default;

    virtual void getOpenIdToken(std::function<void(bool, const std::string&, const OpenIdToken&)> callback) = 0;
    virtual bool isTokenValid(const OpenIdToken& token) const = 0;
};

// ── Permalink Service ──
class PermalinkService {
public:
    virtual ~PermalinkService() = default;

    virtual std::string createPermalink(const std::string& roomId, const std::string& eventId = "") const = 0;
    virtual std::string createRoomLink(const std::string& roomId, const std::vector<std::string>& viaServers) const = 0;
    virtual std::string createUserLink(const std::string& userId) const = 0;
    virtual bool isPermalink(const std::string& url) const = 0;
    virtual bool parsePermalink(const std::string& url, std::string& roomId, std::string& eventId) const = 0;
};

// ── Presence Service ──
class PresenceService {
public:
    virtual ~PresenceService() = default;

    virtual void setPresence(const std::string& presence, const std::string& statusMsg, SimpleCallback callback) = 0;
    virtual std::string getPresence(const std::string& userId) const = 0;
    virtual std::string getStatusMessage(const std::string& userId) const = 0;
    virtual bool isUserOnline(const std::string& userId) const = 0;
    virtual int64_t getLastActiveAgo(const std::string& userId) const = 0;
};

// ── Profile Service ──
class ProfileService {
public:
    using ProfileCallback = std::function<void(bool, const std::string&, const UserProfile&)>;

    virtual ~ProfileService() = default;

    virtual void fetchProfile(const std::string& userId, ProfileCallback callback) = 0;
    virtual std::optional<UserProfile> getCachedProfile(const std::string& userId) const = 0;
    virtual void setDisplayName(const std::string& displayName, SimpleCallback callback) = 0;
    virtual void setAvatarUrl(const std::string& mxcUrl, SimpleCallback callback) = 0;
    virtual void refreshProfile(SimpleCallback callback) = 0;
};

// ── Pushers Service ──
class PushersService {
public:
    virtual ~PushersService() = default;

    virtual void addPusher(const std::string& pushKey, const std::string& appId, const std::string& appDisplayName,
        const std::string& deviceDisplayName, const std::string& lang, const std::string& profileTag,
        const std::string& appUrl, bool append, SimpleCallback callback) = 0;
    virtual void removePusher(const std::string& pushKey, const std::string& appId, SimpleCallback callback) = 0;
    virtual void getPushers(std::function<void(bool, const std::string&, const std::vector<std::string>&)> callback) = 0;
};

// ── Push Rule Service ──
class PushRuleService {
public:
    virtual ~PushRuleService() = default;

    virtual void fetchPushRules(SimpleCallback callback) = 0;
    virtual void setPushRuleEnabled(const std::string& ruleId, bool enabled, SimpleCallback callback) = 0;
    virtual void setPushRuleActions(const std::string& ruleId, const std::vector<std::string>& actions, SimpleCallback callback) = 0;
    virtual std::string getPushRuleForRoom(const std::string& roomId) const = 0;
    virtual bool isPushRuleEnabled(const std::string& ruleId) const = 0;
    virtual std::vector<std::string> getAllPushRules() const = 0;
};

// ── Room Directory Service ──
class RoomDirectoryService {
public:
    using PublicRoomsCallback = std::function<void(bool, const std::string&, const std::vector<PublicRoom>&, const std::string&, int)>;

    virtual ~RoomDirectoryService() = default;

    virtual void getPublicRooms(const std::string& server, int limit, const std::string& since,
        const std::string& filter, bool includeAllNetworks, const std::string& thirdPartyInstanceId,
        PublicRoomsCallback callback) = 0;
    virtual void getRoomPreview(const std::string& roomId, const std::vector<std::string>& viaServers,
        std::function<void(bool, const std::string&, const RoomPreview&)> callback) = 0;
    virtual void getThirdPartyProtocol(const std::string& protocol,
        std::function<void(bool, const std::string&, const ThirdPartyProtocol&)> callback) = 0;
    virtual void getThirdPartyUser(const std::string& protocol, const std::string& fields,
        std::function<void(bool, const std::string&, const ThirdPartyUser&)> callback) = 0;
    virtual void getThirdPartyLocation(const std::string& protocol, const std::string& fields,
        std::function<void(bool, const std::string&, const ThirdPartyLocation&)> callback) = 0;
};

// ── Room Service ──
class RoomService {
public:
    using RoomCallback = std::function<void(bool, const std::string&, std::shared_ptr<Room>)>;

    virtual ~RoomService() = default;

    virtual std::shared_ptr<Room> getRoom(const std::string& roomId) = 0;
    virtual void getRoomLive(const std::string& roomId, RoomCallback callback) = 0;
    virtual std::vector<std::string> getRoomIds() const = 0;
    virtual void createRoom(const CreateRoomParams& params,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual void joinRoom(const std::string& roomId, const std::vector<std::string>& viaServers,
        SimpleCallback callback) = 0;
    virtual void leaveRoom(const std::string& roomId, SimpleCallback callback) = 0;
    virtual void forgetRoom(const std::string& roomId, SimpleCallback callback) = 0;
    virtual void joinRoomByAlias(const std::string& alias, SimpleCallback callback) = 0;
    virtual void knockRoom(const std::string& roomId, const std::vector<std::string>& viaServers,
        const std::string& reason, SimpleCallback callback) = 0;
    virtual void getExistingDirectRoomWithUser(const std::string& userId,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual void createDirectRoom(const std::string& userId,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
};

// ── Search Service ──
class SearchService {
public:
    using SearchCallback = std::function<void(bool, const std::string&, const std::vector<SearchResult>&, int)>;

    virtual ~SearchService() = default;

    virtual void search(const std::string& query, int limit, const std::string& nextBatch,
        const std::string& order, const std::vector<std::string>& roomIds, SearchCallback callback) = 0;
    virtual void searchRooms(const std::string& query, int limit, SearchCallback callback) = 0;
    virtual void searchMessages(const std::string& query, int limit, SearchCallback callback) = 0;
    virtual void searchUsers(const std::string& query, int limit,
        std::function<void(bool, const std::string&, const std::vector<UserProfile>&)> callback) = 0;
};

// ── Shared Secret Storage Service ──
class SharedSecretStorageService {
public:
    virtual ~SharedSecretStorageService() = default;

    virtual void setSecret(const std::string& name, const std::string& value, SimpleCallback callback) = 0;
    virtual void getSecret(const std::string& name,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual void deleteSecret(const std::string& name, SimpleCallback callback) = 0;
    virtual void getDefaultKey(std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual void setDefaultKey(const std::string& keyId, SimpleCallback callback) = 0;
};

// ── Sign Out Service ──
class SignOutService {
public:
    virtual ~SignOutService() = default;

    virtual void signOut(bool signOutAllDevices, SimpleCallback callback) = 0;
    virtual void softLogout(SimpleCallback callback) = 0;
    virtual void clearLocalData(SimpleCallback callback) = 0;
};

// ── Space Service ──
class SpaceService {
public:
    using SpaceChildrenCallback = std::function<void(bool, const std::string&, const std::vector<SpaceChildInfo>&)>;

    virtual ~SpaceService() = default;

    virtual void getSpaceChildren(const std::string& spaceId, int maxRoomsPerSpace,
        SpaceChildrenCallback callback) = 0;
    virtual void getSpaceSummary(const std::string& spaceId,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual std::shared_ptr<Room> getSpace(const std::string& spaceId) = 0;
    virtual std::vector<std::string> getRootSpaces() const = 0;
    virtual void addRoomToSpace(const std::string& spaceId, const std::string& roomId,
        bool suggested, const std::string& order, const std::string& via, SimpleCallback callback) = 0;
    virtual void removeRoomFromSpace(const std::string& spaceId, const std::string& roomId, SimpleCallback callback) = 0;
    virtual void setSpaceOrder(const std::string& spaceId, const std::string& order, SimpleCallback callback) = 0;
};

// ── Terms Service ──
class TermsService {
public:
    virtual ~TermsService() = default;

    virtual void getTerms(const std::string& baseUrl, const std::string& language,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual void agreeToTerms(const std::string& baseUrl, const std::vector<std::string>& agreedUrls,
        SimpleCallback callback) = 0;
    virtual bool hasUnagreedTerms() const = 0;
};

// ── Third Party Service ──
class ThirdPartyService {
public:
    virtual ~ThirdPartyService() = default;

    virtual void getThirdPartyProtocol(const std::string& protocol,
        std::function<void(bool, const std::string&, const ThirdPartyProtocol&)> callback) = 0;
    virtual void getThirdPartyUser(const std::string& protocol, const std::string& fields,
        std::function<void(bool, const std::string&, const ThirdPartyUser&)> callback) = 0;
    virtual void getThirdPartyLocation(const std::string& protocol, const std::string& fields,
        std::function<void(bool, const std::string&, const ThirdPartyLocation&)> callback) = 0;
};

// ── Typing Users Tracker ──
class TypingUsersTracker {
public:
    virtual ~TypingUsersTracker() = default;

    virtual void trackTyping(const std::string& roomId, SimpleCallback callback) = 0;
    virtual std::vector<std::string> getTypingUsers(const std::string& roomId) const = 0;
    virtual bool isUserTyping(const std::string& roomId, const std::string& userId) const = 0;
};

// ── User Service ──
class UserService {
public:
    using UserCallback = std::function<void(bool, const std::string&, const UserProfile&)>;

    virtual ~UserService() = default;

    virtual void getUser(const std::string& userId, UserCallback callback) = 0;
    virtual std::optional<UserProfile> getCachedUser(const std::string& userId) const = 0;
    virtual void fetchUsers(const std::vector<std::string>& userIds,
        std::function<void(bool, const std::string&, const std::map<std::string, UserProfile>&)> callback) = 0;
    virtual void searchUsers(const std::string& query, int limit,
        std::function<void(bool, const std::string&, const std::vector<UserProfile>&)> callback) = 0;
    virtual void setDisplayName(const std::string& displayName, SimpleCallback callback) = 0;
    virtual void setAvatarUrl(const std::string& mxcUrl, SimpleCallback callback) = 0;
    virtual void ignoreUser(const std::string& userId, SimpleCallback callback) = 0;
    virtual void unignoreUser(const std::string& userId, SimpleCallback callback) = 0;
    virtual std::vector<std::string> getIgnoredUsers() const = 0;
};

// ── Widget Service ──
class WidgetService {
public:
    virtual ~WidgetService() = default;

    virtual void getWidgets(const std::string& roomId,
        std::function<void(bool, const std::string&, const std::vector<Widget>&)> callback) = 0;
    virtual void createWidget(const std::string& roomId, const Widget& widget, SimpleCallback callback) = 0;
    virtual void deleteWidget(const std::string& roomId, const std::string& widgetId, SimpleCallback callback) = 0;
    virtual void getWidget(const std::string& roomId, const std::string& widgetId,
        std::function<void(bool, const std::string&, const Widget&)> callback) = 0;
};

} // namespace matrix_sdk

#endif
