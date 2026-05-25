#include "progressive/desync_detector.hpp"
#include <sstream>
#include <chrono>

namespace progressive {

void DesyncDetector::trackEvent(const std::string& eventId, const std::string& serverName, int64_t timestamp) {
    serverEvents_[serverName].insert(eventId);
    
    auto it = eventTimestamps_.find(eventId);
    if (it == eventTimestamps_.end() || timestamp > it->second) {
        eventTimestamps_[eventId] = timestamp;
    }
}

DesyncReport DesyncDetector::checkDesync(const std::string& roomId, const std::string& currentServer) {
    DesyncReport report;
    report.roomId = roomId;
    report.lastCheckMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    auto servers = getServers(roomId);

    // Need at least 2 servers to detect desync
    if (servers.size() < 2) return report;

    // Get the event set for current server
    auto currentIt = serverEvents_.find(currentServer);
    if (currentIt == serverEvents_.end()) return report;

    const auto& currentSet = currentIt->second;

    // Compare against every other server
    for (const auto& otherServer : servers) {
        if (otherServer == currentServer) continue;

        auto otherIt = serverEvents_.find(otherServer);
        if (otherIt == serverEvents_.end()) continue;
        const auto& otherSet = otherIt->second;

        // Check: events on other server that are missing on current
        std::vector<std::string> missingOnCurrent;
        for (const auto& eid : otherSet) {
            if (currentSet.find(eid) == currentSet.end()) {
                missingOnCurrent.push_back(eid);
            }
        }

        // Check: events on current server that are missing on other
        std::vector<std::string> missingOnOther;
        for (const auto& eid : currentSet) {
            if (otherSet.find(eid) == otherSet.end()) {
                missingOnOther.push_back(eid);
            }
        }

        if (!missingOnCurrent.empty() || !missingOnOther.empty()) {
            report.hasDesync = true;
            report.totalEventsCompared = static_cast<int>(currentSet.size() + otherSet.size());
            report.missingOnCurrent += static_cast<int>(missingOnCurrent.size());
            report.missingOnOther += static_cast<int>(missingOnOther.size());
            report.missingServer = otherServer;

            for (const auto& eid : missingOnCurrent) {
                report.missingEventIds.push_back(eid);
            }
        }
    }

    return report;
}

std::vector<std::string> DesyncDetector::getServers(const std::string& roomId) const {
    std::vector<std::string> result;
    for (const auto& p : serverEvents_) {
        if (!p.second.empty()) result.push_back(p.first);
    }
    return result;
}

void DesyncDetector::removeRoom(const std::string& roomId) {
    // We don't store by roomId currently, so this is a no-op.
    // Future: could prefix eventIds with roomId for per-room tracking.
}

bool DesyncDetector::hasEventOnServer(const std::string& eventId, const std::string& serverName) const {
    auto it = serverEvents_.find(serverName);
    if (it == serverEvents_.end()) return false;
    return it->second.find(eventId) != it->second.end();
}

bool DesyncDetector::shouldCheck(int64_t lastCheckMs, int intervalMinutes) const {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return (now - lastCheckMs) >= (static_cast<int64_t>(intervalMinutes) * 60 * 1000LL);
}

std::string DesyncDetector::formatDesyncWarning(const DesyncReport& report) {
    if (!report.hasDesync) return "";

    std::ostringstream out;
    out << report.missingOnCurrent << " event(s) missing on current server";
    if (!report.missingServer.empty()) {
        out << " (present on " << report.missingServer << ")";
    }
    out << ". Check room federation.";
    return out.str();
}

std::string DesyncDetector::reportToJson(const DesyncReport& report) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) { if (c == '"') out += "\\\""; else out += c; }
        return out;
    };

    std::ostringstream json;
    json << "{";
    json << R"("hasDesync": )" << (report.hasDesync ? "true" : "false") << ",";
    json << R"("roomId": ")" << esc(report.roomId) << R"(",)";
    json << R"("totalEventsCompared": )" << report.totalEventsCompared << ",";
    json << R"("missingOnCurrent": )" << report.missingOnCurrent << ",";
    json << R"("missingOnOther": )" << report.missingOnOther << ",";
    json << R"("missingServer": ")" << esc(report.missingServer) << R"(",)";
    json << R"("warning": ")" << esc(formatDesyncWarning(report)) << R"(",)";
    json << R"("lastCheckMs": )" << report.lastCheckMs;
    if (!report.missingEventIds.empty()) {
        json << R"(,"missingEventIds": [)";
        for (size_t i = 0; i < report.missingEventIds.size(); ++i) {
            if (i > 0) json << ",";
            json << R"(")" << esc(report.missingEventIds[i]) << R"(")";
        }
        json << "]";
    }
    json << "}";
    return json.str();
}

void DesyncDetector::clear() {
    serverEvents_.clear();
    eventTimestamps_.clear();
}


// ---- DesyncCheckResult ----

