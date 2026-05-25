#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <nlohmann/json.hpp>
#include "matrix_common_types.hpp"

namespace matrix_sdk {

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────
//  Full /sync loop with filter management, live and initial
//  Equivalent to DefaultSyncService in matrix-sdk-android
// ─────────────────────────────────────────────────────────────

// ── Sync filter ──

struct SyncFilter {
    json roomFilter;
    json presenceFilter;
    json accountDataFilter;
    int timelineLimit = 50;
    bool includePresence = true;
    bool includeAccountData = true;
    bool lazyLoadMembers = true;
    bool unreadThreadNotifications = true;
    std::vector<std::string> includedRooms;
    std::vector<std::string> excludedRooms;
    std::vector<std::string> includedEventTypes;
    std::vector<std::string> excludedEventTypes;

    std::string filterId; // serverside filter ID if uploaded

    json toJson() const;
    static SyncFilter fromJson(const json& j);
};

// ── Sync state ──

enum class SyncState {
    IDLE,
    INITIAL_SYNC,
    RUNNING,
    PAUSED,
    RECONNECTING,
    STOPPED,
    ERROR
};

inline std::string syncStateToString(SyncState state) {
    switch (state) {
        case SyncState::IDLE:         return "IDLE";
        case SyncState::INITIAL_SYNC: return "INITIAL_SYNC";
        case SyncState::RUNNING:      return "RUNNING";
        case SyncState::PAUSED:       return "PAUSED";
        case SyncState::RECONNECTING: return "RECONNECTING";
        case SyncState::STOPPED:      return "STOPPED";
        case SyncState::ERROR:        return "ERROR";
    }
    return "UNKNOWN";
}

// ── Sync configuration ──

struct SyncConfig {
    int initialTimeoutMs = 0;        // 0 = long-polling for initial sync
    int liveTimeoutMs = 30000;       // 30s long-poll
    int reconnectBackoffMs = 2000;
    int maxReconnectBackoffMs = 60000;
    double backoffMultiplier = 2.0;
    int maxSyncAttempts = -1;        // -1 = unlimited
    bool fullState = false;          // request full state
    bool enablePresence = true;
    bool enableTyping = true;
    bool enableReceipts = true;
    bool enableToDevice = true;
    bool autoUploadFilter = true;
};

// ── Sync progress ──

struct SyncProgress {
    std::string nextBatch;
    int timelineEvents = 0;
    int stateEvents = 0;
    int accountDataEvents = 0;
    int toDeviceEvents = 0;
    int deviceListChanges = 0;
    int oneTimeKeyCounts = 0;
    int presenceEvents = 0;
    int64_t syncDurationMs = 0;
    int64_t totalSyncTimeMs = 0;
    int syncCount = 0;
    bool isInitialSync = false;
};

// ── Sync response ──

struct SyncResponse {
    std::string nextBatch;
    std::string presenceJson;
    std::string accountDataJson;
    std::string toDeviceJson;
    std::string deviceListsJson;
    std::string deviceOneTimeKeysCountJson;
    std::string groupsJson;

    struct RoomInfo {
        std::string state;
        std::string timeline;
        std::string ephemeral;
        std::string accountData;
        std::string summary;
        std::string unreadNotifications;
    };
    std::map<std::string, RoomInfo> joinedRooms;
    std::map<std::string, RoomInfo> invitedRooms;
    std::map<std::string, RoomInfo> leftRooms;

    static SyncResponse fromJson(const json& j);
};

// ── Sync event callbacks ──

using SyncStateChangeCallback = std::function<void(SyncState, SyncState)>;
using TimelineEventCallback = std::function<void(const std::string& roomId, const json& event)>;
using StateEventCallback = std::function<void(const std::string& roomId, const std::string& type,
                                               const std::string& stateKey, const json& content)>;
using AccountDataCallback = std::function<void(const std::string& type, const json& content)>;
using ToDeviceCallback = std::function<void(const std::string& type, const json& content)>;
using PresenceCallback = std::function<void(const std::string& userId, const std::string& presence)>;
using DeviceListChangeCallback = std::function<void(const std::vector<std::string>& changed,
                                                     const std::vector<std::string>& left)>;
using SyncProgressCallback = std::function<void(const SyncProgress& progress)>;
using SyncErrorCallback = std::function<void(const std::string& error, int retryInMs)>;
using JoinedRoomCallback = std::function<void(const std::string& roomId, const json& state)>;
using InvitedRoomCallback = std::function<void(const std::string& roomId, const json& state)>;
using LeftRoomCallback = std::function<void(const std::string& roomId)>;

// ── Sync service ──

class MatrixSyncLoop {
public:
    MatrixSyncLoop();
    ~MatrixSyncLoop();

    // ── Configuration ──
    void setConfig(const SyncConfig& config);
    const SyncConfig& config() const { return m_config; }
    void setFilter(const SyncFilter& filter);
    const SyncFilter& filter() const { return m_filter; }
    void setNextBatch(const std::string& since);
    const std::string& nextBatch() const { return m_nextBatch; }

    // ── Lifecycle ──
    bool start(const std::string& since = "");
    void stop();
    void pause();
    void resume();
    void restart();
    bool isRunning() const { return m_running.load(); }
    SyncState state() const { return m_state.load(); }

    // ── HTTP provider (dependency injection) ──
    using HttpGetFunc = std::function<json(const std::string&,
                                            const std::map<std::string, std::string>&)>;
    using HttpPostFunc = std::function<json(const std::string&, const json&)>;
    void setHttpGet(HttpGetFunc func) { m_httpGet = std::move(func); }
    void setHttpPost(HttpPostFunc func) { m_httpPost = std::move(func); }

