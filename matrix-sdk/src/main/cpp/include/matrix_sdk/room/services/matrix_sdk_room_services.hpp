#ifndef MATRIX_SDK_ROOM_SERVICES_HPP
#define MATRIX_SDK_ROOM_SERVICES_HPP

#include "matrix_sdk_types.hpp"
#include "../matrix_sdk_room.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace matrix_sdk {

// ── Timeline Event Listener ──
class TimelineEventListener {
public:
    virtual ~TimelineEventListener() = default;
    virtual void onNewEvents(const std::vector<std::shared_ptr<TimelineEvent>>& events) {}
    virtual void onTimelineUpdated() {}
    virtual void onEventModified(const std::shared_ptr<TimelineEvent>& event) {}
    virtual void onEventDeleted(const std::string& eventId) {}
};

// ── Timeline Service ──
class TimelineService {
public:
    virtual ~TimelineService() = default;

    // ── Pagination ──
    virtual void paginate(int count, const std::string& direction,
        std::function<void(bool, const std::string&, bool)> callback) = 0;
    virtual void paginateForward(int count, SimpleCallback callback) = 0;
    virtual void paginateBackward(int count, SimpleCallback callback) = 0;
    virtual bool canPaginateForward() const = 0;
    virtual bool canPaginateBackward() const = 0;
    virtual bool isLive() const = 0;

    // ── Events ──
    virtual std::vector<std::shared_ptr<TimelineEvent>> getEvents() const = 0;
    virtual std::shared_ptr<TimelineEvent> getEvent(const std::string& eventId) const = 0;
    virtual int getEventCount() const = 0;
    virtual std::shared_ptr<TimelineEvent> getLastLiveEvent() const = 0;

    // ── Listeners ──
    virtual void addListener(std::shared_ptr<TimelineEventListener> listener) = 0;
    virtual void removeListener(std::shared_ptr<TimelineEventListener> listener) = 0;

    // ── Filters ──
    virtual void filterByType(EventType type) = 0;
    virtual void filterByTypes(const std::vector<EventType>& types) = 0;
    virtual void clearFilter() = 0;
    virtual void filterByThread(const std::string& threadId) = 0;
    virtual void clearThreadFilter() = 0;
};

// ── Send Service ──
class SendService {
public:
    using SendCallback = std::function<void(bool, const std::string&, const std::shared_ptr<TimelineEvent>&)>;

    virtual ~SendService() = default;

    virtual void sendMessage(const std::string& body, MessageType type, SendCallback callback) = 0;
    virtual void sendFormattedMessage(const std::string& body, const std::string& formattedBody, SendCallback callback) = 0;
    virtual void sendReaction(const std::string& eventId, const std::string& reactionKey, SendCallback callback) = 0;
    virtual void sendRedaction(const std::string& eventId, const std::string& reason, SendCallback callback) = 0;
    virtual void sendEvent(EventType type, const std::string& contentJson, SendCallback callback) = 0;
    virtual void sendMedia(const std::string& url, const std::string& filename, const std::string& mimeType,
        SendCallback callback) = 0;
    virtual void sendLocation(const LocationData& location, SendCallback callback) = 0;
    virtual void sendPoll(const PollQuestion& poll, SendCallback callback) = 0;
    virtual void sendPollResponse(const std::string& pollEventId, const std::vector<int>& answers, SendCallback callback) = 0;
    virtual void sendReply(const std::string& eventId, const std::string& body, SendCallback callback) = 0;
    virtual void sendEdit(const std::string& eventId, const std::string& newBody, SendCallback callback) = 0;
    virtual void sendReadReceipt(const std::string& eventId, SimpleCallback callback) = 0;
    virtual void sendReadMarker(const std::string& eventId, SimpleCallback callback) = 0;
    virtual void cancelSend(const std::string& transactionId) = 0;
    virtual void resend(const std::string& transactionId, SendCallback callback) = 0;
    virtual void deleteUnsent(const std::string& transactionId) = 0;
};

