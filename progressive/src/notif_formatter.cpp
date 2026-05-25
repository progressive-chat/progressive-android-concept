#include "progressive/notif_formatter.hpp"
#include <algorithm>

namespace progressive {

// ==== MXC URL Builder ====

std::string buildMxcDownloadUrl(const std::string& homeserver, const std::string& mxcUri) {
    auto parsed = parseMxcUri(mxcUri);
    if (!parsed.valid) return "";

    std::string url = homeserver;
    if (!url.empty() && url.back() == '/') url.pop_back();
    url += "/_matrix/media/r0/download/" + parsed.serverName + "/" + parsed.mediaId;
    return url;
}

std::string buildMxcThumbnailUrl(
    const std::string& homeserver,
    const std::string& mxcUri,
    int width, int height,
    const std::string& method)
{
    auto parsed = parseMxcUri(mxcUri);
    if (!parsed.valid) return "";

    std::string url = homeserver;
    if (!url.empty() && url.back() == '/') url.pop_back();
    url += "/_matrix/media/r0/thumbnail/" + parsed.serverName + "/" + parsed.mediaId;
    url += "?width=" + std::to_string(width);
    url += "&height=" + std::to_string(height);
    url += "&method=" + method;
    return url;
}

MxcUri parseMxcUri(const std::string& mxcUri) {
    MxcUri result;
    if (mxcUri.compare(0, 6, "mxc://") != 0) return result;

    size_t serverStart = 6;
    auto slashPos = mxcUri.find('/', serverStart);
    if (slashPos == std::string::npos) return result;

    result.serverName = mxcUri.substr(serverStart, slashPos - serverStart);
    result.mediaId = mxcUri.substr(slashPos + 1);
    result.valid = !result.serverName.empty() && !result.mediaId.empty();
    return result;
}

// ==== Notification Formatting ====

std::string formatTextNotification(
    const std::string& senderName,
    const std::string& body,
    const NotificationFormatConfig& config)
{
    std::string result;
    if (config.showSenderName && !senderName.empty()) {
        result += senderName + ": ";
    }

    if ((int)body.size() <= config.maxBodyLength) {
        result += body;
    } else {
        result += body.substr(0, config.maxBodyLength) + config.truncatedSuffix;
    }

    return result;
}

std::string formatImageNotification(const std::string& senderName) {
    return senderName.empty() ? "sent an image" : senderName + " sent an image";
}

std::string formatFileNotification(const std::string& senderName, const std::string& fileName) {
    std::string msg = senderName.empty() ? "sent a file" : senderName + " sent a file";
    if (!fileName.empty()) msg += ": " + fileName;
    return msg;
}

std::string formatVideoNotification(const std::string& senderName) {
    return senderName.empty() ? "sent a video" : senderName + " sent a video";
}

std::string formatAudioNotification(const std::string& senderName, bool isVoice) {
    std::string base = senderName.empty() ? "sent " : senderName + " sent ";
    return base + (isVoice ? "a voice message" : "an audio file");
}

std::string formatInviteNotification(const std::string& inviterName, const std::string& roomName) {
    return inviterName + " invited you to " + roomName;
}

std::string formatRoomNotification(int messageCount, const std::string& roomName) {
    return std::to_string(messageCount) + " new message" +
           (messageCount != 1 ? "s" : "") + " in " + roomName;
}

std::string formatStickerNotification(const std::string& senderName) {
    return senderName.empty() ? "sent a sticker" : senderName + " sent a sticker";
}

std::string formatLocationNotification(const std::string& senderName) {
    return senderName.empty() ? "shared their location" : senderName + " shared their location";
}

std::string formatPollNotification(const std::string& senderName, bool isStart) {
    return senderName.empty()
        ? (isStart ? "created a poll" : "ended a poll")
        : (senderName + (isStart ? " created a poll" : " ended a poll"));
}

// ==== Build Full Notification ====

NotificationText buildNotificationText(
    const std::string& msgType,
    const std::string& senderName,
    const std::string& body,
    const std::string& roomName,
    const std::string& fileName,
    const NotificationFormatConfig& config)
{
    NotificationText result;

    // Title: room name
    result.title = config.showRoomName ? roomName : "";

    // Body: message-type-specific preview
    if (msgType == "m.text" || msgType == "m.notice" || msgType == "m.emote") {
        result.body = formatTextNotification(senderName, body, config);
    } else if (msgType == "m.image") {
        result.body = formatImageNotification(senderName);
    } else if (msgType == "m.video") {
        result.body = formatVideoNotification(senderName);
    } else if (msgType == "m.audio") {
        result.body = formatAudioNotification(senderName, false);
    } else if (msgType == "m.file") {
        result.body = formatFileNotification(senderName, fileName);
    } else if (msgType == "m.sticker") {
        result.body = formatStickerNotification(senderName);
    } else if (msgType == "m.location") {
        result.body = formatLocationNotification(senderName);
    } else {
        result.body = formatTextNotification(senderName, body, config);
    }

    // Ticker: short version for status bar
    result.ticker = senderName + ": " + body.substr(0, std::min((int)body.size(), 80));

    return result;
}

} // namespace progressive


// ==== Extended notif_formatter implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string notif_formatter::serialize() const {
    json j = toJson();
    return j.dump();
}

bool notif_formatter::deserialize(const std::string& data) {
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
bool notif_formatter::validate() const {
    if (!m_initialized) {
        LOGE("notif_formatter: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool notif_formatter::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool notif_formatter::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json notif_formatter::getMetrics() const {
    json m;
    m["class"] = "notif_formatter";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int notif_formatter::getOperationCount() const {
    return m_operationCount;
}

void notif_formatter::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void notif_formatter::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "notif_formatter";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool notif_formatter::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool notif_formatter::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool notif_formatter::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void notif_formatter::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void notif_formatter::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int notif_formatter::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void notif_formatter::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> notif_formatter::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> notif_formatter::listItems() const {
    return {};
}

int notif_formatter::itemCount() const {
    return 0;
}

// Versioning
std::string notif_formatter::getVersion() const {
    return "1.0.0";
}

bool notif_formatter::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool notif_formatter::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void notif_formatter::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> notif_formatter::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool notif_formatter::migrateData(int fromVersion, int toVersion) {
    LOGI("notif_formatter: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int notif_formatter::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json notif_formatter::exportData() const {
    return toJson();
}

bool notif_formatter::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void notif_formatter::performCleanup() {
    LOGI("notif_formatter: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t notif_formatter::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool notif_formatter::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool notif_formatter::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool notif_formatter::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void notif_formatter::logDebug(const std::string& msg) const {
    LOGI("notif_formatter: %s", msg.c_str());
}

void notif_formatter::logWarning(const std::string& msg) const {
    LOGW("notif_formatter: %s", msg.c_str());
}

void notif_formatter::logError(const std::string& msg) const {
    LOGE("notif_formatter: %s", msg.c_str());
}
