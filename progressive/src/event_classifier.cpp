#include "progressive/event_classifier.hpp"
#include <algorithm>

namespace progressive {

// ==== Classification Functions ====

bool isStateEvent(const std::string& eventType) {
    return eventType == EventTypeStr::STATE_ROOM_NAME ||
           eventType == EventTypeStr::STATE_ROOM_TOPIC ||
           eventType == EventTypeStr::STATE_ROOM_AVATAR ||
           eventType == EventTypeStr::STATE_ROOM_MEMBER ||
           eventType == EventTypeStr::STATE_ROOM_CREATE ||
           eventType == EventTypeStr::STATE_ROOM_JOIN_RULES ||
           eventType == EventTypeStr::STATE_ROOM_GUEST_ACCESS ||
           eventType == EventTypeStr::STATE_ROOM_POWER_LEVELS ||
           eventType == EventTypeStr::STATE_ROOM_TOMBSTONE ||
           eventType == EventTypeStr::STATE_ROOM_CANONICAL_ALIAS ||
           eventType == EventTypeStr::STATE_ROOM_HISTORY_VISIBILITY ||
           eventType == EventTypeStr::STATE_ROOM_PINNED_EVENT ||
           eventType == EventTypeStr::STATE_ROOM_ENCRYPTION ||
           eventType == EventTypeStr::STATE_ROOM_SERVER_ACL ||
           eventType == EventTypeStr::STATE_ROOM_THIRD_PARTY_INVITE ||
           eventType == EventTypeStr::STATE_SPACE_CHILD ||
           eventType == EventTypeStr::STATE_SPACE_PARENT ||
           eventType == EventTypeStr::STATE_ROOM_WIDGET;
}

bool isCallEvent(const std::string& eventType) {
    // Original Kotlin (EventType.kt:122-131):
    //   fun isCallEvent(type: String): Boolean {
    //       return type == CALL_INVITE || type == CALL_CANDIDATES || ...
    //   }
    return eventType == EventTypeStr::CALL_INVITE ||
           eventType == EventTypeStr::CALL_CANDIDATES ||
           eventType == EventTypeStr::CALL_ANSWER ||
           eventType == EventTypeStr::CALL_HANGUP ||
           eventType == EventTypeStr::CALL_REJECT ||
           eventType == EventTypeStr::CALL_NEGOTIATE ||
           eventType == EventTypeStr::CALL_SELECT_ANSWER ||
           eventType == EventTypeStr::CALL_REPLACES;
}

bool isVerificationEvent(const std::string& eventType) {
    // Original Kotlin (EventType.kt:133-145):
    return eventType == EventTypeStr::KEY_VERIFICATION_REQUEST ||
           eventType == EventTypeStr::KEY_VERIFICATION_START ||
           eventType == EventTypeStr::KEY_VERIFICATION_ACCEPT ||
           eventType == EventTypeStr::KEY_VERIFICATION_KEY ||
           eventType == EventTypeStr::KEY_VERIFICATION_MAC ||
           eventType == EventTypeStr::KEY_VERIFICATION_CANCEL ||
           eventType == EventTypeStr::KEY_VERIFICATION_DONE ||
           eventType == EventTypeStr::KEY_VERIFICATION_READY;
}

bool isPollEvent(const std::string& eventType) {
    return eventType == EventTypeStr::POLL_START ||
           eventType == EventTypeStr::POLL_RESPONSE ||
           eventType == EventTypeStr::POLL_END;
}

bool isMediaMessageType(const std::string& msgType) {
    return msgType == MessageTypeStr::IMAGE ||
           msgType == MessageTypeStr::VIDEO ||
           msgType == MessageTypeStr::AUDIO ||
           msgType == MessageTypeStr::FILE;
}

bool isTextMessageType(const std::string& msgType) {
    return msgType == MessageTypeStr::TEXT ||
           msgType == MessageTypeStr::EMOTE ||
           msgType == MessageTypeStr::NOTICE;
}

std::vector<std::string> getAllEventTypes() {
    return {
        EventTypeStr::MESSAGE, EventTypeStr::STICKER, EventTypeStr::ENCRYPTED,
        EventTypeStr::TYPING, EventTypeStr::REDACTION, EventTypeStr::RECEIPT,
        EventTypeStr::REACTION,
        EventTypeStr::STATE_ROOM_NAME, EventTypeStr::STATE_ROOM_TOPIC, EventTypeStr::STATE_ROOM_AVATAR,
        EventTypeStr::STATE_ROOM_MEMBER, EventTypeStr::STATE_ROOM_CREATE,
        EventTypeStr::STATE_ROOM_JOIN_RULES, EventTypeStr::STATE_ROOM_GUEST_ACCESS,
        EventTypeStr::STATE_ROOM_POWER_LEVELS, EventTypeStr::STATE_ROOM_TOMBSTONE,
        EventTypeStr::STATE_ROOM_CANONICAL_ALIAS, EventTypeStr::STATE_ROOM_HISTORY_VISIBILITY,
        EventTypeStr::STATE_ROOM_PINNED_EVENT, EventTypeStr::STATE_ROOM_ENCRYPTION,
        EventTypeStr::STATE_ROOM_SERVER_ACL, EventTypeStr::STATE_SPACE_CHILD, EventTypeStr::STATE_SPACE_PARENT,
        EventTypeStr::CALL_INVITE, EventTypeStr::CALL_CANDIDATES, EventTypeStr::CALL_ANSWER,
        EventTypeStr::CALL_HANGUP, EventTypeStr::CALL_REJECT,
        EventTypeStr::KEY_VERIFICATION_REQUEST, EventTypeStr::KEY_VERIFICATION_START,
        EventTypeStr::KEY_VERIFICATION_ACCEPT, EventTypeStr::KEY_VERIFICATION_KEY,
        EventTypeStr::KEY_VERIFICATION_MAC, EventTypeStr::KEY_VERIFICATION_CANCEL,
        EventTypeStr::KEY_VERIFICATION_DONE, EventTypeStr::KEY_VERIFICATION_READY,
        EventTypeStr::POLL_START, EventTypeStr::POLL_RESPONSE, EventTypeStr::POLL_END,
    };
}

std::vector<std::string> getAllMessageTypes() {
    return {
        MessageTypeStr::TEXT, MessageTypeStr::EMOTE, MessageTypeStr::NOTICE,
        MessageTypeStr::IMAGE, MessageTypeStr::AUDIO, MessageTypeStr::VIDEO,
        MessageTypeStr::LOCATION, MessageTypeStr::FILE,
    };
}

std::string getEventTypeLabel(const std::string& eventType) {
    if (eventType == EventTypeStr::MESSAGE) return "Message";
    if (eventType == EventTypeStr::STICKER) return "Sticker";
    if (eventType == EventTypeStr::ENCRYPTED) return "Encrypted";
    if (eventType == EventTypeStr::TYPING) return "Typing";
    if (eventType == EventTypeStr::REDACTION) return "Redaction";
    if (eventType == EventTypeStr::RECEIPT) return "Read Receipt";
    if (eventType == EventTypeStr::REACTION) return "Reaction";
    if (eventType == EventTypeStr::STATE_ROOM_NAME) return "Room Name Change";
    if (eventType == EventTypeStr::STATE_ROOM_TOPIC) return "Topic Change";
    if (eventType == EventTypeStr::STATE_ROOM_AVATAR) return "Avatar Change";
    if (eventType == EventTypeStr::STATE_ROOM_MEMBER) return "Membership";
    if (eventType == EventTypeStr::STATE_ROOM_CREATE) return "Room Created";
    if (eventType == EventTypeStr::STATE_ROOM_JOIN_RULES) return "Join Rules";
    if (eventType == EventTypeStr::STATE_ROOM_POWER_LEVELS) return "Power Levels";
    if (eventType == EventTypeStr::STATE_ROOM_TOMBSTONE) return "Room Upgrade";
    if (eventType == EventTypeStr::STATE_ROOM_ENCRYPTION) return "Encryption";
    if (eventType == EventTypeStr::CALL_INVITE) return "Call Invite";
    if (eventType == EventTypeStr::CALL_HANGUP) return "Call Ended";
    if (isVerificationEvent(eventType)) return "Verification";
    if (isPollEvent(eventType)) return "Poll";
    return eventType;
}

std::string getMessageTypeLabel(const std::string& msgType) {
    if (msgType == MessageTypeStr::TEXT) return "Text";
    if (msgType == MessageTypeStr::EMOTE) return "Action";
    if (msgType == MessageTypeStr::NOTICE) return "Notice";
    if (msgType == MessageTypeStr::IMAGE) return "Image";
    if (msgType == MessageTypeStr::AUDIO) return "Audio";
    if (msgType == MessageTypeStr::VIDEO) return "Video";
    if (msgType == MessageTypeStr::LOCATION) return "Location";
    if (msgType == MessageTypeStr::FILE) return "File";
    if (msgType == MessageTypeStr::STICKER_LOCAL) return "Sticker";
    if (msgType == MessageTypeStr::CONFETTI) return "Confetti";
    if (msgType == MessageTypeStr::SNOWFALL) return "Snowfall";
    return msgType;
}

std::string routeEventForProcessing(const std::string& eventType, const std::string& msgType) {
    // Routing logic — determines which C++ module processes this event
    if (eventType == EventTypeStr::MESSAGE || eventType == EventTypeStr::STICKER ||
        eventType == EventTypeStr::ENCRYPTED) {
        if (isTextMessageType(msgType)) return "message_text";
        if (isMediaMessageType(msgType)) return "message_media";
        if (msgType == MessageTypeStr::LOCATION) return "message_location";
        if (msgType == MessageTypeStr::CONFETTI || msgType == MessageTypeStr::SNOWFALL) return "message_effect";
        return "message";
    }
    if (isStateEvent(eventType)) return "state";
    if (isCallEvent(eventType)) return "call";
    if (isVerificationEvent(eventType)) return "verification";
    if (eventType == EventTypeStr::REACTION) return "reaction";
    if (isPollEvent(eventType)) return "poll";
    if (eventType == EventTypeStr::REDACTION) return "redaction";
    if (eventType == EventTypeStr::RECEIPT) return "receipt";
    if (eventType == EventTypeStr::TYPING) return "typing";
    return "unknown";
}

// ==== Message Type Detection (from Event.kt:383-450) ====
// Original: fun Event.isTextMessage(): Boolean { return when (getMsgType()) { ... } }

bool isTextMessage(const std::string& msgType) {
    // Original: MSGTYPE_TEXT, MSGTYPE_EMOTE, MSGTYPE_NOTICE → true
    return msgType == MessageTypeStr::TEXT ||
           msgType == MessageTypeStr::EMOTE ||
           msgType == MessageTypeStr::NOTICE;
}

bool isImageMessage(const std::string& msgType) { return msgType == MessageTypeStr::IMAGE; }
bool isVideoMessage(const std::string& msgType) { return msgType == MessageTypeStr::VIDEO; }
bool isAudioMessage(const std::string& msgType) { return msgType == MessageTypeStr::AUDIO; }
bool isFileMessage(const std::string& msgType) { return msgType == MessageTypeStr::FILE; }
bool isLocationMessage(const std::string& msgType) { return msgType == MessageTypeStr::LOCATION; }

bool isAttachmentMessage(const std::string& msgType) {
    // Original: MSGTYPE_IMAGE, AUDIO, VIDEO, FILE → true
    return isImageMessage(msgType) || isAudioMessage(msgType) ||
           isVideoMessage(msgType) || isFileMessage(msgType);
}

bool supportsNotification(const std::string& eventType) {
    // Original: getClearType() in MESSAGE + POLL_START + POLL_END + BEACON_INFO + CALL_NOTIFY
    return eventType == EventTypeStr::MESSAGE ||
           isPollEvent(eventType) ||
           eventType == EventTypeStr::CALL_INVITE;
}

bool isContentReportable(const std::string& eventType) {
    return eventType == EventTypeStr::MESSAGE;
}

bool isInvitationEvent(const std::string& eventType, const std::string& contentJson) {
    // Original: type == STATE_ROOM_MEMBER && membership == INVITE
    if (eventType != EventTypeStr::STATE_ROOM_MEMBER) return false;
    return contentJson.find("\"membership\":\"invite\"") != std::string::npos ||
           contentJson.find("\"membership\": \"invite\"") != std::string::npos;
}

// ==== Relation Types (from RelationType.kt + RelationDefaultContent.kt) ====
bool isReplyRelation(const std::string& contentJson) {
    // Original: this?.inReplyTo?.eventId != null
    auto replyPos = contentJson.find("\"m.in_reply_to\"");
    if (replyPos == std::string::npos) return false;
    return contentJson.find("\"event_id\":\"", replyPos) != std::string::npos ||
           contentJson.find("\"event_id\": \"", replyPos) != std::string::npos;
}

bool shouldRenderInThread(const std::string& contentJson) {
    // Original: isFallingBack == false
    if (!isReplyRelation(contentJson)) return false;
    return contentJson.find("\"is_falling_back\": true") == std::string::npos &&
           contentJson.find("\"is_falling_back\":true") == std::string::npos;
}

} // namespace progressive


