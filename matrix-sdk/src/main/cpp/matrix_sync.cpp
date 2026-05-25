#include "matrix_sync.hpp"
#include <thread>
#include <android/log.h>

#define LOG_TAG "MatrixSync"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

MatrixSync::MatrixSync() = default;
MatrixSync::~MatrixSync() { stop(); }

void MatrixSync::start(const std::string& since, int timeoutMs) {
    if (m_state == SyncState::RUNNING) return;
    m_state = SyncState::RUNNING;
    if (!since.empty()) m_nextBatch = since;
    LOGI("Sync started with timeout=%dms", timeoutMs);
    fetchSync(timeoutMs);
}

void MatrixSync::stop() { m_state = SyncState::STOPPING; }
void MatrixSync::pause() { if (m_state == SyncState::RUNNING) m_state = SyncState::PAUSED; }
void MatrixSync::resume() { if (m_state == SyncState::PAUSED) m_state = SyncState::RUNNING; }

bool MatrixSync::fetchSync(int timeoutMs) {
    if (m_homeserverUrl.empty() || m_accessToken.empty()) {
        LOGE("Sync: missing homeserver URL or access token");
        m_state = SyncState::ERRORED;
        return false;
    }
    // In real implementation: HTTP GET to /_matrix/client/v3/sync
    // with query params: since, timeout, filter
    // This is a stub.
    return true;
}

std::string MatrixSync::buildFilterJson() {
    json filter = {
        {"presence", json::object({{"types", json::array({"m.presence"})}})},
        {"account_data", json::object({{"types", json::array({"*"})}})},
        {"room", {
            {"timeline", {{"limit", m_filter.timelineLimit}}},
            {"state", {{"types", json::array({"*"})}}},
            {"ephemeral", {{"types", json::array({"m.typing", "m.receipt"})}}},
            {"account_data", {{"types", json::array({"*"})}}}
        }}
    };
    return filter.dump();
}

} // namespace matrix_sdk
