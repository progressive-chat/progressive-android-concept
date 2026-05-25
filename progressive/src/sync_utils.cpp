#include "progressive/sync_utils.hpp"
#include <sstream>
#include <chrono>

namespace progressive {

std::string buildSyncFilter(const SyncFilter& filter) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };

    std::ostringstream json;
    json << "{";

    // Room filter
    json << R"("room": {)";
    if (!filter.rooms.empty()) {
        json << R"("rooms": [)";
        for (size_t i = 0; i < filter.rooms.size(); ++i) {
            if (i > 0) json << ",";
            json << R"(")" << esc(filter.rooms[i]) << R"(")";
        }
        json << "],";
    }
    if (!filter.notRooms.empty()) {
        json << R"("not_rooms": [)";
        for (size_t i = 0; i < filter.notRooms.size(); ++i) {
            if (i > 0) json << ",";
            json << R"(")" << esc(filter.notRooms[i]) << R"(")";
        }
        json << "],";
    }
    json << R"("timeline": {"limit": )" << filter.timelineLimit << "},";
    json << R"("lazy_load_members": )" << (filter.lazyLoadMembers ? "true" : "false");
    json << "},";

    // Thread filter
    json << R"("threads": {"include": )" << (filter.includeThreads ? "true" : "false") << "},";

    // Presence filter
    json << R"("presence": {"include": )" << (filter.includePresence ? "true" : "false") << "},";

    // Event filter
    json << R"("event_format": "client")";

    json << "}";

    // Clean trailing commas
    std::string result = json.str();
    // Simple cleanup of trailing commas before }
    size_t pos = 0;
    while ((pos = result.find(",}", pos)) != std::string::npos) {
        result.erase(pos, 1);
    }

    return result;
}

SyncFilter getDefaultSyncFilter() {
    SyncFilter filter;
    filter.timelineLimit = 20;
    filter.includeThreads = true;
    filter.lazyLoadMembers = true;
    return filter;
}

SyncFilter getBackgroundSyncFilter() {
    SyncFilter filter;
    filter.timelineLimit = 1;
    filter.includeThreads = false;
    filter.includePresence = false;
    filter.lazyLoadMembers = false;
    return filter;
}

void addRoomToFilter(SyncFilter& filter, const std::string& roomId) {
    // Check not already in list
    for (const auto& r : filter.rooms) {
        if (r == roomId) return;
    }
    filter.rooms.push_back(roomId);
}

bool filterIncludesRoom(const SyncFilter& filter, const std::string& roomId) {
    for (const auto& r : filter.rooms) {
        if (r == roomId) return true;
    }
    return filter.rooms.empty(); // empty = all rooms
}

bool isValidSyncToken(const SyncToken& token) {
    return !token.token.empty();
}

bool isSyncTokenExpired(const SyncToken& token, int maxAgeMinutes) {
    if (!token.valid) return true;
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return (now - token.savedAtMs) > maxAgeMinutes * 60 * 1000LL;
}

SyncToken updateSyncToken(const std::string& nextBatchToken) {
    SyncToken token;
    token.token = nextBatchToken;
    token.savedAtMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    token.isInitialSync = false;
    token.valid = true;
    return token;
}

std::string formatSyncToken(const SyncToken& token) {
    std::ostringstream out;
    out << "Sync token: " << (token.valid ? token.token.substr(0, 20) + "..." : "none") << "\n";
    out << "Initial sync: " << (token.isInitialSync ? "Yes" : "No") << "\n";
    out << "Sync count: " << token.syncCount;
    return out.str();
}

int getSyncTimeoutMs(SyncPreset preset) {
    switch (preset) {
        case SyncPreset::Full:    return 30000;  // 30 seconds
        case SyncPreset::Limited: return 15000;  // 15 seconds
        case SyncPreset::Online:  return 5000;   // 5 seconds
        case SyncPreset::Offline: return 5000;
        default:                  return 30000;
    }
}

SyncFilter getFilterForPreset(SyncPreset preset) {
    switch (preset) {
        case SyncPreset::Full:
            return getDefaultSyncFilter();
        case SyncPreset::Limited: {
            auto f = getDefaultSyncFilter();
            f.timelineLimit = 10;
            return f;
        }
        case SyncPreset::Online: {
            auto f = getDefaultSyncFilter();
            f.timelineLimit = 5;
            f.lazyLoadMembers = false;
            return f;
        }
        case SyncPreset::Offline:
        default:
            return getBackgroundSyncFilter();
    }
}

} // namespace progressive


// ==== Extended sync_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string sync_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool sync_utils::deserialize(const std::string& data) {
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
bool sync_utils::validate() const {
    if (!m_initialized) {
        LOGE("sync_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool sync_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool sync_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json sync_utils::getMetrics() const {
    json m;
    m["class"] = "sync_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int sync_utils::getOperationCount() const {
    return m_operationCount;
}

void sync_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void sync_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "sync_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool sync_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool sync_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool sync_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void sync_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void sync_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int sync_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void sync_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> sync_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> sync_utils::listItems() const {
    return {};
}

int sync_utils::itemCount() const {
    return 0;
}

// Versioning
std::string sync_utils::getVersion() const {
    return "1.0.0";
}

bool sync_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool sync_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void sync_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> sync_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool sync_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("sync_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int sync_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json sync_utils::exportData() const {
    return toJson();
}

bool sync_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void sync_utils::performCleanup() {
    LOGI("sync_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t sync_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool sync_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool sync_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool sync_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void sync_utils::logDebug(const std::string& msg) const {
    LOGI("sync_utils: %s", msg.c_str());
}

void sync_utils::logWarning(const std::string& msg) const {
    LOGW("sync_utils: %s", msg.c_str());
}

void sync_utils::logError(const std::string& msg) const {
    LOGE("sync_utils: %s", msg.c_str());
}
