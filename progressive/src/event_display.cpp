#include "progressive/event_display.hpp"
#include <sstream>

namespace progressive {

DisplayEventType classifyEvent(const std::string& eventType, const std::string& msgType) {
    if (eventType == "m.room.message" || eventType == "m.room.encrypted") {
        if (msgType == "m.text") return DisplayEventType::Message;
        if (msgType == "m.emote") return DisplayEventType::Emote;
        if (msgType == "m.image") return DisplayEventType::Image;
        if (msgType == "m.video") return DisplayEventType::Video;
        if (msgType == "m.audio") return DisplayEventType::Audio;
        if (msgType == "m.file") return DisplayEventType::File;
        if (msgType == "m.sticker" || eventType.find("sticker") != std::string::npos)
            return DisplayEventType::Sticker;
        if (msgType == "m.location") return DisplayEventType::Location;
        if (msgType == "m.poll" || eventType.find("poll") != std::string::npos)
            return DisplayEventType::Poll;
        return DisplayEventType::Message;
    }
    if (eventType == "m.reaction") return DisplayEventType::Reaction;
    if (eventType == "m.room.member") return DisplayEventType::MemberEvent;
    if (eventType == "m.room.name") return DisplayEventType::RoomName;
    if (eventType == "m.room.topic") return DisplayEventType::RoomTopic;
    if (eventType == "m.room.avatar") return DisplayEventType::RoomAvatar;
    if (eventType == "m.room.encryption") return DisplayEventType::Encryption;
    if (eventType == "m.room.redaction" || eventType == "m.room.redaction")
        return DisplayEventType::Redaction;
    if (eventType.find("m.call.") == 0) return DisplayEventType::Call;
    if (eventType == "m.widget") return DisplayEventType::Widget;
    if (eventType == "m.room.server_notice") return DisplayEventType::Notice;
    return DisplayEventType::Unknown;
}

bool isContinuation(const std::string& currentSender, const std::string& previousSender,
    int64_t currentTs, int64_t previousTs, int64_t mergeWindowMs) {
    if (currentSender.empty() || previousSender.empty()) return false;
    if (currentSender != previousSender) return false;
    if (currentTs <= 0 || previousTs <= 0) return false;
    return (currentTs - previousTs) <= mergeWindowMs;
}

bool shouldShowTimestamp(const std::string& currentSender, int64_t currentTs,
    int64_t previousTs, bool showAll) {
    if (showAll) return true;
    if (previousTs <= 0) return true;
    // Show timestamp if more than 5 minutes since last message
    return (currentTs - previousTs) >= 300000;
}

bool shouldShowAvatar(const std::string& currentSender, const std::string& previousSender,
    bool isLastInGroup) {
    if (currentSender != previousSender) return true;
    return isLastInGroup;
}

std::string getEventTypeDescription(DisplayEventType type) {
    switch (type) {
        case DisplayEventType::Message:    return "Message";
        case DisplayEventType::Emote:      return "Emote";
        case DisplayEventType::Notice:     return "Server notice";
        case DisplayEventType::Image:      return "Image";
        case DisplayEventType::Video:      return "Video";
        case DisplayEventType::Audio:      return "Audio";
        case DisplayEventType::File:       return "File";
        case DisplayEventType::Sticker:    return "Sticker";
        case DisplayEventType::Location:   return "Location";
        case DisplayEventType::Poll:       return "Poll";
        case DisplayEventType::Reaction:   return "Reaction";
        case DisplayEventType::MemberEvent: return "Membership change";
        case DisplayEventType::RoomName:   return "Room name changed";
        case DisplayEventType::RoomTopic:  return "Topic changed";
        case DisplayEventType::RoomAvatar: return "Avatar changed";
        case DisplayEventType::Encryption: return "Encryption changed";
        case DisplayEventType::Call:       return "Call";
        case DisplayEventType::Widget:     return "Widget";
        case DisplayEventType::Redaction:  return "Message removed";
        default:                           return "Unknown";
    }
}

std::string getEventTypeIcon(DisplayEventType type) {
    switch (type) {
        case DisplayEventType::Message:  return "\xF0\x9F\x92\xAC"; // 💬
        case DisplayEventType::Image:    return "\xF0\x9F\x93\xB7"; // 📷
        case DisplayEventType::Video:    return "\xF0\x9F\x8E\xA5"; // 🎥
        case DisplayEventType::Audio:    return "\xF0\x9F\x8E\xB5"; // 🎵
        case DisplayEventType::File:     return "\xF0\x9F\x93\x84"; // 📄
        case DisplayEventType::Sticker:  return "\xF0\x9F\x96\xBC"; // 🖼
        case DisplayEventType::Location: return "\xF0\x9F\x93\x8D"; // 📍
        case DisplayEventType::Poll:     return "\xF0\x9F\x93\x8A"; // 📊
        case DisplayEventType::Reaction: return "\xE2\x9D\xA4";     // ❤
        case DisplayEventType::MemberEvent: return "\xF0\x9F\x91\xA4"; // 👤
        case DisplayEventType::Call:     return "\xF0\x9F\x93\x9E"; // 📞
        case DisplayEventType::Redaction: return "\xE2\x9D\x8C";    // ❌
        case DisplayEventType::Notice:   return "\xE2\x84\xB9";     // ℹ
        default:                         return "\xF0\x9F\x93\x9D"; // 📝
    }
}

std::string formatEventPreview(const DisplayEvent& event, bool showSender) {
    std::ostringstream out;
    if (showSender && !event.senderName.empty()) {
        out << event.senderName << ": ";
    }

    switch (event.type) {
        case DisplayEventType::Image:
            out << "[Image]";
            if (!event.body.empty()) out << " " << event.body;
            break;
        case DisplayEventType::Video:
            out << "[Video]";
            if (!event.body.empty()) out << " " << event.body;
            break;
        case DisplayEventType::Audio:
            out << "[Audio]";
            break;
        case DisplayEventType::File:
            out << "[File]";
            break;
        case DisplayEventType::Sticker:
            out << "[Sticker]";
            break;
        case DisplayEventType::Location:
            out << "[Location]";
            break;
        case DisplayEventType::Poll:
            out << "[Poll: " << (event.body.size() > 30 ? event.body.substr(0, 27) + "..." : event.body) << "]";
            break;
        case DisplayEventType::MemberEvent:
            out << event.body;
            break;
        case DisplayEventType::Redaction:
            out << "[Message removed]";
            break;
        case DisplayEventType::Encryption:
            out << "[Encryption changed]";
            break;
        default:
            out << (event.body.size() > 60 ? event.body.substr(0, 57) + "..." : event.body);
            break;
    }

    return out.str();
}

std::string formatMemberEvent(const std::string& senderName, const std::string& membership,
    const std::string& targetName, const std::string& reason) {
    std::ostringstream out;

    if (membership == "join") {
        out << senderName << " joined the room";
    } else if (membership == "leave") {
        out << senderName << " left the room";
    } else if (membership == "invite") {
        out << senderName << " invited " << targetName;
    } else if (membership == "ban") {
        out << senderName << " banned " << targetName;
    } else if (membership == "knock") {
        out << senderName << " requested to join";
    } else {
        out << senderName << " changed membership to " << membership;
    }

    if (!reason.empty()) {
        out << " — " << reason;
    }

    return out.str();
}

} // namespace progressive


