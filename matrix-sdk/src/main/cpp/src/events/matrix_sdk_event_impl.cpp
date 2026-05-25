#include "matrix_sdk/events/matrix_sdk_event.hpp"
#include <nlohmann/json.hpp>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <map>

namespace matrix_sdk {

using json = nlohmann::json;

// ── Internal helper: extract media info ──
static MediaInfo extractMediaInfo(const json& content) {
    MediaInfo info;
    if (content.contains("info")) {
        const auto& i = content["info"];
        info.mimeType = i.value("mimetype", "");
        info.width = i.value("w", 0);
        info.height = i.value("h", 0);
        info.size = i.value("size", 0);
        info.duration = i.value("duration", 0);
        if (i.contains("thumbnail_url"))
            info.thumbnailUrl = i["thumbnail_url"].get<std::string>();
        if (i.contains("thumbnail_info")) {
            info.thumbnailWidth = i["thumbnail_info"].value("w", 0);
            info.thumbnailHeight = i["thumbnail_info"].value("h", 0);
            info.thumbnailMimeType = i["thumbnail_info"].value("mimetype", "");
            info.thumbnailSize = i["thumbnail_info"].value("size", 0);
        }
        info.blurhash = i.value("xyz.amorgan.blurhash", "");
    }
    return info;
}

// ── Internal helper: extract message type ──
static MessageType extractMsgType(const json& content) {
    std::string msgtype = content.value("msgtype", "");
    return messageTypeFromString(msgtype);
}

// ── Internal helper: extract content ──
static EventContent extractContent(const json& content) {
    EventContent ec;
    ec.body = content.value("body", "");
    ec.formattedBody = content.value("formatted_body", "");
    ec.format = content.value("format", "");
    ec.msgType = extractMsgType(content);
    ec.url = content.value("url", "");
    ec.filename = content.value("filename", "");
    ec.info = extractMediaInfo(content);

    if (content.contains("geo_uri")) ec.geoUri = content["geo_uri"].get<std::string>();

    // Relation
    if (content.contains("m.relates_to")) {
        const auto& rel = content["m.relates_to"];
        ec.relatesToEventId = rel.value("event_id", "");
        std::string relType = rel.value("rel_type", "m.reference");
        if (relType == "m.thread") ec.relationType = RelationType::THREAD;
        else if (relType == "m.replace") ec.relationType = RelationType::REPLACE;
        else if (relType == "m.annotation") ec.relationType = RelationType::ANNOTATION;
        else ec.relationType = RelationType::REFERENCE;

        if (ec.relationType == RelationType::REPLACE && rel.contains("m.new_content")) {
            ec.newContent = rel["m.new_content"].dump();
        }
    }

    // Membership
    ec.membershipChange = content.value("membership", "");
    ec.displayName = content.value("displayname", "");
    ec.avatarUrl = content.value("avatar_url", "");
    ec.isDirect = content.value("is_direct", false);
    ec.reason = content.value("reason", "");
    ec.topic = content.value("topic", "");
    ec.name = content.value("name", "");

    // Call
    ec.callId = content.value("call_id", "");
    ec.sdp = content.value("sdp", "");
    ec.lifetime = content.value("lifetime", 0);
    ec.version = content.value("version", "");
    ec.offer = content.value("offer", "");
    ec.answer = content.value("answer", "");

    // Poll
    if (content.contains("org.matrix.msc3381.poll.start") ||
        content.contains("m.poll.start")) {
        const auto& poll = content.contains("org.matrix.msc3381.poll.start") ?
            content["org.matrix.msc3381.poll.start"] :
            content["m.poll.start"];
        ec.pollQuestion.question = poll["question"]["body"].value("text", "");
        ec.pollQuestion.maxSelections = poll.value("max_selections", 1);
        if (poll.contains("answers")) {
            for (const auto& ans : poll["answers"]) {
                ec.pollQuestion.answers.push_back(ans["org.matrix.msc1767.text"].get<std::string>());
            }
        }
    }

    return ec;
}

// ── TimelineEventImpl ──
class TimelineEventImpl : public TimelineEvent {
public:
    explicit TimelineEventImpl(const json& eventJson, bool isLocalEcho = false) {
        eventId_ = eventJson.value("event_id", "");
        roomId_ = eventJson.value("room_id", "");
        senderId_ = eventJson.value("sender", "");
        originServerTs_ = eventJson.value("origin_server_ts", 0);
        age_ = eventJson.value("age", 0);
        typeStr_ = eventJson.value("type", "");
        type_ = eventTypeFromString(typeStr_);
        stateKey_ = eventJson.value("state_key", "");

        if (eventJson.contains("content")) {
            contentJson_ = eventJson["content"].dump();
            content_ = extractContent(eventJson["content"]);
        }

        if (eventJson.contains("prev_content")) {
            prevContentJson_ = eventJson["prev_content"].dump();
        }

        if (eventJson.contains("unsigned")) {
            unsignedData_ = eventJson["unsigned"].dump();
        }

        redactsEventId_ = eventJson.value("redacts", "");
        isLocalEcho_ = isLocalEcho;
        transactionId_ = eventJson.value("txn_id", "");

        if (eventJson.contains("origin_server_ts")) {
            int64_t ts = eventJson["origin_server_ts"].get<int64_t>();
            int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            age_ = now - ts;
        }
    }

