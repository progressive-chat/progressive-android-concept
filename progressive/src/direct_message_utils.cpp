#include "progressive/direct_message_utils.hpp"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp>
#include <android/log.h>

#define LOG_TAG "direct_message_utils"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using json = nlohmann::json;

namespace progressive {

namespace {
// String utilities
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::istringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) result.push_back(item);
    return result;
}

static std::string toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

static bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static std::string replaceAll(std::string s, const std::string& from,
                                const std::string& to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

static bool isValidMatrixId(const std::string& id) {
    return !id.empty() && id.size() <= 255 && id.find(' ') == std::string::npos;
}

static uint64_t currentTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

static std::string generateTxnId() {
    static std::atomic<uint64_t> counter0;
    return "txn_" + std::to_string(currentTimeMs()) + "_" +
           std::to_string(counter.fetch_add(1));
}

} // anonymous namespace

// ==== direct_message_utils Implementation ====
// Translated from Kotlin: direct_message_utils.kt

direct_message_utils::direct_message_utils() {
    LOGI("direct_message_utils constructor");
}

direct_message_utils::direct_message_utils(const json& config) {
    LOGI("direct_message_utils constructor with config");
    configure(config);
}

direct_message_utils::~direct_message_utils() {
    onDestroy();
    LOGI("direct_message_utils destructor");
}

bool direct_message_utils::initialize() {
    LOGI("direct_message_utils::initialize");
    if (m_initialized) return true;
    m_initialized = true;
    return true;
}

void direct_message_utils::configure(const json& config) {
    if (config.empty()) return;
    m_config = config;
    LOGI("direct_message_utils::configure - config loaded");
}

void direct_message_utils::reset() {
    LOGW("direct_message_utils::reset");
    m_lastError.clear();
}

void direct_message_utils::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        LOGI("direct_message_utils: enabled = %d", enabled);
    }
}

bool direct_message_utils::isEnabled() const {
    return m_enabled;
}

std::string direct_message_utils::getStatus() const {
    json status;
    status["class"] = "direct_message_utils";
    status["initialized"] = m_initialized;
    status["enabled"] = m_enabled;
    return status.dump();
}

json direct_message_utils::toJson() const {
    json j;
    j["type"] = "direct_message_utils";
    j["enabled"] = m_enabled;
    j["initialized"] = m_initialized;
    return j;
}

bool direct_message_utils::fromJson(const json& j) {
    if (j.empty()) return false;
    m_enabled = j.value("enabled", true);
    return true;
}

std::string direct_message_utils::lastError() const {
    return m_lastError;
}

void direct_message_utils::setError(const std::string& error) {
    m_lastError = error;
    LOGE("direct_message_utils: %s", error.c_str());
    if (m_errorCallback) m_errorCallback(error);
}

void direct_message_utils::onUpdate(std::function<void(const json&)> cb) {
    m_updateCallback = std::move(cb);
}

void direct_message_utils::onError(std::function<void(const std::string&)> cb) {
    m_errorCallback = std::move(cb);
}

void direct_message_utils::notifyUpdate(const json& data) {
    if (m_updateCallback) m_updateCallback(data);
}

void direct_message_utils::onPause() {
    LOGI("direct_message_utils::onPause");
    m_paused = true;
}

void direct_message_utils::onResume() {
    LOGI("direct_message_utils::onResume");
    m_paused = false;
}

void direct_message_utils::onDestroy() {
    LOGI("direct_message_utils::onDestroy");
    m_updateCallback = nullptr;
    m_errorCallback = nullptr;
}

// ==== Room methods ====

bool direct_message_utils::joinRoom(const std::string& roomId) {
    if (!isValidMatrixId(roomId)) { setError("Invalid room ID"); return false; }
    LOGI("Joining room: %s", roomId.c_str());
    return true;
}

bool direct_message_utils::leaveRoom(const std::string& roomId) {
    LOGI("Leaving room: %s", roomId.c_str());
    return true;
}

bool direct_message_utils::inviteUser(const std::string& roomId, const std::string& userId) {
    if (roomId.empty() || userId.empty()) return false;
    LOGI("Inviting %s to %s", userId.c_str(), roomId.c_str());
    return true;
}