    // ── Filter management ──
    bool uploadFilter();
    bool downloadFilter(const std::string& filterId);
    bool deleteFilter(const std::string& filterId);
    std::string filterId() const { return m_filter.filterId; }

    // ── Sync execution ──
    SyncResponse performSyncRequest(const std::string& since = "",
                                     int timeout = 30000,
                                     const std::string& filterId = "");
    SyncResponse performInitialSync();

    // ── Callbacks ──
    void setOnStateChanged(SyncStateChangeCallback cb) {
        m_stateChangeCallback = std::move(cb);
    }
    void setOnTimelineEvent(TimelineEventCallback cb) {
        m_timelineCallback = std::move(cb);
    }
    void setOnStateEvent(StateEventCallback cb) {
        m_stateEventCallback = std::move(cb);
    }
    void setOnAccountData(AccountDataCallback cb) {
        m_accountDataCallback = std::move(cb);
    }
    void setOnToDevice(ToDeviceCallback cb) {
        m_toDeviceCallback = std::move(cb);
    }
    void setOnPresence(PresenceCallback cb) {
        m_presenceCallback = std::move(cb);
    }
    void setOnDeviceListChange(DeviceListChangeCallback cb) {
        m_deviceListCallback = std::move(cb);
    }
    void setOnProgress(SyncProgressCallback cb) {
        m_progressCallback = std::move(cb);
    }
    void setOnError(SyncErrorCallback cb) {
        m_errorCallback = std::move(cb);
    }
    void setOnJoinedRoom(JoinedRoomCallback cb) {
        m_joinedRoomCallback = std::move(cb);
    }
    void setOnInvitedRoom(InvitedRoomCallback cb) {
        m_invitedRoomCallback = std::move(cb);
    }
    void setOnLeftRoom(LeftRoomCallback cb) {
        m_leftRoomCallback = std::move(cb);
    }

    // ── Statistics ──
    SyncProgress getProgress() const { return m_progress; }
    int64_t totalSyncCount() const { return m_totalSyncCount.load(); }
    int64_t lastSyncDurationMs() const { return m_lastSyncDurationMs.load(); }
    int64_t uptimeMs() const;
    double syncsPerMinute() const;

    // ── Request builder ──
    std::string buildSyncUrl(const std::string& since = "",
                              int timeout = 30000,
                              const std::string& filterId = "");

    // ── Network awareness ──
    void onNetworkAvailable();
    void onNetworkLost();

private:
    void syncLoop();
    void processSyncResponse(const SyncResponse& response);
    void processJoinedRooms(const std::map<std::string, SyncResponse::RoomInfo>& rooms);
    void processInvitedRooms(const std::map<std::string, SyncResponse::RoomInfo>& rooms);
    void processLeftRooms(const std::map<std::string, SyncResponse::RoomInfo>& rooms);
    void processTimeline(const std::string& roomId, const std::string& timelineJson);
    void processState(const std::string& roomId, const std::string& stateJson);
    void processEphemeral(const std::string& roomId, const std::string& ephemeralJson);
    void processAccountDataSync(const std::string& accountDataJson);
    void processToDevice(const std::string& toDeviceJson);
    void processPresence(const std::string& presenceJson);
    void processDeviceLists(const std::string& deviceListsJson,
                            const std::string& otkCountJson);
    void notifyStateChange(SyncState newState);
    int64_t computeRetryDelay(int attempt);
    bool shouldRetry(int attempt);

    SyncConfig m_config;
    SyncFilter m_filter;
    std::atomic<SyncState> m_state{SyncState::IDLE};
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_paused{false};
    std::atomic<bool> m_networkAvailable{true};
    std::string m_nextBatch;
    SyncProgress m_progress;
    std::atomic<int64_t> m_totalSyncCount{0};
    std::atomic<int64_t> m_lastSyncDurationMs{0};
    std::chrono::steady_clock::time_point m_startTime;

    std::thread m_syncThread;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;

    // HTTP functions (injected)
    HttpGetFunc m_httpGet;
    HttpPostFunc m_httpPost;

    // Callbacks
    SyncStateChangeCallback m_stateChangeCallback;
    TimelineEventCallback m_timelineCallback;
    StateEventCallback m_stateEventCallback;
    AccountDataCallback m_accountDataCallback;
    ToDeviceCallback m_toDeviceCallback;
    PresenceCallback m_presenceCallback;
    DeviceListChangeCallback m_deviceListCallback;
    SyncProgressCallback m_progressCallback;
    SyncErrorCallback m_errorCallback;
    JoinedRoomCallback m_joinedRoomCallback;
    InvitedRoomCallback m_invitedRoomCallback;
    LeftRoomCallback m_leftRoomCallback;

    // Retry tracking
    int m_syncAttempt = 0;
    int m_retryDelayMs = 0;
};

// ── Sync helpers ──

namespace sync_helpers {

    // Parse sync response from raw JSON
    SyncResponse parseSyncResponse(const json& raw);

    // Extract events from timeline chunk
    std::vector<json> extractTimelineEvents(const json& timelineChunk);

    // Extract state events
    std::vector<json> extractStateEvents(const json& stateChunk);

    // Extract ephemeral events (typing, receipts)
    struct EphemeralEvents {
        std::vector<json> typing;
        std::vector<json> receipts;
    };
    EphemeralEvents extractEphemeralEvents(const json& ephemeralChunk);

    // Build sync query parameters
    std::map<std::string, std::string> buildSyncParams(const std::string& since,
                                                         int timeout,
                                                         const std::string& filterId,
                                                         bool fullState);

    // Validate a sync token
    bool isValidSyncToken(const std::string& token);

    // Compare two sync tokens to detect gaps
    bool isTokenContiguous(const std::string& prev, const std::string& current);

} // namespace sync_helpers

} // namespace matrix_sdk