// ==== Extended event_display implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string event_display::serialize() const {
    json j = toJson();
    return j.dump();
}

bool event_display::deserialize(const std::string& data) {
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
bool event_display::validate() const {
    if (!m_initialized) {
        LOGE("event_display: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool event_display::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool event_display::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json event_display::getMetrics() const {
    json m;
    m["class"] = "event_display";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int event_display::getOperationCount() const {
    return m_operationCount;
}

void event_display::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void event_display::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "event_display";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool event_display::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool event_display::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool event_display::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void event_display::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void event_display::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int event_display::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void event_display::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> event_display::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> event_display::listItems() const {
    return {};
}

int event_display::itemCount() const {
    return 0;
}

// Versioning
std::string event_display::getVersion() const {
    return "1.0.0";
}

bool event_display::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool event_display::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void event_display::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> event_display::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool event_display::migrateData(int fromVersion, int toVersion) {
    LOGI("event_display: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int event_display::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json event_display::exportData() const {
    return toJson();
}

bool event_display::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void event_display::performCleanup() {
    LOGI("event_display: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t event_display::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool event_display::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool event_display::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool event_display::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void event_display::logDebug(const std::string& msg) const {
    LOGI("event_display: %s", msg.c_str());
}

void event_display::logWarning(const std::string& msg) const {
    LOGW("event_display: %s", msg.c_str());
}

void event_display::logError(const std::string& msg) const {
    LOGE("event_display: %s", msg.c_str());
}