bool direct_message_utils::kickUser(const std::string& roomId, const std::string& userId,
                              const std::string& reason) {
    LOGI("Kicking %s from %s: %s", userId.c_str(), roomId.c_str(), reason.c_str());
    return true;
}

// ==== Event methods ====

std::string direct_message_utils::processEvent(const json& event) {
    if (event.empty()) return "";
    std::string eventType = event.value("type", "");
    LOGI("Processing event type: %s", eventType.c_str());
    return eventType;
}

bool direct_message_utils::validateEvent(const json& event) {
    if (!event.contains("event_id")) { setError("Missing event_id"); return false; }
    if (!event.contains("type")) { setError("Missing type"); return false; }
    if (!event.contains("sender")) { setError("Missing sender"); return false; }
    return true;
}

std::string direct_message_utils::getEventType(const json& event) {
    return event.value("type", "");
}

std::string direct_message_utils::getEventSender(const json& event) {
    return event.value("sender", "");
}

uint64_t direct_message_utils::getEventTimestamp(const json& event) {
    return event.value("origin_server_ts", 0ULL);
}

// ==== Cache management ====

void direct_message_utils::clearCache() {
    LOGI("Clearing cache");
    m_cache.clear();
}

void direct_message_utils::flushCache() {
    LOGI("Flushing cache");
}

size_t direct_message_utils::cacheSize() const {
    return m_cache.size();
}

// ==== Diagnostics ====

std::string direct_message_utils::diagnostics() const {
    json diag;
    diag["class"] = "direct_message_utils";
    diag["initialized"] = m_initialized;
    diag["enabled"] = m_enabled;
    diag["paused"] = m_paused;
    diag["timestamp"] = currentTimeMs();
    return diag.dump(2);
}

void direct_message_utils::dumpState() const {
    LOGI("State dump: %s", diagnostics().c_str());
}

void direct_message_utils::lock() {
    m_mutex.lock();
}

void direct_message_utils::unlock() {
    m_mutex.unlock();
}

bool direct_message_utils::tryLock() {
    return m_mutex.try_lock();
}

// ==== Batch operations ====

void direct_message_utils::beginBatch() {
    m_batchMode = true;
}

void direct_message_utils::endBatch() {
    m_batchMode = false;
    notifyUpdate(json::object());
}

bool direct_message_utils::isBatchMode() const {
    return m_batchMode;
}

} // namespace progressive



// ==== Extended direct_message_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string direct_message_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool direct_message_utils::deserialize(const std::string& data) {
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
bool direct_message_utils::validate() const {
    if (!m_initialized) {
        LOGE("direct_message_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool direct_message_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool direct_message_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json direct_message_utils::getMetrics() const {
    json m;
    m["class"] = "direct_message_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int direct_message_utils::getOperationCount() const {
    return m_operationCount;
}

void direct_message_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void direct_message_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "direct_message_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool direct_message_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool direct_message_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool direct_message_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void direct_message_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void direct_message_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int direct_message_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void direct_message_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> direct_message_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> direct_message_utils::listItems() const {
    return {};
}

int direct_message_utils::itemCount() const {
    return 0;
}

// Versioning
std::string direct_message_utils::getVersion() const {
    return "1.0.0";
}

bool direct_message_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool direct_message_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void direct_message_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> direct_message_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool direct_message_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("direct_message_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int direct_message_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json direct_message_utils::exportData() const {
    return toJson();
}

bool direct_message_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void direct_message_utils::performCleanup() {
    LOGI("direct_message_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t direct_message_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool direct_message_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool direct_message_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool direct_message_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void direct_message_utils::logDebug(const std::string& msg) const {
    LOGI("direct_message_utils: %s", msg.c_str());
}

void direct_message_utils::logWarning(const std::string& msg) const {
    LOGW("direct_message_utils: %s", msg.c_str());
}

void direct_message_utils::logError(const std::string& msg) const {
    LOGE("direct_message_utils: %s", msg.c_str());
}