// ── Membership Service ──
class MembershipService {
public:
    virtual ~MembershipService() = default;

    virtual void loadMembers(std::function<void(bool, const std::string&, const std::vector<RoomMember>&)> callback) = 0;
    virtual std::vector<RoomMember> getCachedMembers() const = 0;
    virtual std::optional<RoomMember> getMember(const std::string& userId) const = 0;
    virtual int getNumberOfJoinedMembers() const = 0;
    virtual int getNumberOfInvitedMembers() const = 0;
    virtual std::vector<std::string> getJoinedMemberIds() const = 0;
    virtual void invite(const std::string& userId, const std::string& reason, SimpleCallback callback) = 0;
    virtual void kick(const std::string& userId, const std::string& reason, SimpleCallback callback) = 0;
    virtual void ban(const std::string& userId, const std::string& reason, SimpleCallback callback) = 0;
    virtual void unban(const std::string& userId, SimpleCallback callback) = 0;
    virtual void join(SimpleCallback callback) = 0;
    virtual void leave(SimpleCallback callback) = 0;
    virtual void forget(SimpleCallback callback) = 0;
    virtual void acceptInvite(SimpleCallback callback) = 0;
    virtual void rejectInvite(SimpleCallback callback) = 0;
    virtual bool isUserIgnored(const std::string& userId) const = 0;
    virtual void ignoreUser(const std::string& userId, SimpleCallback callback) = 0;
    virtual void unignoreUser(const std::string& userId, SimpleCallback callback) = 0;
};

// ── State Service ──
class StateService {
public:
    virtual ~StateService() = default;

    virtual std::string getStateEvent(const std::string& eventType, const std::string& stateKey = "") const = 0;
    virtual std::string getRoomName() const = 0;
    virtual std::string getRoomTopic() const = 0;
    virtual std::string getCanonicalAlias() const = 0;
    virtual std::vector<std::string> getAliases() const = 0;
    virtual std::string getRoomAvatarUrl() const = 0;
    virtual std::string getJoinRule() const = 0;
    virtual std::string getGuestAccess() const = 0;
    virtual std::string getHistoryVisibility() const = 0;
    virtual PowerLevels getPowerLevels() const = 0;
    virtual std::string getEncryptionAlgorithm() const = 0;
    virtual std::string getCreateEventContent() const = 0;
    virtual std::string getRoomVersion() const = 0;
    virtual std::string getPinnedEvents() const = 0;
    virtual std::string getServerAcl() const = 0;
    virtual std::string getRoomType() const = 0;
    virtual std::string getTombstoneContent() const = 0;
    virtual void setRoomName(const std::string& name, SimpleCallback callback) = 0;
    virtual void setRoomTopic(const std::string& topic, SimpleCallback callback) = 0;
    virtual void setCanonicalAlias(const std::string& alias, SimpleCallback callback) = 0;
    virtual void setRoomAvatar(const std::string& mxcUrl, SimpleCallback callback) = 0;
    virtual void setJoinRule(const std::string& rule, SimpleCallback callback) = 0;
    virtual void setGuestAccess(const std::string& access, SimpleCallback callback) = 0;
    virtual void setHistoryVisibility(const std::string& visibility, SimpleCallback callback) = 0;
    virtual void setPowerLevels(const PowerLevels& levels, SimpleCallback callback) = 0;
    virtual void enableEncryption(SimpleCallback callback) = 0;
    virtual void sendStateEvent(const std::string& eventType, const std::string& stateKey,
        const std::string& contentJson, SimpleCallback callback) = 0;
};

// ── Read Service ──
class ReadService {
public:
    virtual ~ReadService() = default;

    virtual void markAsRead(const std::string& eventId) = 0;
    virtual void markAllAsRead() = 0;
    virtual void setReadMarker(const std::string& eventId) = 0;
    virtual void setReadReceipt(const std::string& eventId) = 0;
    virtual std::string getReadMarkerEventId() const = 0;
    virtual std::string getReadReceiptEventId() const = 0;
    virtual int getUnreadCount() const = 0;
    virtual int getHighlightCount() const = 0;
    virtual int getNotificationCount() const = 0;
    virtual bool hasUnreadMessages() const = 0;
    virtual std::string getLastReadEventId() const = 0;
    virtual std::string getLastDisplayedEventId() const = 0;
};

