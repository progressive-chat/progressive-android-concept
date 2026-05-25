#ifndef MATRIX_SDK_ROOM_HPP
#define MATRIX_SDK_ROOM_HPP

#include "matrix_sdk_types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <optional>

namespace matrix_sdk {

// Forward declarations for room services
class RoomAccountDataService;
class AliasService;
class RoomCallService;
class RoomCryptoService;
class LocationSharingService;
class MembershipService;
class RoomSummary;
class LocalRoomSummary;
class RelationService;
class RoomPushRuleService;
class PollHistoryService;
class ReadService;
class ReportingService;
class DraftService;
class SendService;
class StateService;
class TagsService;
class ThreadsService;
class ThreadsLocalService;
class TimelineService;
class TypingService;
class UploadsService;
class RoomVersionService;
class TimelineEvent;
class BeaconInfoSummaryService;

// ── Room Summary ──
class RoomSummary {
public:
    virtual ~RoomSummary() = default;
    virtual std::string getRoomId() const = 0;
    virtual std::string getDisplayName() const = 0;
    virtual std::string getTopic() const = 0;
    virtual std::string getCanonicalAlias() const = 0;
    virtual std::string getAvatarUrl() const = 0;
    virtual std::string getLastMessageBody() const = 0;
    virtual std::string getLastMessageSenderId() const = 0;
    virtual std::string getLastMessageSenderName() const = 0;
    virtual int64_t getLastMessageTimestamp() const = 0;
    virtual int getJoinedMembersCount() const = 0;
    virtual int getInvitedMembersCount() const = 0;
    virtual int getNotificationCount() const = 0;
    virtual int getHighlightCount() const = 0;
    virtual bool isDirect() const = 0;
    virtual bool isEncrypted() const = 0;
    virtual bool isPublic() const = 0;
    virtual bool hasUnreadMessages() const = 0;
    virtual bool isSpace() const = 0;
    virtual bool isFavourite() const = 0;
    virtual bool isLowPriority() const = 0;
    virtual bool isServerNotice() const = 0;
    virtual std::string getRoomType() const = 0;
    virtual std::string getMembership() const = 0;
    virtual RoomNotificationState getNotificationState() const = 0;
    virtual std::string getLatestPreviewableEvent() const = 0;
};

// ── Local Room Summary ──
class LocalRoomSummary {
public:
    virtual ~LocalRoomSummary() = default;
    virtual std::string getRoomId() const = 0;
    virtual std::string getLocalName() const = 0;
    virtual std::string getLocalTopic() const = 0;
    virtual std::string getLocalAvatarPath() const = 0;
    virtual bool isLocallyModified() const = 0;
};

// ── Room ──
class Room {
public:
    virtual ~Room() = default;

    // ── Identity ──
    virtual std::string getRoomId() const = 0;

    // ── Summary ──
    virtual std::shared_ptr<RoomSummary> roomSummary() const = 0;
    virtual std::shared_ptr<LocalRoomSummary> localRoomSummary() const = 0;
    virtual void refreshSummary() = 0;

    // ── Live summaries for observation ──
    using SummaryCallback = std::function<void(const std::shared_ptr<RoomSummary>&)>;
    virtual void observeRoomSummary(SummaryCallback callback) = 0;
    virtual void observeLocalRoomSummary(std::function<void(const std::shared_ptr<LocalRoomSummary>&)> callback) = 0;

    // ── Services ──
    virtual std::shared_ptr<TimelineService> timelineService() = 0;
    virtual std::shared_ptr<SendService> sendService() = 0;
    virtual std::shared_ptr<StateService> stateService() = 0;
    virtual std::shared_ptr<MembershipService> membershipService() = 0;
    virtual std::shared_ptr<ReadService> readService() = 0;
    virtual std::shared_ptr<TypingService> typingService() = 0;
    virtual std::shared_ptr<UploadsService> uploadsService() = 0;
    virtual std::shared_ptr<DraftService> draftService() = 0;
    virtual std::shared_ptr<RoomPushRuleService> pushRuleService() = 0;
    virtual std::shared_ptr<TagsService> tagsService() = 0;
    virtual std::shared_ptr<RelationService> relationService() = 0;
    virtual std::shared_ptr<ThreadsService> threadsService() = 0;
    virtual std::shared_ptr<ThreadsLocalService> threadsLocalService() = 0;
    virtual std::shared_ptr<PollHistoryService> pollHistoryService() = 0;
    virtual std::shared_ptr<RoomCryptoService> cryptoService() = 0;
    virtual std::shared_ptr<RoomCallService> callService() = 0;
    virtual std::shared_ptr<LocationSharingService> locationSharingService() = 0;
    virtual std::shared_ptr<BeaconInfoSummaryService> beaconInfoSummaryService() = 0;
    virtual std::shared_ptr<AliasService> aliasService() = 0;
    virtual std::shared_ptr<ReportingService> reportingService() = 0;
    virtual std::shared_ptr<RoomVersionService> versionService() = 0;
    virtual std::shared_ptr<RoomAccountDataService> accountDataService() = 0;

    // ── Actions ──
    virtual void leave(SimpleCallback callback) = 0;
    virtual void forget(SimpleCallback callback) = 0;
    virtual void join(SimpleCallback callback) = 0;
    virtual void markAsRead(const std::string& eventId, SimpleCallback callback) = 0;
    virtual void setDisplayName(const std::string& name, SimpleCallback callback) = 0;
    virtual void setTopic(const std::string& topic, SimpleCallback callback) = 0;
    virtual void setAvatar(const std::string& uri, const std::string& filename, SimpleCallback callback) = 0;
    virtual void updateAvatarUrl(const std::string& newUrl, SimpleCallback callback) = 0;
    virtual void invite(const std::string& userId, SimpleCallback callback) = 0;
    virtual void invite3pid(const std::string& address, const std::string& medium, SimpleCallback callback) = 0;
    virtual void kick(const std::string& userId, const std::string& reason, SimpleCallback callback) = 0;
    virtual void ban(const std::string& userId, const std::string& reason, SimpleCallback callback) = 0;
    virtual void unban(const std::string& userId, SimpleCallback callback) = 0;
    virtual void redactEvent(const std::string& eventId, const std::string& reason, SimpleCallback callback) = 0;
    virtual void reportContent(const std::string& eventId, int score, const std::string& reason, SimpleCallback callback) = 0;
    virtual void ignoreUser(const std::string& userId, SimpleCallback callback) = 0;
    virtual void unignoreUser(const std::string& userId, SimpleCallback callback) = 0;
    virtual void upgradeRoom(const std::string& newVersion, SimpleCallback callback) = 0;

    // ── Events ──
    virtual void fetchRoomMembers(std::function<void(bool, const std::string&, const std::vector<RoomMember>&)> callback) = 0;
    virtual void fetchRoomInfo(std::function<void(bool, const std::string&, const RoomInfo&)> callback) = 0;

    // ── Crypto ──
    virtual bool isEncrypted() const = 0;
    virtual void enableEncryption(SimpleCallback callback) = 0;
    virtual void encryptionEventContent(std::string& algo, std::string& rotationPeriodMs, int& rotationPeriodMsgs) const = 0;
};

} // namespace matrix_sdk

#endif