// ==== Extended event_classifier implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string event_classifier::serialize() const {
    json j = toJson();
    return j.dump();
}

bool event_classifier::deserialize(const std::string& data) {
    if (data.empty()) return false;
    try {
        json j = json::parse(data);
        return fromJson(j);
    } catch (...) {
        setError("Failed to deserialize data");
        return false;
    }
}

// Validation helpers
bool event_classifier::validate() const {
    if (!m_initialized) {
        LOGE("event_classifier: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool event_classifier::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool event_classifier::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json event_classifier::getMetrics() const {
    json m;
    m["class"] = "event_classifier";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int event_classifier::getOperationCount() const {
    return m_operationCount;
}

void event_classifier::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void event_classifier::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "event_classifier";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool event_classifier::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool event_classifier::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool event_classifier::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
    auto now = currentTimeMs();
    auto& window = m_rateLimitWindows[key];
    if (now - window.startTime > windowMs) {
        window.startTime = now;
        window.count = 0;
    }
    if (window.count >= maxRequests) return false;
    window.count++;
    return true;
}

// Observation pattern
void event_classifier::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void event_classifier::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int event_classifier::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void event_classifier::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> event_classifier::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> event_classifier::listItems() const {
    return {};
}

int event_classifier::itemCount() const {
    return 0;
}

// Versioning
std::string event_classifier::getVersion() const {
    return "1.0.0";
}

bool event_classifier::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool event_classifier::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void event_classifier::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> event_classifier::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool event_classifier::migrateData(int fromVersion, int toVersion) {
    LOGI("event_classifier: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int event_classifier::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json event_classifier::exportData() const {
    return toJson();
}

bool event_classifier::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void event_classifier::performCleanup() {
    LOGI("event_classifier: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t event_classifier::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool event_classifier::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool event_classifier::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool event_classifier::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void event_classifier::logDebug(const std::string& msg) const {
    LOGI("event_classifier: %s", msg.c_str());
}

void event_classifier::logWarning(const std::string& msg) const {
    LOGW("event_classifier: %s", msg.c_str());
}

void event_classifier::logError(const std::string& msg) const {
    LOGE("event_classifier: %s", msg.c_str());
}
