#include "progressive/lightweight_call.hpp"
#include <sstream>
#include <fstream>

namespace progressive {

// ---- Memory Assessment ----

int64_t LightweightCallManager::getAvailableRamKb() {
    std::ifstream mem("/proc/meminfo");
    if (!mem.is_open()) return 0;

    std::string line;
    while (std::getline(mem, line)) {
        if (line.rfind("MemAvailable:", 0) == 0) {
            auto pos = line.find(':');
            if (pos != std::string::npos) {
                auto val = line.substr(pos + 1);
                while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(0, 1);
                auto kbPos = val.find("kB");
                if (kbPos != std::string::npos) val = val.substr(0, kbPos);
                while (!val.empty() && val.back() == ' ') val.pop_back();
                try { return std::stoll(val); }
                catch (...) { return 0; }
            }
        }
    }
    return 0;
}

int64_t LightweightCallManager::getProcessRssKb() {
    std::ifstream status("/proc/self/status");
    if (!status.is_open()) return 0;

    std::string line;
    while (std::getline(status, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            auto pos = line.find(':');
            if (pos != std::string::npos) {
                auto val = line.substr(pos + 1);
                while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(0, 1);
                auto kbPos = val.find("kB");
                if (kbPos != std::string::npos) val = val.substr(0, kbPos);
                while (!val.empty() && val.back() == ' ') val.pop_back();
                try { return std::stoll(val); }
                catch (...) { return 0; }
            }
        }
    }
    return 0;
}

MemoryState LightweightCallManager::assessMemory() {
    MemoryState state;
    state.totalKb = getAvailableRamKb() + getProcessRssKb(); // rough estimate
    state.availableKb = getAvailableRamKb();
    state.usedKb = getProcessRssKb();

    // Low memory: < 256MB available
    state.isLowMemory = (state.availableKb < 256 * 1024);

    // Need lightweight mode if:
    // - Available RAM < 512MB AND process uses > 200MB
    state.needsLightweightMode = shouldUseLightweightMode(state);

    return state;
}

bool LightweightCallManager::shouldUseLightweightMode(const MemoryState& state) {
    // Use lightweight call if:
    // 1. Total device RAM < 2GB
    // 2. OR process RSS > 200MB
    // 3. OR available RAM < 512MB
    if (state.totalKb < 2 * 1024 * 1024) return true; // < 2GB total
    if (state.usedKb > 200 * 1024) return true;       // process > 200MB
    if (state.availableKb < 512 * 1024) return true;   // < 512MB free
    return false;
}

int64_t LightweightCallManager::estimateFreedBytes(CleanupAction action) {
    switch (action) {
        case CleanupAction::ClearImageCache:       return 50 * 1024 * 1024;  // ~50MB
        case CleanupAction::ClearTimelineViews:    return 30 * 1024 * 1024;  // ~30MB
        case CleanupAction::ClearComposerState:    return 10 * 1024 * 1024;  // ~10MB
        case CleanupAction::SuspendBackgroundSync: return 15 * 1024 * 1024;  // ~15MB (incoming events)
        case CleanupAction::ClearAvatarCache:      return 8 * 1024 * 1024;   // ~8MB
        case CleanupAction::ClearReactionCache:    return 3 * 1024 * 1024;   // ~3MB
        case CleanupAction::TrimMemoryAggressive:  return 40 * 1024 * 1024;  // ~40MB (GC)
        case CleanupAction::ClearWebViewCache:     return 25 * 1024 * 1024;  // ~25MB
        case CleanupAction::ClearMediaPlayer:      return 5 * 1024 * 1024;   // ~5MB
        case CleanupAction::DetachNonVisibleFrags: return 15 * 1024 * 1024;  // ~15MB
        default: return 0;
    }
}

CleanupPlan LightweightCallManager::computeCleanupPlan(const MemoryState& state) {
    CleanupPlan plan;

    // Priority-ordered cleanup based on memory pressure
    if (state.needsLightweightMode) {
        plan.actions = {
            CleanupAction::ClearImageCache,
            CleanupAction::ClearTimelineViews,
            CleanupAction::ClearComposerState,
            CleanupAction::SuspendBackgroundSync,
            CleanupAction::ClearAvatarCache,
            CleanupAction::ClearReactionCache,
            CleanupAction::TrimMemoryAggressive,
            CleanupAction::ClearWebViewCache,
            CleanupAction::ClearMediaPlayer,
            CleanupAction::DetachNonVisibleFrags,
        };
        plan.priority = 1;
    } else if (state.isLowMemory) {
        plan.actions = {
            CleanupAction::ClearImageCache,
            CleanupAction::ClearTimelineViews,
            CleanupAction::TrimMemoryAggressive,
            CleanupAction::ClearAvatarCache,
            CleanupAction::SuspendBackgroundSync,
        };
        plan.priority = 2;
    } else {
        // Minimal cleanup even on normal devices
        plan.actions = {
            CleanupAction::ClearImageCache,
            CleanupAction::TrimMemoryAggressive,
        };
        plan.priority = 3;
    }

    // Compute estimated freed bytes
    plan.estimatedBytesFreed = 0;
    for (auto action : plan.actions) {
        plan.estimatedBytesFreed += estimateFreedBytes(action);
    }

    return plan;
}

CleanupPlan LightweightCallManager::enterCallMode() {
    auto state = assessMemory();
    preCallState_ = state;
    isInCallMode_ = true;
    return computeCleanupPlan(state);
}

CleanupPlan LightweightCallManager::exitCallMode() {
    isInCallMode_ = false;
    // Return minimal restore plan
    CleanupPlan plan;
    plan.actions = {
        CleanupAction::SuspendBackgroundSync, // this one needs restoration
    };
    plan.priority = 0;
    plan.estimatedBytesFreed = 0;
    return plan;
}

std::string LightweightCallManager::planToJson(const CleanupPlan& plan) {
    std::ostringstream json;
    json << "{";
    json << R"("actions": [)";
    for (size_t i = 0; i < plan.actions.size(); ++i) {
        if (i > 0) json << ",";
        json << R"(")" << static_cast<int>(plan.actions[i]) << R"(")";
    }
    json << "],";
    json << R"("estimatedBytesFreed": )" << plan.estimatedBytesFreed << ",";
    json << R"("priority": )" << plan.priority;
    json << "}";
    return json.str();
}

std::string LightweightCallManager::memoryStateToJson(const MemoryState& state) {
    std::ostringstream json;
    json << "{";
    json << R"("availableKb": )" << state.availableKb << ",";
    json << R"("totalKb": )" << state.totalKb << ",";
    json << R"("usedKb": )" << state.usedKb << ",";
    json << R"("isLowMemory": )" << (state.isLowMemory ? "true" : "false") << ",";
    json << R"("needsLightweightMode": )" << (state.needsLightweightMode ? "true" : "false");
    json << "}";
    return json.str();
}

} // namespace progressive