// ── Typing Service ──
class TypingService {
public:
    virtual ~TypingService() = default;

    virtual void sendTyping(bool isTyping, int timeoutMs, SimpleCallback callback) = 0;
    virtual std::vector<std::string> getTypingUsers() const = 0;
    virtual int getTypingUsersCount() const = 0;
    virtual bool isUserTyping(const std::string& userId) const = 0;
    using TypingCallback = std::function<void(const std::vector<std::string>&)>;
    virtual void observeTypingUsers(TypingCallback callback) = 0;
};

// ── Uploads Service ──
class UploadsService {
public:
    using UploadCallback = std::function<void(bool, const std::string&, const std::string&, const std::string&)>;

    virtual ~UploadsService() = default;

    virtual void uploadFile(const std::string& localPath, const std::string& filename,
        const std::string& mimeType, ProgressCallback progress, UploadCallback callback) = 0;
    virtual void cancelUpload(const std::string& uploadId) = 0;
    virtual void cancelAllUploads() = 0;
    virtual int getPendingUploadCount() const = 0;
    virtual int getActiveUploadCount() const = 0;
};

// ── Draft Service ──
class DraftService {
public:
    virtual ~DraftService() = default;

    virtual void saveDraft(const std::string& text) = 0;
    virtual std::string getDraft() const = 0;
    virtual void deleteDraft() = 0;
    virtual bool hasDraft() const = 0;
    virtual void saveThreadDraft(const std::string& threadId, const std::string& text) = 0;
    virtual std::string getThreadDraft(const std::string& threadId) const = 0;
    virtual void deleteThreadDraft(const std::string& threadId) = 0;
};

// ── Room Push Rule Service ──
class RoomPushRuleService {
public:
    virtual ~RoomPushRuleService() = default;

    virtual RoomNotificationState getNotificationState() const = 0;
    virtual void setNotificationState(RoomNotificationState state, SimpleCallback callback) = 0;
    virtual bool isPushEnabled() const = 0;
    virtual void setPushEnabled(bool enabled, SimpleCallback callback) = 0;
};

// ── Tags Service ──
class TagsService {
public:
    virtual ~TagsService() = default;

    virtual std::vector<std::string> getTags() const = 0;
    virtual void setTag(const std::string& tag, double order, SimpleCallback callback) = 0;
    virtual void removeTag(const std::string& tag, SimpleCallback callback) = 0;
    virtual bool isFavourite() const = 0;
    virtual bool isLowPriority() const = 0;
    virtual void setFavourite(bool favourite, SimpleCallback callback) = 0;
    virtual void setLowPriority(bool lowPriority, SimpleCallback callback) = 0;
};

// ── Relation Service ──
class RelationService {
public:
    virtual ~RelationService() = default;

    virtual std::map<std::string, EventAnnotation> getAnnotations(const std::string& eventId) const = 0;
    virtual int getAnnotationCount(const std::string& eventId) const = 0;
    virtual void fetchAnnotations(const std::string& eventId, const std::string& key,
        std::function<void(bool, const std::string&, const std::map<std::string, EventAnnotation>&)> callback) = 0;
};

// ── Threads Service ──
class ThreadsService {
public:
    using ThreadCallback = std::function<void(bool, const std::string&, const std::vector<ThreadInfo>&)>;

    virtual ~ThreadsService() = default;

    virtual void fetchThreads(ThreadCallback callback) = 0;
    virtual std::vector<ThreadInfo> getCachedThreads() const = 0;
    virtual std::optional<ThreadInfo> getThread(const std::string& threadId) const = 0;
    virtual int getThreadCount() const = 0;
};

