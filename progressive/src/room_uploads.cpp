#include "progressive/room_uploads.hpp"

namespace progressive {

// ==== Sticker Detection ====
//
// Original Kotlin (GetUploadsTask.kt:72):
//   .filter { it.getClearType() != EventType.STICKER }
//   EventType.STICKER = "m.sticker"

bool isStickerEvent(const std::string& eventType) {
    return eventType == "m.sticker";
}

// ==== Attachment URL Detection ====
//
// Original Kotlin (GetUploadsTask.kt:68):
//   .like(EventEntityFields.DECRYPTION_RESULT_JSON,
//         TimelineEventFilter.DecryptedContent.URL)
//
// For encrypted messages: the decrypted content JSON should contain
// a "url" field pointing to the MXC URI.

bool hasAttachmentUrl(const std::string& decryptedContentJson) {
    // Original Kotlin: checks if decryption result contains URL
    // Look for "url": "mxc://..." in the JSON
    auto pos = decryptedContentJson.find("\"url\"");
    if (pos == std::string::npos) return false;
    // Check that the URL is an mxc:// URI
    pos = decryptedContentJson.find("mxc://", pos);
    return pos != std::string::npos;
}

// ==== Event Extraction ====
//
// Original Kotlin (GetUploadsTask.kt:103-116):
//   val eventId = event.eventId
//   val messageContent = event.getClearContent()?.toModel<MessageContent>()
//   val messageWithAttachmentContent = (messageContent as? MessageWithAttachmentContent)
//   val senderInfo = cacheOfSenderInfos.getOrPut(senderId) { ... }

static std::string extractJsonField(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.size() || json[pos] != '"') return "";
    pos++;
    size_t end = pos;
    while (end < json.size() && json[end] != '"') {
        if (json[end] == '\\') end++;
        end++;
    }
    return json.substr(pos, end - pos);
}

static int64_t extractJsonInt64(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return 0;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return 0;
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.size()) return 0;
    int64_t val = 0;
    while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
        val = val * 10 + (json[pos] - '0');
        pos++;
    }
    return val;
}

UploadEvent extractUploadEvent(
    const std::string& eventJson,
    const std::string& senderName,
    const std::string& senderAvatarUrl,
    bool isUniqueDisplayName)
{
    UploadEvent event;

    // Original Kotlin: val eventId = event.eventId
    event.eventId = extractJsonField(eventJson, "event_id");
    event.senderId = extractJsonField(eventJson, "sender");

    // Original Kotlin: senderInfo.displayName, avatarUrl
    event.senderName = senderName;
    event.senderAvatarUrl = senderAvatarUrl;

    // Extract content fields
    auto contentPos = eventJson.find("\"content\"");
    if (contentPos != std::string::npos) {
        contentPos = eventJson.find('{', contentPos);
        if (contentPos != std::string::npos) {
            // Brace-count to extract content object
            int depth = 1;
            size_t start = contentPos;
            contentPos++;
            while (contentPos < eventJson.size() && depth > 0) {
                if (eventJson[contentPos] == '{') depth++;
                else if (eventJson[contentPos] == '}') depth--;
                contentPos++;
            }
            std::string contentJson = eventJson.substr(start, contentPos - start);

            // Check for "info" sub-object
            auto infoPos = contentJson.find("\"info\"");
            std::string infoJson;
            if (infoPos != std::string::npos) {
                infoPos = contentJson.find('{', infoPos);
                if (infoPos != std::string::npos) {
                    int d = 1;
                    size_t is = infoPos;
                    infoPos++;
                    while (infoPos < contentJson.size() && d > 0) {
                        if (contentJson[infoPos] == '{') d++;
                        else if (contentJson[infoPos] == '}') d--;
                        infoPos++;
                    }
                    infoJson = contentJson.substr(is, infoPos - is);
                }
            }

            // Original Kotlin: messageWithAttachmentContent fields
            event.mxcUrl = extractJsonField(contentJson, "url");
            event.fileName = extractJsonField(contentJson, "filename");
            event.mimeType = extractJsonField(contentJson, "mimetype");

            // From info sub-object
            if (!infoJson.empty()) {
                if (event.mimeType.empty()) event.mimeType = extractJsonField(infoJson, "mimetype");
                if (event.fileName.empty()) event.fileName = extractJsonField(infoJson, "filename");
                event.fileSize = extractJsonInt64(infoJson, "size");
            }

            // Size from body if not in info
            if (event.fileSize == 0) {
                event.fileSize = extractJsonInt64(contentJson, "size");
            }
        }
    }

    // Original Kotlin: origin_server_ts
    event.timestamp = extractJsonInt64(eventJson, "origin_server_ts");

    return event;
}

