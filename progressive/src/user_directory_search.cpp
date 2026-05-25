#include "progressive/user_directory_search.hpp"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp>
#include <android/log.h>

#define LOG_TAG "user_directory_search"
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

// ==== user_directory_search Implementation ====
// Translated from Kotlin: user_directory_search.kt

user_directory_search::user_directory_search() {
    LOGI("user_directory_search constructor");
}

user_directory_search::user_directory_search(const json& config) {
    LOGI("user_directory_search constructor with config");
    configure(config);
}

user_directory_search::~user_directory_search() {
    onDestroy();
    LOGI("user_directory_search destructor");
}

bool user_directory_search::initialize() {
    LOGI("user_directory_search::initialize");
    if (m_initialized) return true;
    m_initialized = true;
    return true;
}

void user_directory_search::configure(const json& config) {
    if (config.empty()) return;
    m_config = config;
    LOGI("user_directory_search::configure - config loaded");
}

void user_directory_search::reset() {
    LOGW("user_directory_search::reset");
    m_lastError.clear();
}

void user_directory_search::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        LOGI("user_directory_search: enabled = %d", enabled);
    }
}

bool user_directory_search::isEnabled() const {
    return m_enabled;
}

std::string user_directory_search::getStatus() const {
    json status;
    status["class"] = "user_directory_search";
    status["initialized"] = m_initialized;
    status["enabled"] = m_enabled;
    return status.dump();
}

json user_directory_search::toJson() const {
    json j;
    j["type"] = "user_directory_search";
    j["enabled"] = m_enabled;
    j["initialized"] = m_initialized;
    return j;
}

bool user_directory_search::fromJson(const json& j) {
    if (j.empty()) return false;
    m_enabled = j.value("enabled", true);
    return true;
}

std::string user_directory_search::lastError() const {
    return m_lastError;
}

void user_directory_search::setError(const std::string& error) {
    m_lastError = error;
    LOGE("user_directory_search: %s", error.c_str());
    if (m_errorCallback) m_errorCallback(error);
}

void user_directory_search::onUpdate(std::function<void(const json&)> cb) {
    m_updateCallback = std::move(cb);
}

void user_directory_search::onError(std::function<void(const std::string&)> cb) {
    m_errorCallback = std::move(cb);
}

void user_directory_search::notifyUpdate(const json& data) {
    if (m_updateCallback) m_updateCallback(data);
}

void user_directory_search::onPause() {
    LOGI("user_directory_search::onPause");
    m_paused = true;
}

void user_directory_search::onResume() {
    LOGI("user_directory_search::onResume");
    m_paused = false;
}

void user_directory_search::onDestroy() {
    LOGI("user_directory_search::onDestroy");
    m_updateCallback = nullptr;
    m_errorCallback = nullptr;
}

// ==== Room methods ====

bool user_directory_search::joinRoom(const std::string& roomId) {
    if (!isValidMatrixId(roomId)) { setError("Invalid room ID"); return false; }
    LOGI("Joining room: %s", roomId.c_str());
    return true;
}

bool user_directory_search::leaveRoom(const std::string& roomId) {
    LOGI("Leaving room: %s", roomId.c_str());
    return true;
}

bool user_directory_search::inviteUser(const std::string& roomId, const std::string& userId) {
    if (roomId.empty() || userId.empty()) return false;
    LOGI("Inviting %s to %s", userId.c_str(), roomId.c_str());
    return true;
}

bool user_directory_search::kickUser(const std::string& roomId, const std::string& userId,
                              const std::string& reason) {
    LOGI("Kicking %s from %s: %s", userId.c_str(), roomId.c_str(), reason.c_str());
    return true;
}

// ==== User/profile methods ====

std::string user_directory_search::getDisplayName(const std::string& userId) {
    if (userId.empty()) return "";
    return userId;
}

std::string user_directory_search::getAvatarUrl(const std::string& userId) {
    if (userId.empty()) return "";
    return "";
}

bool user_directory_search::setDisplayName(const std::string& name) {
    if (name.empty()) return false;
    LOGI("Setting display name: %s", name.c_str());
    return true;
}

// ==== Search methods ====

std::vector<std::string> user_directory_search::search(const std::string& query, int maxResults) {
    std::vector<std::string> results;
    if (query.empty() || maxResults <= 0) return results;
    LOGI("Searching: %s (max %d)", query.c_str(), maxResults);
    return results;
}

bool user_directory_search::match(const std::string& query, const std::string& target) {
    return toLower(target).find(toLower(query)) != std::string::npos;
}

// ==== Cache management ====

void user_directory_search::clearCache() {
    LOGI("Clearing cache");
    m_cache.clear();
}

void user_directory_search::flushCache() {
    LOGI("Flushing cache");
}

size_t user_directory_search::cacheSize() const {
    return m_cache.size();
}

// ==== Diagnostics ====

std::string user_directory_search::diagnostics() const {
    json diag;
    diag["class"] = "user_directory_search";
    diag["initialized"] = m_initialized;
    diag["enabled"] = m_enabled;
    diag["paused"] = m_paused;
    diag["timestamp"] = currentTimeMs();
    return diag.dump(2);
}

void user_directory_search::dumpState() const {
    LOGI("State dump: %s", diagnostics().c_str());
}

void user_directory_search::lock() {
    m_mutex.lock();
}

void user_directory_search::unlock() {
    m_mutex.unlock();
}

bool user_directory_search::tryLock() {
    return m_mutex.try_lock();
}

// ==== Batch operations ====

void user_directory_search::beginBatch() {
    m_batchMode = true;
}

void user_directory_search::endBatch() {
    m_batchMode = false;
    notifyUpdate(json::object());
}

bool user_directory_search::isBatchMode() const {
    return m_batchMode;
}

} // namespace progressive