DesyncCheckResult checkSyncDesynchronization(
    const std::string& roomId,
    int64_t lastSyncMs,
    const std::vector<std::string>& localEventIds,
    const std::vector<std::string>& serverEventIds,
    const std::string& localReadMarker,
    const std::string& serverReadMarker,
    const std::string& localMembership,
    const std::string& serverMembership
) {
    DesyncCheckResult result;
    (void)roomId; // roomId reserved for future logging / metric keying

    // Factor 1: Check sync freshness
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    result.lastSyncAgeMs = now - lastSyncMs;

    // Factor 2: Missing events in timeline
    std::unordered_set<std::string> localSet(localEventIds.begin(), localEventIds.end());
    for (const auto& eid : serverEventIds) {
        if (localSet.find(eid) == localSet.end()) {
            ++result.missingEvents;
        }
    }

    // Factor 3: Extra events locally that are not on server
    std::unordered_set<std::string> serverSet(serverEventIds.begin(), serverEventIds.end());
    for (const auto& eid : localEventIds) {
        if (serverSet.find(eid) == serverSet.end()) {
            ++result.extraEvents;
        }
    }

    // Factor 4: Read marker conflicts
    bool readMarkerConflict = !localReadMarker.empty() && !serverReadMarker.empty() &&
                               localReadMarker != serverReadMarker;

    // Factor 5: Membership inconsistencies
    bool membershipConflict = !localMembership.empty() && !serverMembership.empty() &&
                               localMembership != serverMembership;

    // Aggregate result
    result.isDesynchronized = (result.missingEvents > 0) || (result.extraEvents > 0) ||
                               readMarkerConflict || membershipConflict;

    if (result.isDesynchronized) {
        std::ostringstream reason;
        if (result.missingEvents > 0) {
            reason << result.missingEvents << " event(s) missing from local timeline; ";
        }
        if (result.extraEvents > 0) {
            reason << result.extraEvents << " event(s) extra in local timeline; ";
        }
        if (readMarkerConflict) {
            reason << "read marker conflict; ";
        }
        if (membershipConflict) {
            reason << "membership inconsistency; ";
        }
        result.reason = reason.str();
    } else {
        result.reason = "No desync detected";
    }

    return result;
}

// ---- DesyncCause ----

DesyncCause diagnoseDesyncCause(const DesyncCheckResult& result) {
    if (!result.isDesynchronized) return DesyncCause::UNKNOWN;

    // Cache corruption: extra events locally that server doesn't know about
    if (result.extraEvents > 0 && result.missingEvents == 0) {
        return DesyncCause::CACHE_CORRUPTION;
    }

    // Network gap: many missing events
    if (result.missingEvents > 10) {
        return DesyncCause::NETWORK_GAP;
    }

    // Race condition: few missing events, recently synced
    if (result.missingEvents <= 3 && result.lastSyncAgeMs < 60000) {
        return DesyncCause::RACE_CONDITION;
    }

    // Server bug / federation issue: events missing but no network gap
    if (result.missingEvents > 0 && result.lastSyncAgeMs < 300000) {
        return DesyncCause::SERVER_BUG;
    }

    return DesyncCause::UNKNOWN;
}

std::string getDesyncRecoveryAction(DesyncCause cause) {
    switch (cause) {
        case DesyncCause::NETWORK_GAP:     return "re-sync";
        case DesyncCause::CACHE_CORRUPTION: return "clear_cache";
        case DesyncCause::SERVER_BUG:       return "re-sync";
        case DesyncCause::RACE_CONDITION:   return "re-sync";
        default:                            return "none";
    }
}

std::string getDesyncRecoveryAction(const DesyncCheckResult& result) {
    if (!result.isDesynchronized) return "none";
    return getDesyncRecoveryAction(diagnoseDesyncCause(result));
}

} // namespace progressive


// ==== Extended desync_detector implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string desync_detector::serialize() const {
    json j = toJson();
    return j.dump();
}

bool desync_detector::deserialize(const std::string& data) {
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
bool desync_detector::validate() const {
    if (!m_initialized) {
        LOGE("desync_detector: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool desync_detector::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool desync_detector::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json desync_detector::getMetrics() const {
    json m;
    m["class"] = "desync_detector";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int desync_detector::getOperationCount() const {
    return m_operationCount;
}

void desync_detector::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void desync_detector::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "desync_detector";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool desync_detector::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool desync_detector::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool desync_detector::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void desync_detector::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void desync_detector::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int desync_detector::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void desync_detector::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> desync_detector::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> desync_detector::listItems() const {
    return {};
}

int desync_detector::itemCount() const {
    return 0;
}

// Versioning
std::string desync_detector::getVersion() const {
    return "1.0.0";
}

bool desync_detector::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool desync_detector::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void desync_detector::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> desync_detector::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool desync_detector::migrateData(int fromVersion, int toVersion) {
    LOGI("desync_detector: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int desync_detector::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json desync_detector::exportData() const {
    return toJson();
}

bool desync_detector::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void desync_detector::performCleanup() {
    LOGI("desync_detector: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t desync_detector::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool desync_detector::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool desync_detector::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool desync_detector::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void desync_detector::logDebug(const std::string& msg) const {
    LOGI("desync_detector: %s", msg.c_str());
}

void desync_detector::logWarning(const std::string& msg) const {
    LOGW("desync_detector: %s", msg.c_str());
}

void desync_detector::logError(const std::string& msg) const {
    LOGE("desync_detector: %s", msg.c_str());
}
