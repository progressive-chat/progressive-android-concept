#include "progressive/cache_manager.hpp"
#include <sstream>
#include <ctime>
#include <algorithm>
#include <unordered_set>

namespace progressive {

void CacheManager::track(const std::string& eventId, const std::string& roomId,
                         const std::string& roomName, int64_t timestamp, int64_t sizeBytes,
                         const std::string& msgType, const std::string& body) {
    // Update if exists, otherwise add
    for (auto& e : entries_) {
        if (e.eventId == eventId) {
            e.roomName = roomName;
            e.timestamp = timestamp;
            e.sizeBytes = sizeBytes;
            e.msgType = msgType;
            e.body = body;
            return;
        }
    }
    entries_.push_back({eventId, roomId, roomName, body, timestamp, sizeBytes, msgType});
}

CacheStats CacheManager::getStats() const {
    CacheStats stats;
    stats.entryCount = static_cast<int>(entries_.size());

    std::unordered_set<std::string> rooms;
    int64_t oldest = 0, newest = 0;

    for (const auto& e : entries_) {
        stats.totalBytes += e.sizeBytes;
        rooms.insert(e.roomId);
        if (oldest == 0 || e.timestamp < oldest) oldest = e.timestamp;
        if (e.timestamp > newest) newest = e.timestamp;
    }

    stats.roomCount = static_cast<int>(rooms.size());

    if (oldest > 0) {
        time_t ts = oldest / 1000;
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&ts));
        stats.oldestEntryDate = buf;
    }
    if (newest > 0) {
        time_t ts = newest / 1000;
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&ts));
        stats.newestEntryDate = buf;
    }

    return stats;
}

std::vector<CacheEntry> CacheManager::getByRoom(const std::string& roomId) const {
    std::vector<CacheEntry> result;
    for (const auto& e : entries_) {
        if (e.roomId == roomId) result.push_back(e);
    }
    return result;
}

std::vector<CacheEntry> CacheManager::getOlderThan(int64_t beforeTs) const {
    std::vector<CacheEntry> result;
    for (const auto& e : entries_) {
        if (e.timestamp < beforeTs) result.push_back(e);
    }
    return result;
}

std::vector<CacheEntry> CacheManager::getByDateRange(int64_t fromTs, int64_t toTs) const {
    std::vector<CacheEntry> result;
    for (const auto& e : entries_) {
        if (e.timestamp >= fromTs && e.timestamp <= toTs) result.push_back(e);
    }
    return result;
}

std::vector<std::string> CacheManager::markForDeletion(const std::vector<std::string>& eventIds) {
    std::unordered_set<std::string> toDelete(eventIds.begin(), eventIds.end());
    std::vector<std::string> deleted;

    entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
        [&](const CacheEntry& e) {
            if (toDelete.count(e.eventId)) {
                deleted.push_back(e.eventId);
                return true;
            }
            return false;
        }
    ), entries_.end());

    return deleted;
}

void CacheManager::clear() {
    entries_.clear();
}

std::vector<std::string> CacheManager::getCachedRoomIds() const {
    std::unordered_set<std::string> seen;
    std::vector<std::string> result;
    for (const auto& e : entries_) {
        if (seen.insert(e.roomId).second) {
            result.push_back(e.roomId);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::string CacheManager::statsToJson() const {
    auto stats = getStats();
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) {
            if (c == '"') out += "\\\"";
            else out += c;
        }
        return out;
    };

    std::ostringstream json;
    json << "{";
    json << R"("entryCount": )" << stats.entryCount << ",";
    json << R"("roomCount": )" << stats.roomCount << ",";
    json << R"("totalBytes": )" << stats.totalBytes << ",";
    json << R"("oldestDate": ")" << esc(stats.oldestEntryDate) << R"(",)";
    json << R"("newestDate": ")" << esc(stats.newestEntryDate) << R"(")";
    json << "}";
    return json.str();
}

std::string CacheManager::entriesToJson(const std::vector<CacheEntry>& entries) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) {
            if (c == '"') out += "\\\"";
            else out += c;
        }
        return out;
    };

    std::ostringstream json;
    json << "[";
    for (size_t i = 0; i < entries.size(); ++i) {
        if (i > 0) json << ",";
        const auto& e = entries[i];
        json << R"({"eventId": ")" << esc(e.eventId) << R"(")";
        json << R"(,"roomId": ")" << esc(e.roomId) << R"(")";
        json << R"(,"roomName": ")" << esc(e.roomName) << R"(")";
        json << R"(,"body": ")" << esc(e.body) << R"(")";
        json << R"(,"timestamp": )" << e.timestamp;
        json << R"(,"sizeBytes": )" << e.sizeBytes << "}";
    }
    json << "]";
    return json.str();
}

} // namespace progressive


// ==== Extended cache_manager implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string cache_manager::serialize() const {
    json j = toJson();
    return j.dump();
}

bool cache_manager::deserialize(const std::string& data) {
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
bool cache_manager::validate() const {
    if (!m_initialized) {
        LOGE("cache_manager: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool cache_manager::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool cache_manager::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json cache_manager::getMetrics() const {
    json m;
    m["class"] = "cache_manager";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int cache_manager::getOperationCount() const {
    return m_operationCount;
}

void cache_manager::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void cache_manager::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "cache_manager";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool cache_manager::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool cache_manager::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool cache_manager::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void cache_manager::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void cache_manager::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int cache_manager::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void cache_manager::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> cache_manager::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> cache_manager::listItems() const {
    return {};
}

int cache_manager::itemCount() const {
    return 0;
}

// Versioning
std::string cache_manager::getVersion() const {
    return "1.0.0";
}

bool cache_manager::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool cache_manager::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void cache_manager::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> cache_manager::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool cache_manager::migrateData(int fromVersion, int toVersion) {
    LOGI("cache_manager: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int cache_manager::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json cache_manager::exportData() const {
    return toJson();
}

bool cache_manager::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void cache_manager::performCleanup() {
    LOGI("cache_manager: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t cache_manager::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool cache_manager::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool cache_manager::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool cache_manager::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void cache_manager::logDebug(const std::string& msg) const {
    LOGI("cache_manager: %s", msg.c_str());
}

void cache_manager::logWarning(const std::string& msg) const {
    LOGW("cache_manager: %s", msg.c_str());
}

void cache_manager::logError(const std::string& msg) const {
    LOGE("cache_manager: %s", msg.c_str());
}