    // Accessors
    std::string getEventId() const override { return eventId_; }
    std::string getRoomId() const override { return roomId_; }
    std::string getSenderId() const override { return senderId_; }
    int64_t getOriginServerTs() const override { return originServerTs_; }
    int64_t getAge() const override { return age_; }
    EventType getType() const override { return type_; }
    std::string getTypeString() const override { return typeStr_; }
    std::string getStateKey() const override { return stateKey_; }
    std::string getContentJson() const override { return contentJson_; }
    std::string getPrevContentJson() const override { return prevContentJson_; }
    std::string getSenderName() const override { return senderName_; }
    std::string getSenderAvatarUrl() const override { return senderAvatarUrl_; }
    bool isEncrypted() const override { return type_ == EventType::ROOM_MESSAGE_ENCRYPTED; }
    bool isRedacted() const override { return !redactsEventId_.empty(); }
    bool isEditable() const override { return content_.msgType == MessageType::TEXT && type_ == EventType::ROOM_MESSAGE; }
    SendState getSendState() const override { return sendState_; }
    bool isLocalEcho() const override { return isLocalEcho_; }

    std::string getBody() const override { return content_.body; }
    std::string getFormattedBody() const override { return content_.formattedBody; }
    MessageType getMessageType() const override { return content_.msgType; }
    std::string getDecryptedContent() const override { return decryptedContent_; }
    std::string getDecryptedType() const override { return decryptedType_; }

    std::string getUrl() const override { return content_.url; }
    std::string getThumbnailUrl() const override { return content_.info.thumbnailUrl; }
    MediaInfo getMediaInfo() const override { return content_.info; }
    std::string getMimeType() const override { return content_.info.mimeType; }
    int64_t getFileSize() const override { return content_.info.size; }
    std::string getFileName() const override { return content_.filename; }

    std::optional<EventAnnotation> getBestAnnotation() const override {
        if (annotations_.empty()) return std::nullopt;
        auto best = std::max_element(annotations_.begin(), annotations_.end(),
            [](const auto& a, const auto& b) { return a.second.count < b.second.count; });
        return best->second;
    }

    std::map<std::string, EventAnnotation> getAllAnnotations() const override { return annotations_; }
    int getAnnotationCount() const override { return static_cast<int>(annotations_.size()); }

    int getAnnotationCountForKey(const std::string& key) const override {
        int count = 0;
        for (const auto& [k, v] : annotations_) {
            if (v.key == key) count++;
        }
        return count;
    }

    bool isThreadRoot() const override {
        return !threadRootId_.empty() && threadRootId_ == eventId_;
    }

    bool isThreadReply() const override {
        return !threadRootId_.empty() && threadRootId_ != eventId_;
    }

    std::string getThreadRootId() const override { return threadRootId_; }
    int getThreadReplyCount() const override { return threadReplyCount_; }

    bool isPoll() const override {
        return type_ == EventType::POLL_START || type_ == EventType::POLL_RESPONSE || type_ == EventType::POLL_END;
    }

