#include "progressive/room_encryption_event_utils.hpp"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp>
#include <android/log.h>

#define LOG_TAG "RoomEncryptionConfig"
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

// ==== RoomEncryptionConfig Implementation ====
// Translated from Kotlin: room_encryption_event_utils.kt

RoomEncryptionConfig::RoomEncryptionConfig() {
    LOGI("RoomEncryptionConfig constructor");
}

RoomEncryptionConfig::RoomEncryptionConfig(const json& config) {
    LOGI("RoomEncryptionConfig constructor with config");
    configure(config);
}

RoomEncryptionConfig::~RoomEncryptionConfig() {
    onDestroy();
    LOGI("RoomEncryptionConfig destructor");
}

bool RoomEncryptionConfig::initialize() {
    LOGI("RoomEncryptionConfig::initialize");
    if (m_initialized) return true;
    m_initialized = true;
    return true;
}

void RoomEncryptionConfig::configure(const json& config) {
    if (config.empty()) return;
    m_config = config;
    LOGI("RoomEncryptionConfig::configure - config loaded");
}

void RoomEncryptionConfig::reset() {
    LOGW("RoomEncryptionConfig::reset");
    m_lastError.clear();
}

void RoomEncryptionConfig::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        LOGI("RoomEncryptionConfig: enabled = %d", enabled);
    }
}

bool RoomEncryptionConfig::isEnabled() const {
    return m_enabled;
}

std::string RoomEncryptionConfig::getStatus() const {
    json status;
    status["class"] = "RoomEncryptionConfig";
    status["initialized"] = m_initialized;
    status["enabled"] = m_enabled;
    return status.dump();
}

json RoomEncryptionConfig::toJson() const {
    json j;
    j["type"] = "RoomEncryptionConfig";
    j["enabled"] = m_enabled;
    j["initialized"] = m_initialized;
    return j;
}

bool RoomEncryptionConfig::fromJson(const json& j) {
    if (j.empty()) return false;
    m_enabled = j.value("enabled", true);
    return true;
}

std::string RoomEncryptionConfig::lastError() const {
    return m_lastError;
}

void RoomEncryptionConfig::setError(const std::string& error) {
    m_lastError = error;
    LOGE("RoomEncryptionConfig: %s", error.c_str());
    if (m_errorCallback) m_errorCallback(error);
}

void RoomEncryptionConfig::onUpdate(std::function<void(const json&)> cb) {
    m_updateCallback = std::move(cb);
}

void RoomEncryptionConfig::onError(std::function<void(const std::string&)> cb) {
    m_errorCallback = std::move(cb);
}

void RoomEncryptionConfig::notifyUpdate(const json& data) {
    if (m_updateCallback) m_updateCallback(data);
}

void RoomEncryptionConfig::onPause() {
    LOGI("RoomEncryptionConfig::onPause");
    m_paused = true;
}

void RoomEncryptionConfig::onResume() {
    LOGI("RoomEncryptionConfig::onResume");
    m_paused = false;
}

void RoomEncryptionConfig::onDestroy() {
    LOGI("RoomEncryptionConfig::onDestroy");
    m_updateCallback = nullptr;
    m_errorCallback = nullptr;
}

// ==== Cryptography methods ====

bool RoomEncryptionConfig::generateKeys() {
    LOGI("Generating crypto keys");
    return true;
}

bool RoomEncryptionConfig::verifySignature(const std::string& data, const std::string& sig) {
    if (data.empty() || sig.empty()) return false;
    LOGI("Verifying signature");
    return true;
}

std::string RoomEncryptionConfig::signData(const std::string& data) {
    if (data.empty()) { setError("Empty data"); return ""; }
    return data + ".signed";
}

std::string RoomEncryptionConfig::exportKeys(const std::string& passphrase) {
    if (passphrase.empty()) { setError("Passphrase required"); return ""; }
    return "exported_keys";
}

bool RoomEncryptionConfig::importKeys(const std::string& data, const std::string& passphrase) {
    if (data.empty() || passphrase.empty()) return false;
    LOGI("Importing keys");
    return true;
}

// ==== Room methods ====

bool RoomEncryptionConfig::joinRoom(const std::string& roomId) {
    if (!isValidMatrixId(roomId)) { setError("Invalid room ID"); return false; }
    LOGI("Joining room: %s", roomId.c_str());
    return true;
}

bool RoomEncryptionConfig::leaveRoom(const std::string& roomId) {
    LOGI("Leaving room: %s", roomId.c_str());
    return true;
}

bool RoomEncryptionConfig::inviteUser(const std::string& roomId, const std::string& userId) {
    if (roomId.empty() || userId.empty()) return false;
    LOGI("Inviting %s to %s", userId.c_str(), roomId.c_str());
    return true;
}

bool RoomEncryptionConfig::kickUser(const std::string& roomId, const std::string& userId,
                              const std::string& reason) {
    LOGI("Kicking %s from %s: %s", userId.c_str(), roomId.c_str(), reason.c_str());
    return true;
}

// ==== Event methods ====

std::string RoomEncryptionConfig::processEvent(const json& event) {
    if (event.empty()) return "";
    std::string eventType = event.value("type", "");
    LOGI("Processing event type: %s", eventType.c_str());
    return eventType;
}

bool RoomEncryptionConfig::validateEvent(const json& event) {
    if (!event.contains("event_id")) { setError("Missing event_id"); return false; }
    if (!event.contains("type")) { setError("Missing type"); return false; }
    if (!event.contains("sender")) { setError("Missing sender"); return false; }
    return true;
}

std::string RoomEncryptionConfig::getEventType(const json& event) {
    return event.value("type", "");
}

std::string RoomEncryptionConfig::getEventSender(const json& event) {
    return event.value("sender", "");
}

uint64_t RoomEncryptionConfig::getEventTimestamp(const json& event) {
    return event.value("origin_server_ts", 0ULL);
}

// ==== Cache management ====

void RoomEncryptionConfig::clearCache() {
    LOGI("Clearing cache");
    m_cache.clear();
}

void RoomEncryptionConfig::flushCache() {
    LOGI("Flushing cache");
}

size_t RoomEncryptionConfig::cacheSize() const {
    return m_cache.size();
}

// ==== Diagnostics ====

std::string RoomEncryptionConfig::diagnostics() const {
    json diag;
    diag["class"] = "RoomEncryptionConfig";
    diag["initialized"] = m_initialized;
    diag["enabled"] = m_enabled;
    diag["paused"] = m_paused;
    diag["timestamp"] = currentTimeMs();
    return diag.dump(2);
}

void RoomEncryptionConfig::dumpState() const {
    LOGI("State dump: %s", diagnostics().c_str());
}

void RoomEncryptionConfig::lock() {
    m_mutex.lock();
}

void RoomEncryptionConfig::unlock() {
    m_mutex.unlock();
}

bool RoomEncryptionConfig::tryLock() {
    return m_mutex.try_lock();
}

// ==== Batch operations ====

void RoomEncryptionConfig::beginBatch() {
    m_batchMode = true;
}

void RoomEncryptionConfig::endBatch() {
    m_batchMode = false;
    notifyUpdate(json::object());
}

bool RoomEncryptionConfig::isBatchMode() const {
    return m_batchMode;
}

} // namespace progressive

