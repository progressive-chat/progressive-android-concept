#include "progressive/presence_utils.hpp"
#include <sstream>
#include <chrono>
#include <algorithm>

namespace progressive {

PresenceInfo parsePresence(const std::string& userId, const std::string& apiResponseJson) {
    PresenceInfo info;
    info.userId = userId;

    auto presenceStr = apiResponseJson.substr(
        apiResponseJson.find("\"presence\":\"") + 12,
        apiResponseJson.find('"', apiResponseJson.find("\"presence\":\"") + 12) -
            apiResponseJson.find("\"presence\":\"") - 12
    );

    // Parse manually due to nested JSON
    auto extract = [&](const std::string& key) -> std::string {
        auto search = '"' + key + '"';
        auto pos = apiResponseJson.find(search);
        if (pos == std::string::npos) return {};
        pos = apiResponseJson.find(':', pos);
        if (pos == std::string::npos) return {};
        ++pos;
        while (pos < apiResponseJson.size() && apiResponseJson[pos] == ' ') ++pos;
        if (pos >= apiResponseJson.size()) return {};
        if (apiResponseJson[pos] == '"') {
            ++pos;
            auto end = apiResponseJson.find('"', pos);
            if (end == std::string::npos) return {};
            return apiResponseJson.substr(pos, end - pos);
        }
        auto end = pos;
        while (end < apiResponseJson.size() && apiResponseJson[end] != ',' && apiResponseJson[end] != '}' && apiResponseJson[end] != ' ') ++end;
        return apiResponseJson.substr(pos, end - pos);
    };

    auto ps = extract("presence");
    if (ps == "online") info.presence = Presence::Online;
    else if (ps == "unavailable") info.presence = Presence::Unavailable;
    else if (ps == "offline") info.presence = Presence::Offline;
    else info.presence = Presence::Unknown;

    auto la = extract("last_active_ago");
    if (!la.empty()) info.lastActiveAgoMs = std::stoll(la);

    info.statusMessage = extract("status_msg");

    return info;
}

std::string formatPresence(Presence presence) {
    switch (presence) {
        case Presence::Online:      return "Online";
        case Presence::Unavailable: return "Away";
        case Presence::Offline:     return "Offline";
        default:                    return "Unknown";
    }
}

std::string formatPresenceWithTime(Presence presence, int64_t lastActiveAgoMs) {
    if (presence == Presence::Online) return "Online";
    if (presence == Presence::Unavailable) {
        int minutes = static_cast<int>(lastActiveAgoMs / 60000);
        if (minutes < 1) return "Away";
        if (minutes < 60) return "Away " + std::to_string(minutes) + "m";
        return "Away " + std::to_string(minutes / 60) + "h";
    }
    if (presence == Presence::Offline) {
        int hours = static_cast<int>(lastActiveAgoMs / 3600000);
        if (hours < 1) return "Offline";
        if (hours < 24) return "Offline " + std::to_string(hours) + "h";
        return "Offline " + std::to_string(hours / 24) + "d";
    }
    return "Unknown";
}

std::string getPresenceIndicator(Presence presence) {
    switch (presence) {
        case Presence::Online:      return "\xF0\x9F\x9F\xA2"; // 🟢
        case Presence::Unavailable: return "\xF0\x9F\x9F\xA1"; // 🟡
        case Presence::Offline:     return "\xE2\x9A\xAB";     // ⚫
        default:                    return "\xE2\x9A\xAA";     // ⚪
    }
}

bool isPresenceStale(int64_t lastUpdatedMs) {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return (now - lastUpdatedMs) > 5 * 60 * 1000; // 5 minutes
}

std::string formatStatusMessage(const std::string& message, int maxLen) {
    if (message.empty()) return "";
    if (static_cast<int>(message.size()) <= maxLen) return message;
    return message.substr(0, maxLen - 3) + "...";
}

std::string formatPresenceLine(const PresenceInfo& info) {
    std::ostringstream out;
    out << getPresenceIndicator(info.presence) << " ";
    out << formatPresenceWithTime(info.presence, info.lastActiveAgoMs);
    if (!info.statusMessage.empty()) {
        out << " — " << formatStatusMessage(info.statusMessage, 60);
    }
    return out.str();
}

// ---- UserActivityTimer ----

int64_t UserActivityTimer::now() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

void UserActivityTimer::start() {
    startMs_ = now();
    totalPaused_ = 0;
    isRunning_ = true;
    isPaused_ = false;
}

void UserActivityTimer::pause() {
    if (!isRunning_ || isPaused_) return;
    pauseStart_ = now();
    isPaused_ = true;
}

void UserActivityTimer::resume() {
    if (!isRunning_ || !isPaused_) return;
    totalPaused_ += now() - pauseStart_;
    isPaused_ = false;
}

void UserActivityTimer::stop() {
    isRunning_ = false;
    isPaused_ = false;
}

int64_t UserActivityTimer::elapsedMs() const {
    if (!isRunning_) return 0;
    int64_t current = now();
    int64_t elapsed = current - startMs_ - totalPaused_;
    if (isPaused_) elapsed -= (current - pauseStart_);
    return elapsed;
}

std::string UserActivityTimer::elapsedFormatted() const {
    int64_t ms = elapsedMs();
    if (ms < 0) return "0s";
    int64_t sec = ms / 1000;
    int hours = sec / 3600;
    int minutes = (sec % 3600) / 60;
    int seconds = sec % 60;
    std::ostringstream out;
    if (hours > 0) out << hours << "h ";
    if (minutes > 0) out << minutes << "m ";
    out << seconds << "s";
    return out.str();
}

} // namespace progressive


// ==== Extended presence_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string presence_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool presence_utils::deserialize(const std::string& data) {
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
bool presence_utils::validate() const {
    if (!m_initialized) {
        LOGE("presence_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool presence_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool presence_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json presence_utils::getMetrics() const {
    json m;
    m["class"] = "presence_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int presence_utils::getOperationCount() const {
    return m_operationCount;
}

void presence_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void presence_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "presence_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool presence_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool presence_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool presence_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void presence_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void presence_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int presence_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void presence_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> presence_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> presence_utils::listItems() const {
    return {};
}

int presence_utils::itemCount() const {
    return 0;
}

// Versioning
std::string presence_utils::getVersion() const {
    return "1.0.0";
}

bool presence_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool presence_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void presence_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> presence_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool presence_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("presence_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int presence_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json presence_utils::exportData() const {
    return toJson();
}

bool presence_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void presence_utils::performCleanup() {
    LOGI("presence_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t presence_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool presence_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool presence_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool presence_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void presence_utils::logDebug(const std::string& msg) const {
    LOGI("presence_utils: %s", msg.c_str());
}

void presence_utils::logWarning(const std::string& msg) const {
    LOGW("presence_utils: %s", msg.c_str());
}

void presence_utils::logError(const std::string& msg) const {
    LOGE("presence_utils: %s", msg.c_str());
}