    PollQuestion getPollData() const override { return content_.pollQuestion; }
    bool hasPollEnded() const override { return pollEnded_; }
    std::string getPollEndEventId() const override { return pollEndEventId_; }
    std::vector<int> getMyPollAnswer() const override { return myPollAnswer_; }

    bool isLocation() const override { return type_ == EventType::LOCATION; }
    LocationData getLocationData() const override { return content_.location; }

    bool isCallEvent() const override {
        return type_ == EventType::CALL_INVITE || type_ == EventType::CALL_CANDIDATES ||
               type_ == EventType::CALL_ANSWER || type_ == EventType::CALL_HANGUP ||
               type_ == EventType::CALL_REJECT || type_ == EventType::CALL_NEGOTIATE;
    }

    std::string getCallId() const override { return content_.callId; }

    bool isWidgetEvent() const override { return type_ == EventType::WIDGET; }
    Widget getWidgetData() const override { return widgetData_; }

    std::string getMembership() const override { return content_.membershipChange; }
    std::string getTargetUserId() const override { return stateKey_; }

    bool canBeReplaced() const override {
        return type_ == EventType::ROOM_MESSAGE && !isEncrypted() && !isRedacted();
    }

    std::string getReplacementEventId() const override { return replacementEventId_; }

    std::string toJson() const override {
        json j;
        j["event_id"] = eventId_;
        j["room_id"] = roomId_;
        j["sender"] = senderId_;
        j["origin_server_ts"] = originServerTs_;
        j["type"] = typeStr_;
        if (!stateKey_.empty()) j["state_key"] = stateKey_;
        if (!contentJson_.empty()) j["content"] = json::parse(contentJson_);
        if (!redactsEventId_.empty()) j["redacts"] = redactsEventId_;
        if (!unsignedData_.empty()) j["unsigned"] = json::parse(unsignedData_);
        return j.dump();
    }

    std::string toContentJson() const override { return contentJson_; }

    void setSendState(SendState state) override { sendState_ = state; }

    // Setters for internal use
    void setSenderName(const std::string& name) { senderName_ = name; }
    void setSenderAvatarUrl(const std::string& url) { senderAvatarUrl_ = url; }
    void setDecryptedContent(const std::string& content) { decryptedContent_ = content; }
    void setDecryptedType(const std::string& type) { decryptedType_ = type; }
    void setThreadRootId(const std::string& id) { threadRootId_ = id; }
    void setAnnotations(const std::map<std::string, EventAnnotation>& a) { annotations_ = a; }
    void setPollEnded(bool ended) { pollEnded_ = ended; }
    void setPollEndEventId(const std::string& id) { pollEndEventId_ = id; }
    void setMyPollAnswer(const std::vector<int>& ans) { myPollAnswer_ = ans; }
    void setWidgetData(const Widget& w) { widgetData_ = w; }
    void setReplacementEventId(const std::string& id) { replacementEventId_ = id; }

private:
    std::string eventId_;
    std::string roomId_;
    std::string senderId_;
    int64_t originServerTs_ = 0;
    int64_t age_ = 0;
    EventType type_ = EventType::UNKNOWN;
    std::string typeStr_;
    std::string stateKey_;
    std::string contentJson_;
    std::string prevContentJson_;
    std::string unsignedData_;
    std::string redactsEventId_;
    EventContent content_;
    std::string senderName_;
    std::string senderAvatarUrl_;
    std::string decryptedContent_;
    std::string decryptedType_;
    std::string threadRootId_;
    int threadReplyCount_ = 0;
    std::map<std::string, EventAnnotation> annotations_;
    bool pollEnded_ = false;
    std::string pollEndEventId_;
    std::vector<int> myPollAnswer_;
    Widget widgetData_;
    std::string replacementEventId_;
    SendState sendState_ = SendState::SENT;
    bool isLocalEcho_ = false;
    std::string transactionId_;
};

// ── EventServiceImpl ──
class EventServiceImpl : public EventService {
public:
    std::shared_ptr<TimelineEvent> createEvent(const CreateEventParams& params) override {
        json event;
        event["type"] = eventTypeToString(params.type);
        event["room_id"] = params.roomId;
        event["txn_id"] = params.transactionId;

        json content;
        content["body"] = params.content.body;
        content["msgtype"] = messageTypeToString(params.content.msgType);
        if (!params.content.formattedBody.empty()) {
            content["formatted_body"] = params.content.formattedBody;
            content["format"] = "org.matrix.custom.html";
        }
        if (!params.content.url.empty()) content["url"] = params.content.url;
        if (!params.content.filename.empty()) content["filename"] = params.content.filename;
        if (!params.content.relatesToEventId.empty()) {
            json rel;
            rel["event_id"] = params.content.relatesToEventId;
            rel["rel_type"] = "m.reference";
            content["m.relates_to"] = rel;
        }

        event["content"] = content;
        return std::make_shared<TimelineEventImpl>(event, true);
    }