// ── Poll History Service ──
class PollHistoryService {
public:
    virtual ~PollHistoryService() = default;

    virtual std::vector<PollQuestion> getActivePolls() const = 0;
    virtual std::vector<PollQuestion> getEndedPolls() const = 0;
    virtual std::optional<PollQuestion> getPoll(const std::string& eventId) const = 0;
};

// ── Location Sharing Service ──
class LocationSharingService {
public:
    virtual ~LocationSharingService() = default;

    virtual void shareLocation(const LocationData& location, SimpleCallback callback) = 0;
    virtual void stopSharingLocation(SimpleCallback callback) = 0;
    virtual bool isSharingLocation() const = 0;
    virtual std::vector<BeaconInfo> getActiveBeacons() const = 0;
};

// ── Beacon Info Summary Service ──
class BeaconInfoSummaryService {
public:
    virtual ~BeaconInfoSummaryService() = default;

    virtual std::vector<BeaconInfo> getBeaconInfos() const = 0;
    virtual std::optional<BeaconInfo> getBeaconInfo(const std::string& eventId) const = 0;
    virtual bool hasActiveBeacons() const = 0;
};

// ── Room Call Service ──
class RoomCallService {
public:
    virtual ~RoomCallService() = default;

    virtual void placeCall(const std::string& callId, SimpleCallback callback) = 0;
    virtual void answerCall(const std::string& callId, SimpleCallback callback) = 0;
    virtual void hangupCall(const std::string& callId, SimpleCallback callback) = 0;
    virtual void rejectCall(const std::string& callId, SimpleCallback callback) = 0;
    virtual bool hasActiveCall(const std::string& roomId) const = 0;
    virtual bool hasActiveCalls() const = 0;
};

// ── Alias Service ──
class AliasService {
public:
    virtual ~AliasService() = default;

    virtual void addAlias(const std::string& alias, SimpleCallback callback) = 0;
    virtual void removeAlias(const std::string& alias, SimpleCallback callback) = 0;
    virtual void resolveAlias(const std::string& alias,
        std::function<void(bool, const std::string&, const std::string&, const std::vector<std::string>&)> callback) = 0;
    virtual std::vector<std::string> getLocalAliases() const = 0;
    virtual std::string getPublishedAlias() const = 0;
};

// ── Reporting Service ──
class ReportingService {
public:
    virtual ~ReportingService() = default;

    virtual void reportEvent(const std::string& eventId, int score, const std::string& reason, SimpleCallback callback) = 0;
    virtual void reportRoom(int score, const std::string& reason, SimpleCallback callback) = 0;
    virtual void reportUser(const std::string& userId, int score, const std::string& reason, SimpleCallback callback) = 0;
};

// ── Room Version Service ──
class RoomVersionService {
public:
    virtual ~RoomVersionService() = default;

    virtual std::string getRoomVersion() const = 0;
    virtual void upgradeRoom(const std::string& newVersion, SimpleCallback callback) = 0;
    virtual std::vector<std::string> getSupportedVersions() const = 0;
    virtual bool isVersionSupported(const std::string& version) const = 0;
    virtual bool canUpgrade() const = 0;
};

// ── Room Account Data Service ──
class RoomAccountDataService {
public:
    virtual ~RoomAccountDataService() = default;

    virtual void setAccountData(const std::string& type, const std::string& content, SimpleCallback callback) = 0;
    virtual std::string getAccountData(const std::string& type) const = 0;
    virtual std::map<std::string, std::string> getAllAccountData() const = 0;
};

// ── Threads Local Service ──
class ThreadsLocalService {
public:
    virtual ~ThreadsLocalService() = default;

    virtual void addLocalEcho(const std::string& threadId, const std::shared_ptr<TimelineEvent>& event) = 0;
    virtual std::vector<std::shared_ptr<TimelineEvent>> getLocalEchos(const std::string& threadId) const = 0;
    virtual void removeLocalEcho(const std::string& transactionId) = 0;
};

} // namespace matrix_sdk

#endif