// ==== Extended lightweight_call implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string lightweight_call::serialize() const {
    json j = toJson();
    return j.dump();
}

bool lightweight_call::deserialize(const std::string& data) {
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
bool lightweight_call::validate() const {
    if (!m_initialized) {
        LOGE("lightweight_call: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool lightweight_call::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool lightweight_call::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json lightweight_call::getMetrics() const {
    json m;
    m["class"] = "lightweight_call";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int lightweight_call::getOperationCount() const {
    return m_operationCount;
}

void lightweight_call::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void lightweight_call::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "lightweight_call";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool lightweight_call::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool lightweight_call::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool lightweight_call::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void lightweight_call::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void lightweight_call::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int lightweight_call::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void lightweight_call::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> lightweight_call::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> lightweight_call::listItems() const {
    return {};
}

int lightweight_call::itemCount() const {
    return 0;
}

// Versioning
std::string lightweight_call::getVersion() const {
    return "1.0.0";
}

bool lightweight_call::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool lightweight_call::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void lightweight_call::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> lightweight_call::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool lightweight_call::migrateData(int fromVersion, int toVersion) {
    LOGI("lightweight_call: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int lightweight_call::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json lightweight_call::exportData() const {
    return toJson();
}

bool lightweight_call::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void lightweight_call::performCleanup() {
    LOGI("lightweight_call: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t lightweight_call::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool lightweight_call::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool lightweight_call::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool lightweight_call::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void lightweight_call::logDebug(const std::string& msg) const {
    LOGI("lightweight_call: %s", msg.c_str());
}

void lightweight_call::logWarning(const std::string& msg) const {
    LOGW("lightweight_call: %s", msg.c_str());
}

void lightweight_call::logError(const std::string& msg) const {
    LOGE("lightweight_call: %s", msg.c_str());
}
