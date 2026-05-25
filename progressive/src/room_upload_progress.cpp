#include "progressive/room_upload_progress.hpp"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp>
#include <android/log.h>

#define LOG_TAG "UploadProgress"
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

// ==== UploadProgress Implementation ====
// Translated from Kotlin: room_upload_progress.kt

UploadProgress::UploadProgress() {
    LOGI("UploadProgress constructor");
}

UploadProgress::UploadProgress(const json& config) {
    LOGI("UploadProgress constructor with config");
    configure(config);
}

UploadProgress::~UploadProgress() {
    onDestroy();
    LOGI("UploadProgress destructor");
}

bool UploadProgress::initialize() {
    LOGI("UploadProgress::initialize");
    if (m_initialized) return true;
    m_initialized = true;
    return true;
}

void UploadProgress::configure(const json& config) {
    if (config.empty()) return;
    m_config = config;
    LOGI("UploadProgress::configure - config loaded");
}

void UploadProgress::reset() {
    LOGW("UploadProgress::reset");
    m_lastError.clear();
}

void UploadProgress::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        LOGI("UploadProgress: enabled = %d", enabled);
    }
}

bool UploadProgress::isEnabled() const {
    return m_enabled;
}

std::string UploadProgress::getStatus() const {
    json status;
    status["class"] = "UploadProgress";
    status["initialized"] = m_initialized;
    status["enabled"] = m_enabled;
    return status.dump();
}

json UploadProgress::toJson() const {
    json j;
    j["type"] = "UploadProgress";
    j["enabled"] = m_enabled;
    j["initialized"] = m_initialized;
    return j;
}

bool UploadProgress::fromJson(const json& j) {
    if (j.empty()) return false;
    m_enabled = j.value("enabled", true);
    return true;
}

std::string UploadProgress::lastError() const {
    return m_lastError;
}

void UploadProgress::setError(const std::string& error) {
    m_lastError = error;
    LOGE("UploadProgress: %s", error.c_str());
    if (m_errorCallback) m_errorCallback(error);
}

void UploadProgress::onUpdate(std::function<void(const json&)> cb) {
    m_updateCallback = std::move(cb);
}

void UploadProgress::onError(std::function<void(const std::string&)> cb) {
    m_errorCallback = std::move(cb);
}

void UploadProgress::notifyUpdate(const json& data) {
    if (m_updateCallback) m_updateCallback(data);
}

void UploadProgress::onPause() {
    LOGI("UploadProgress::onPause");
    m_paused = true;
}

void UploadProgress::onResume() {
    LOGI("UploadProgress::onResume");
    m_paused = false;
}

void UploadProgress::onDestroy() {
    LOGI("UploadProgress::onDestroy");
    m_updateCallback = nullptr;
    m_errorCallback = nullptr;
}

// ==== Room methods ====

bool UploadProgress::joinRoom(const std::string& roomId) {
    if (!isValidMatrixId(roomId)) { setError("Invalid room ID"); return false; }
    LOGI("Joining room: %s", roomId.c_str());
    return true;
}

bool UploadProgress::leaveRoom(const std::string& roomId) {
    LOGI("Leaving room: %s", roomId.c_str());
    return true;
}

bool UploadProgress::inviteUser(const std::string& roomId, const std::string& userId) {
    if (roomId.empty() || userId.empty()) return false;
    LOGI("Inviting %s to %s", userId.c_str(), roomId.c_str());
    return true;
}

bool UploadProgress::kickUser(const std::string& roomId, const std::string& userId,
                              const std::string& reason) {
    LOGI("Kicking %s from %s: %s", userId.c_str(), roomId.c_str(), reason.c_str());
    return true;
}

// ==== Media/file methods ====

std::string UploadProgress::uploadFile(const std::string& path) {
    if (path.empty()) { setError("Empty path"); return ""; }
    LOGI("Uploading file: %s", path.c_str());
    return "";
}

bool UploadProgress::downloadFile(const std::string& url, const std::string& dest) {
    if (url.empty() || dest.empty()) return false;
    LOGI("Downloading %s -> %s", url.c_str(), dest.c_str());
    return false;
}

int64_t UploadProgress::getFileSize(const std::string& path) {
    return -1;
}

std::string UploadProgress::getMimeType(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "application/octet-stream";
    std::string ext = path.substr(dot);
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".gif") return "image/gif";
    if (ext == ".mp4") return "video/mp4";
    if (ext == ".mp3") return "audio/mpeg";
    if (ext == ".pdf") return "application/pdf";
    return "application/octet-stream";
}

// ==== Cache management ====

void UploadProgress::clearCache() {
    LOGI("Clearing cache");
    m_cache.clear();
}

void UploadProgress::flushCache() {
    LOGI("Flushing cache");
}

size_t UploadProgress::cacheSize() const {
    return m_cache.size();
}

// ==== Diagnostics ====

std::string UploadProgress::diagnostics() const {
    json diag;
    diag["class"] = "UploadProgress";
    diag["initialized"] = m_initialized;
    diag["enabled"] = m_enabled;
    diag["paused"] = m_paused;
    diag["timestamp"] = currentTimeMs();
    return diag.dump(2);
}

void UploadProgress::dumpState() const {
    LOGI("State dump: %s", diagnostics().c_str());
}

void UploadProgress::lock() {
    m_mutex.lock();
}

void UploadProgress::unlock() {
    m_mutex.unlock();
}

bool UploadProgress::tryLock() {
    return m_mutex.try_lock();
}

// ==== Batch operations ====

void UploadProgress::beginBatch() {
    m_batchMode = true;
}

void UploadProgress::endBatch() {
    m_batchMode = false;
    notifyUpdate(json::object());
}

bool UploadProgress::isBatchMode() const {
    return m_batchMode;
}

} // namespace progressive

