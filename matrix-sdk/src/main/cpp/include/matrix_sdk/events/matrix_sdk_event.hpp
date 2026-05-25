#ifndef MATRIX_SDK_EVENT_HPP
#define MATRIX_SDK_EVENT_HPP

#include "matrix_sdk_types.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>

namespace matrix_sdk {

// ── Event Content Base ──
struct EventContent {
    std::string body;
    std::string formattedBody;
    std::string format;  // "org.matrix.custom.html"
    MessageType msgType = MessageType::TEXT;
    std::string url;
    std::string filename;
    MediaInfo info;
    std::string geoUri;
    LocationData location;
    std::string relatesToEventId;
    RelationType relationType = RelationType::REFERENCE;
    std::string newContent;
    std::string membershipChange;
    std::string displayName;
    std::string avatarUrl;
    std::string thirdPartyInvite;
    bool isDirect = false;
    std::string reason;
    std::string topic;
    std::string name;
    std::string canonicalAlias;
    PowerLevels powerLevels;
    std::string joinRule;
    std::string guestAccess;
    std::string historyVisibility;
    std::string tombstoneBody;
    std::string replacementRoom;
    std::string encryptionAlgorithm;
    int rotationPeriodMs = 0;
    int rotationPeriodMsgs = 0;
    std::string callId;
    std::string sdp;
    int lifetime = 0;
    std::string version;
    std::string offer;
    std::string answer;
    std::string candidates;
    std::string hangupReason;
    PollQuestion pollQuestion;
    std::vector<PollResponse> pollResponses;
    std::vector<std::string> pinnedEventIds;
    std::map<std::string, std::string> unsignedData;
    std::string beaconInfoId;
    bool beaconLive = true;
    int64_t beaconTimeout = 0;
    std::string serverAclAllow;
    std::string serverAclDeny;
    bool serverAclAllowIpLiterals = false;
};

// ── Unsigned Data ──
struct UnsignedData {
    int age = 0;
    std::string prevContent;
    std::string redactedBecause;
    std::string transactionId;
    std::string replacesState;
    std::map<std::string, EventAnnotation> annotations;
};

// ── Timeline Event ──
class TimelineEvent {
public:
    virtual ~TimelineEvent() = default;

    virtual std::string getEventId() const = 0;
    virtual std::string getRoomId() const = 0;
    virtual std::string getSenderId() const = 0;
    virtual int64_t getOriginServerTs() const = 0;
    virtual int64_t getAge() const = 0;
    virtual EventType getType() const = 0;
    virtual std::string getTypeString() const = 0;
    virtual std::string getStateKey() const = 0;
    virtual std::string getContentJson() const = 0;
    virtual std::string getPrevContentJson() const = 0;

    virtual std::string getSenderName() const = 0;
    virtual std::string getSenderAvatarUrl() const = 0;
    virtual bool isEncrypted() const = 0;
    virtual bool isRedacted() const = 0;
    virtual bool isEditable() const = 0;
    virtual SendState getSendState() const = 0;
    virtual bool isLocalEcho() const = 0;

    virtual std::string getBody() const = 0;
    virtual std::string getFormattedBody() const = 0;
    virtual MessageType getMessageType() const = 0;
    virtual std::string getDecryptedContent() const = 0;
    virtual std::string getDecryptedType() const = 0;

    virtual std::string getUrl() const = 0;
    virtual std::string getThumbnailUrl() const = 0;
    virtual MediaInfo getMediaInfo() const = 0;
    virtual std::string getMimeType() const = 0;
    virtual int64_t getFileSize() const = 0;
    virtual std::string getFileName() const = 0;

    virtual std::optional<EventAnnotation> getBestAnnotation() const = 0;
    virtual std::map<std::string, EventAnnotation> getAllAnnotations() const = 0;
    virtual int getAnnotationCount() const = 0;
    virtual int getAnnotationCountForKey(const std::string& key) const = 0;

    virtual bool isThreadRoot() const = 0;
    virtual bool isThreadReply() const = 0;
    virtual std::string getThreadRootId() const = 0;
    virtual int getThreadReplyCount() const = 0;

    virtual bool isPoll() const = 0;
    virtual PollQuestion getPollData() const = 0;
    virtual bool hasPollEnded() const = 0;
    virtual std::string getPollEndEventId() const = 0;
    virtual std::vector<int> getMyPollAnswer() const = 0;

    virtual bool isLocation() const = 0;
    virtual LocationData getLocationData() const = 0;

    virtual bool isCallEvent() const = 0;
    virtual std::string getCallId() const = 0;

    virtual bool isWidgetEvent() const = 0;
    virtual Widget getWidgetData() const = 0;

    virtual std::string getMembership() const = 0;
    virtual std::string getTargetUserId() const = 0;

    virtual bool canBeReplaced() const = 0;
    virtual std::string getReplacementEventId() const = 0;

    virtual std::string toJson() const = 0;
    virtual std::string toContentJson() const = 0;
    virtual void setSendState(SendState state) = 0;
};

// ── Create Event ──
struct CreateEventParams {
    std::string roomId;
    EventType type;
    EventContent content;
    std::string transactionId;
    std::optional<std::string> threadId;
    std::optional<std::string> relatesToEventId;
    RelationType relationType = RelationType::REFERENCE;
    SendState initialState = SendState::UNSENT;
};

// ── Event Service ──
class EventService {
public:
    virtual ~EventService() = default;

    virtual std::shared_ptr<TimelineEvent> createEvent(const CreateEventParams& params) = 0;
    virtual std::string createContentJson(const std::string& body, MessageType type,
        const std::optional<std::string>& formattedBody = std::nullopt,
        const std::optional<std::string>& url = std::nullopt,
        const std::optional<MediaInfo>& info = std::nullopt) = 0;

    virtual std::shared_ptr<TimelineEvent> parseEvent(const std::string& jsonEvent) = 0;
    virtual std::vector<std::shared_ptr<TimelineEvent>> parseEvents(const std::string& jsonEvents) = 0;

    virtual std::string getEventStringType(const std::string& jsonEvent) const = 0;
    virtual bool isValidEvent(const std::string& jsonEvent) const = 0;
    virtual bool isStateEvent(const std::string& jsonEvent) const = 0;
    virtual bool isEncryptedEvent(const std::string& jsonEvent) const = 0;

    virtual std::string stripUnsupportedFormats(const std::string& formatted) const = 0;
    virtual std::string formatMessage(const std::string& body, const std::string& format) const = 0;
    virtual std::string getPermalink(const std::string& roomId, const std::string& eventId) const = 0;

    virtual void getEventById(const std::string& roomId, const std::string& eventId,
        std::function<void(bool, const std::string&, const std::shared_ptr<TimelineEvent>&)> callback) = 0;
};

} // namespace matrix_sdk

#endif