    std::string createContentJson(const std::string& body, MessageType type,
        const std::optional<std::string>& formattedBody,
        const std::optional<std::string>& url,
        const std::optional<MediaInfo>& info) override {
        json content;
        content["body"] = body;
        content["msgtype"] = messageTypeToString(type);
        if (formattedBody.has_value()) {
            content["formatted_body"] = formattedBody.value();
            content["format"] = "org.matrix.custom.html";
        }
        if (url.has_value()) content["url"] = url.value();
        if (info.has_value()) {
            json jinfo;
            jinfo["mimetype"] = info->mimeType;
            jinfo["size"] = info->size;
            jinfo["w"] = info->width;
            jinfo["h"] = info->height;
            content["info"] = jinfo;
        }
        return content.dump();
    }

    std::shared_ptr<TimelineEvent> parseEvent(const std::string& jsonEvent) override {
        try {
            return std::make_shared<TimelineEventImpl>(json::parse(jsonEvent));
        } catch (...) {
            return nullptr;
        }
    }

    std::vector<std::shared_ptr<TimelineEvent>> parseEvents(const std::string& jsonEvents) override {
        std::vector<std::shared_ptr<TimelineEvent>> events;
        try {
            auto j = json::parse(jsonEvents);
            if (j.is_array()) {
                for (const auto& ev : j) {
                    events.push_back(std::make_shared<TimelineEventImpl>(ev));
                }
            } else if (j.contains("chunk") && j["chunk"].is_array()) {
                for (const auto& ev : j["chunk"]) {
                    events.push_back(std::make_shared<TimelineEventImpl>(ev));
                }
            }
        } catch (...) {}
        return events;
    }

    std::string getEventStringType(const std::string& jsonEvent) const override {
        try {
            auto j = json::parse(jsonEvent);
            return j.value("type", "");
        } catch (...) {
            return "";
        }
    }

    bool isValidEvent(const std::string& jsonEvent) const override {
        try {
            auto j = json::parse(jsonEvent);
            return j.contains("type") && j.contains("event_id") && j.contains("sender");
        } catch (...) {
            return false;
        }
    }

    bool isStateEvent(const std::string& jsonEvent) const override {
        try {
            auto j = json::parse(jsonEvent);
            return j.contains("state_key");
        } catch (...) {
            return false;
        }
    }

    bool isEncryptedEvent(const std::string& jsonEvent) const override {
        try {
            auto j = json::parse(jsonEvent);
            return j.value("type", "") == "m.room.encrypted";
        } catch (...) {
            return false;
        }
    }

    std::string stripUnsupportedFormats(const std::string& formatted) const override {
        std::string result = formatted;
        // Strip any HTML tags that might be unsupported
        // Simple implementation: keep basic HTML
        return result;
    }

    std::string formatMessage(const std::string& body, const std::string& format) const override {
        if (format == "org.matrix.custom.html") {
            return "<p>" + body + "</p>";
        }
        return body;
    }

    std::string getPermalink(const std::string& roomId, const std::string& eventId) const override {
        return "https://matrix.to/#/" + roomId + "/" + eventId;
    }

    void getEventById(const std::string& roomId, const std::string& eventId,
        std::function<void(bool, const std::string&, const std::shared_ptr<TimelineEvent>&)> callback) override {
        // This would normally query the server — for now, return not found
        callback(false, "Not implemented in event service", nullptr);
    }
};

} // namespace matrix_sdk