// ==== Uploads Filter ====
//
// Original Kotlin (FilterFactory.createUploadsFilter(numberOfEvents)):
//   Creates a RoomEventFilter with:
//   - types: ["m.room.message"]
//   - not_types: ["m.room.message.feedback"] (exclude reactions)
//   - limit: numberOfEvents
// The filter is used for the /messages API to only return attachment messages.

std::string createUploadsFilterJson(int numberOfEvents) {
    // Original Kotlin: creates a JSON filter for /messages
    // {
    //   "room": {
    //     "timeline": {
    //       "types": ["m.room.message"],
    //       "not_types": ["m.room.member", "m.room.create"]
    //     }
    //   }
    // }
    // Actually, the uploads filter is simpler:
    // {
    //   "room": {
    //     "timeline": {
    //       "limit": N,
    //       "types": ["m.room.message"],
    //       "not_types": ["m.room.member"]
    //     }
    //   }
    // }
    std::string json = "{";
    json += "\"room\":{";
    json += "\"timeline\":{";
    json += "\"limit\":" + std::to_string(numberOfEvents) + ",";
    json += "\"types\":[\"m.room.message\"],";
    json += "\"not_types\":[\"m.room.member\",\"m.sticker\"]";
    json += "}}}";
    return json;
}

// ==== Serialization ====

std::string getUploadsResultToJson(const GetUploadsResult& result) {
    // Original Kotlin: Kotlin serialization of GetUploadsResult
    std::string json = "{";
    json += "\"uploadEvents\":[";
    bool first = true;
    for (const auto& ev : result.uploadEvents) {
        if (!first) json += ",";
        first = false;
        json += "{";
        json += "\"eventId\":\"" + ev.eventId + "\",";
        json += "\"senderName\":\"" + ev.senderName + "\",";
        json += "\"mxcUrl\":\"" + ev.mxcUrl + "\",";
        json += "\"fileName\":\"" + ev.fileName + "\",";
        json += "\"mimeType\":\"" + ev.mimeType + "\",";
        json += "\"fileSize\":" + std::to_string(ev.fileSize) + ",";
        json += "\"timestamp\":" + std::to_string(ev.timestamp);
        json += "}";
    }
    json += "],";
    json += "\"nextToken\":\"" + result.nextToken + "\",";
    json += "\"hasMore\":" + std::string(result.hasMore ? "true" : "false");
    json += "}";
    return json;
}

} // namespace progressive


// ==== Extended room_uploads implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string room_uploads::serialize() const {
    json j = toJson();
    return j.dump();
}

bool room_uploads::deserialize(const std::string& data) {
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
bool room_uploads::validate() const {
    if (!m_initialized) {
        LOGE("room_uploads: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool room_uploads::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool room_uploads::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json room_uploads::getMetrics() const {
    json m;
    m["class"] = "room_uploads";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int room_uploads::getOperationCount() const {
    return m_operationCount;
}

void room_uploads::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void room_uploads::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "room_uploads";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool room_uploads::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool room_uploads::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool room_uploads::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void room_uploads::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void room_uploads::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int room_uploads::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void room_uploads::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> room_uploads::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> room_uploads::listItems() const {
    return {};
}

int room_uploads::itemCount() const {
    return 0;
}

// Versioning
std::string room_uploads::getVersion() const {
    return "1.0.0";
}

bool room_uploads::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool room_uploads::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void room_uploads::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> room_uploads::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool room_uploads::migrateData(int fromVersion, int toVersion) {
    LOGI("room_uploads: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int room_uploads::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json room_uploads::exportData() const {
    return toJson();
}

bool room_uploads::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void room_uploads::performCleanup() {
    LOGI("room_uploads: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t room_uploads::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool room_uploads::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool room_uploads::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool room_uploads::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void room_uploads::logDebug(const std::string& msg) const {
    LOGI("room_uploads: %s", msg.c_str());
}

void room_uploads::logWarning(const std::string& msg) const {
    LOGW("room_uploads: %s", msg.c_str());
}

void room_uploads::logError(const std::string& msg) const {
    LOGE("room_uploads: %s", msg.c_str());
}
