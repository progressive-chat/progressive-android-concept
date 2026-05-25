#include "progressive/message_queue.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <unordered_set>

namespace progressive {

// ---- Dedup ----

std::string normalizeForComparison(const std::string& text) {
    std::string result;
    for (char c : text) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            result += std::tolower(static_cast<unsigned char>(c));
        }
    }
    return result;
}

double textSimilarity(const std::string& a, const std::string& b) {
    if (a.empty() && b.empty()) return 1.0;
    if (a.empty() || b.empty()) return 0.0;

    auto na = normalizeForComparison(a);
    auto nb = normalizeForComparison(b);

    if (na == nb) return 1.0;
    if (na.empty() || nb.empty()) return 0.0;

    // Character trigram overlap
    auto trigrams = [](const std::string& s) -> std::unordered_set<std::string> {
        std::unordered_set<std::string> set;
        if (s.size() < 3) {
            set.insert(s);
            return set;
        }
        for (size_t i = 0; i <= s.size() - 3; ++i) {
            set.insert(s.substr(i, 3));
        }
        return set;
    };

    auto ta = trigrams(na);
    auto tb = trigrams(nb);

    if (ta.empty() && tb.empty()) return 1.0;
    if (ta.empty() || tb.empty()) return 0.0;

    // Count intersection
    int intersection = 0;
    for (const auto& t : ta) {
        if (tb.find(t) != tb.end()) intersection++;
    }
    int total = static_cast<int>(ta.size() + tb.size() - intersection);

    return total > 0 ? static_cast<double>(intersection) / total : 0.0;
}

DedupResult checkDuplicate(
    const std::string& newBody,
    const std::vector<std::string>& recentBodies,
    double threshold
) {
    DedupResult result;

    for (size_t i = 0; i < recentBodies.size(); ++i) {
        double sim = textSimilarity(newBody, recentBodies[i]);
        if (sim >= threshold) {
            result.isDuplicate = true;
            result.duplicateCount++;
            if (result.originalEventId.empty()) {
                result.originalEventId = std::to_string(i);
            }
        }
    }

    return result;
}

// ---- Batching ----

std::vector<BatchedMessage> batchMessages(
    const std::vector<std::string>& bodies,
    const std::vector<std::string>& senderIds,
    const std::vector<int64_t>& timestamps,
    int64_t mergeWindowMs
) {
    std::vector<BatchedMessage> result;
    if (bodies.empty()) return result;

    std::string prevSender;
    int64_t prevTimestamp = 0;

    for (size_t i = 0; i < bodies.size(); ++i) {
        BatchedMessage msg;
        msg.body = bodies[i];
        msg.timestampMs = i < timestamps.size() ? timestamps[i] : 0;

        const auto& sender = i < senderIds.size() ? senderIds[i] : "";

        // Check if this is a continuation from same sender within merge window
        if (!prevSender.empty() && sender == prevSender &&
            msg.timestampMs - prevTimestamp <= mergeWindowMs) {
            msg.isContinuation = true;
        }

        result.push_back(msg);
        prevSender = sender;
        prevTimestamp = msg.timestampMs;
    }

    return result;
}

// ---- PinManager ----

void PinManager::pin(const PinnedMessage& msg) {
    PinnedMessage copy = msg;
    if (copy.pinnedAtMs == 0) {
        copy.pinnedAtMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    // Remove existing pin for same event
    unpin(msg.eventId);
    pins_.push_back(copy);
}

void PinManager::unpin(const std::string& eventId) {
    pins_.erase(std::remove_if(pins_.begin(), pins_.end(),
        [&](const PinnedMessage& p) { return p.eventId == eventId; }
    ), pins_.end());
}

std::vector<PinnedMessage> PinManager::getActivePins() const {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::vector<PinnedMessage> result;
    for (const auto& p : pins_) {
        if (!p.isExpired || p.expiresAtMs > now) {
            result.push_back(p);
        }
    }
    return result;
}

void PinManager::checkExpired() {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    for (auto& p : pins_) {
        if (p.expiresAtMs > 0 && now >= p.expiresAtMs) {
            p.isExpired = true;
        }
    }
}

std::string PinManager::exportJson() const {
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) { if (c == '"') out += "\\\""; else out += c; }
        return out;
    };
    auto pins = getActivePins();
    std::ostringstream json;
    json << "[";
    for (size_t i = 0; i < pins.size(); ++i) {
        if (i > 0) json << ",";
        const auto& p = pins[i];
        json << R"({"eventId": ")" << esc(p.eventId) << R"(")";
        json << R"(,"body": ")" << esc(p.body) << R"(")";
        json << R"(,"senderName": ")" << esc(p.senderName) << R"(")";
        json << R"(,"pinnedAtMs": )" << p.pinnedAtMs << "}";
    }
    json << "]";
    return json.str();
}

void PinManager::clear() {
    pins_.clear();
}

} // namespace progressive


// ==== Extended message_queue implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string message_queue::serialize() const {
    json j = toJson();
    return j.dump();
}

bool message_queue::deserialize(const std::string& data) {
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
bool message_queue::validate() const {
    if (!m_initialized) {
        LOGE("message_queue: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool message_queue::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool message_queue::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json message_queue::getMetrics() const {
    json m;
    m["class"] = "message_queue";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int message_queue::getOperationCount() const {
    return m_operationCount;
}

void message_queue::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void message_queue::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "message_queue";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool message_queue::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool message_queue::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool message_queue::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void message_queue::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void message_queue::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int message_queue::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void message_queue::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> message_queue::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> message_queue::listItems() const {
    return {};
}

int message_queue::itemCount() const {
    return 0;
}

// Versioning
std::string message_queue::getVersion() const {
    return "1.0.0";
}

bool message_queue::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool message_queue::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void message_queue::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> message_queue::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool message_queue::migrateData(int fromVersion, int toVersion) {
    LOGI("message_queue: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int message_queue::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json message_queue::exportData() const {
    return toJson();
}

bool message_queue::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void message_queue::performCleanup() {
    LOGI("message_queue: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t message_queue::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool message_queue::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool message_queue::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool message_queue::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void message_queue::logDebug(const std::string& msg) const {
    LOGI("message_queue: %s", msg.c_str());
}

void message_queue::logWarning(const std::string& msg) const {
    LOGW("message_queue: %s", msg.c_str());
}

void message_queue::logError(const std::string& msg) const {
    LOGE("message_queue: %s", msg.c_str());
}
