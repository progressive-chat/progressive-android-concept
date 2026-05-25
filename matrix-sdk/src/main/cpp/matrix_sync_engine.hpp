#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <nlohmann/json.hpp>
#include "matrix_common_types.hpp"
#include "matrix_http.hpp"

namespace matrix_sdk {

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────
//  Full /sync engine — initial sync, live sync, gap detection,
//  retry with exponential backoff, filter upload, state dispatch
// ─────────────────────────────────────────────────────────────

// ── Sync request types ──

enum class SyncRequestType {
    INITIAL,          // First sync, no since token
    LIVE,             // Long-poll sync with since token
    CATCHUP,           // Sync after disconnect (limited timeout)
    GUEST              // Guest account sync
};

enum class SyncEventType {
    TIMELINE,
    STATE,
    EPHEMERAL,
    ACCOUNT_DATA,
    TO_DEVICE,
    PRESENCE,
    DEVICE_LIST,
    ONE_TIME_KEYS,
    ROOM_MEMBER,
    NOTIFICATION,
    UNKNOWN
};

// ── Sync statistics ──

struct SyncStats {
    int64_t totalSyncs = 0;
    int64_t successfulSyncs = 0;
    int64_t failedSyncs = 0;
    int64_t totalEvents = 0;
    int64_t timelineEvents = 0;
    int64_t stateEvents = 0;
    int64_t toDeviceEvents = 0;
    int64_t accountDataEvents = 0;
    int64_t presenceEvents = 0;
    int64_t deviceListChanges = 0;
    int64_t totalLatencyMs = 0;
    int64_t maxLatencyMs = 0;
    int64_t minLatencyMs = INT64_MAX;
    int64_t lastSyncAt = 0;
    int64_t lastErrorAt = 0;
    int32_t consecutiveErrors = 0;
    int32_t consecutiveSuccesses = 0;
    double avgEventsPerSync = 0.0;
    double syncsPerMinute = 0.0;

    void reset() {
        *this = SyncStats{};
        minLatencyMs = INT64_MAX;
    }
};

// ── Gap detection result ──

struct GapResult {
    bool hasGap = false;
    std::string fromToken;
    std::string toToken;
    int64_t estimatedMissingEvents = 0;
    std::string roomId;
    int64_t detectedAt = 0;
};

// ── Sync batch ──

struct SyncBatch {
    std::string roomId;
    std::string membership;
    std::vector<json> timelineEvents;
    std::vector<json> stateEvents;
    std::vector<json> ephemeralEvents;
    std::vector<json> accountDataEvents;
    json summary;
    json unreadNotifications;
    bool limited = false;
    std::string prevBatch;
    bool hasMore = false;
};

// ── Sync result ──

struct SyncResult {
    std::string nextBatch;
    std::vector<SyncBatch> rooms;
    std::vector<json> presence;
    std::vector<json> accountData;
    std::vector<json> toDevice;
    std::vector<std::string> changedDevices;
    std::vector<std::string> leftDevices;
    json oneTimeKeyCounts;
    std::vector<GapResult> gaps;
    int64_t syncDurationMs = 0;
    int64_t timestamp = 0;
    bool isInitialSync = false;
    bool hasMore = false;
    std::string error;
};

// ── Sync engine configuration ──

struct SyncEngineConfig {
    // Connection
    std::string homeserverUrl;
    std::string accessToken;
    std::string userId;

    // Timing
    int initialSyncTimeoutMs = 0;        // 0 = no timeout (long poll for initial)
    int liveSyncTimeoutMs = 30000;       // 30s
    int catchupSyncTimeoutMs = 10000;    // 10s
    int minSyncIntervalMs = 100;         // Minimum time between syncs
    int maxSyncIntervalMs = 60000;       // Max backoff
    int initialRetryDelayMs = 2000;      // Starting retry delay

    // Retry
    int maxRetries = 3;
    double backoffMultiplier = 2.0;
    int maxBackoffMs = 120000;
    bool resetBackoffOnSuccess = true;

    // Features
    bool enablePresence = true;
    bool enableTyping = true;
    bool enableReceipts = true;
    bool enableToDevice = true;
    bool enableAccountData = true;
    bool enableLazyLoading = true;
    bool enableThreadNotifications = true;
    bool enableGapDetection = true;
    bool enableInitialSync = true;
    bool autoUploadFilter = true;
    bool requestFullState = false;

    // Filter
    int timelineLimit = 50;
    int presenceLimit = 500;
    std::vector<std::string> includedEventTypes;
    std::vector<std::string> excludedEventTypes;
    std::vector<std::string> includedRooms;
    std::vector<std::string> excludedRooms;

    // Processing
    int maxEventsPerBatch = 2000;
    int maxRoomsPerSync = 500;
    bool processEventsInBackground = true;
    int eventProcessorConcurrency = 2;
};

// ── Sync request ──

struct SyncRequest {
    SyncRequestType type = SyncRequestType::LIVE;
    std::string since;
    std::string filterId;
    int timeoutMs = 30000;
    bool fullState = false;
    std::string setPresence;
    int64_t requestId = 0;
    int64_t createdAt = 0;
};

// ── Callback types ──

using SyncResultCallback = std::function<void(const SyncResult&)>;
using SyncBatchCallback = std::function<void(const SyncBatch&)>;
using SyncEventCallback = std::function<void(SyncEventType, const std::string& roomId, const json&)>;
using SyncErrorCallback = std::function<void(const std::string& error, int retryAfterMs, bool fatal)>;
using SyncStateCallback = std::function<void(const std::string& state, const std::string& prevState)>;
using SyncStatsCallback = std::function<void(const SyncStats&)>;
using GapDetectedCallback = std::function<void(const GapResult&)>;
using SyncFilterCallback = std::function<void(const std::string& filterId)>;

// ── Sync state machine states ──

enum class SyncEngineState {
    UNINITIALIZED,
    INITIALIZING,
    INITIAL_SYNCING,
    LIVE_SYNCING,
    RECONNECTING,
    CATCHING_UP,
    PAUSED,
    STOPPING,
    STOPPED,
    FATAL_ERROR
};

inline std::string syncEngineStateName(SyncEngineState s) {
    switch (s) {
        case SyncEngineState::UNINITIALIZED:  return "UNINITIALIZED";
        case SyncEngineState::INITIALIZING:   return "INITIALIZING";
        case SyncEngineState::INITIAL_SYNCING: return "INITIAL_SYNCING";
        case SyncEngineState::LIVE_SYNCING:   return "LIVE_SYNCING";
        case SyncEngineState::RECONNECTING:   return "RECONNECTING";
        case SyncEngineState::CATCHING_UP:    return "CATCHING_UP";
        case SyncEngineState::PAUSED:         return "PAUSED";
        case SyncEngineState::STOPPING:       return "STOPPING";
        case SyncEngineState::STOPPED:        return "STOPPED";
        case SyncEngineState::FATAL_ERROR:    return "FATAL_ERROR";
    }
    return "UNKNOWN";
}

// ── Sync engine ──

class MatrixSyncEngine {
public:
    MatrixSyncEngine();
    explicit MatrixSyncEngine(const SyncEngineConfig& config);
    ~MatrixSyncEngine();

    // ── Configuration ──
    void configure(const SyncEngineConfig& config);
    const SyncEngineConfig& config() const { return m_config; }
    void setAccessToken(const std::string& token);
    void setHomeserverUrl(const std::string& url);
    void setUserId(const std::string& userId);
    void setNextBatch(const std::string& nextBatch);
    const std::string& nextBatch() const { return m_nextBatch; }

    // ── Lifecycle ──
    bool initialize();
    bool start(const std::string& since = "");
    bool startInitialSync();
    void stop();
    void pause();
    void resume();
    void restart(const std::string& since = "");
    void shutdown();

    // ── State ──
    SyncEngineState state() const { return m_state.load(); }
    bool isRunning() const { return m_running.load(); }
    bool isPaused() const { return m_paused.load(); }
    bool isLiveSyncing() const {
        return m_state.load() == SyncEngineState::LIVE_SYNCING;
    }

    // ── Sync execution ──
    SyncResult performSync(const SyncRequest& request);
    SyncResult performInitialSync();
    SyncResult performLiveSync(int timeoutMs = 30000);
    SyncResult performCatchupSync(int timeoutMs = 10000);

    // ── Filter management ──
    bool uploadFilter(const json& filterDefinition);
    bool downloadFilter(const std::string& filterId);
    bool deleteFilter(const std::string& filterId);
    json buildDefaultFilter();
    json buildRoomFilter(const std::string& roomId);
    std::string currentFilterId() const { return m_filterId; }

    // ── Gap detection ──
    std::vector<GapResult> detectGaps(const SyncResult& result);
    GapResult detectRoomGap(const std::string& roomId,
                             const std::string& prevBatch,
                             const std::string& currentBatch);
    void fillGap(const GapResult& gap);
    void setGapThreshold(int maxGapEvents);

    // ── Callbacks ──
    void setOnSyncResult(SyncResultCallback cb) { m_resultCallback = std::move(cb); }
    void setOnSyncBatch(SyncBatchCallback cb) { m_batchCallback = std::move(cb); }
    void setOnSyncEvent(SyncEventCallback cb) { m_eventCallback = std::move(cb); }
    void setOnError(SyncErrorCallback cb) { m_errorCallback = std::move(cb); }
    void setOnStateChanged(SyncStateCallback cb) { m_stateCallback = std::move(cb); }
    void setOnStats(SyncStatsCallback cb) { m_statsCallback = std::move(cb); }
    void setOnGapDetected(GapDetectedCallback cb) { m_gapCallback = std::move(cb); }
    void setOnFilterId(SyncFilterCallback cb) { m_filterIdCallback = std::move(cb); }

    // ── HTTP injection ──
    void setHttpClient(std::shared_ptr<MatrixHttpClient> client);

    // ── Statistics ──
    SyncStats getStats() const;
    double syncsPerMinute() const;
    double avgSyncLatency() const;
    int64_t totalEventsProcessed() const;
    int64_t uptimeMs() const;
    void resetStats();

    // ── Event processing ──
    void setEventProcessor(std::function<void(const json&)> processor);
    int pendingEventCount() const;
    void clearPendingEvents();

    // ── Network awareness ──
    void onNetworkAvailable();
    void onNetworkLost();
    bool isNetworkAvailable() const { return m_networkAvailable.load(); }

    // ── Diagnostics ──
    json diagnostics() const;
    std::string lastError() const { return m_lastError; }
    int64_t lastSyncAt() const { return m_lastSyncAt.load(); }
    int32_t consecutiveErrors() const { return m_consecutiveErrors.load(); }

private:
    // ── Internal sync loop ──
    void syncLoop();
    void initialSyncLoop();
    void liveSyncLoop();
    void catchupLoop();
    void reconnectingLoop();

    // ── Sync request/response ──
    SyncRequest buildSyncRequest(SyncRequestType type);
    SyncResult executeSyncRequest(const SyncRequest& request);
    SyncResult parseSyncResponse(const json& rawResponse, const SyncRequest& request);

    // ── Response processing ──
    void processSyncResult(const SyncResult& result);
    void processJoinedRooms(const json& rooms, std::vector<SyncBatch>& batches);
    void processInvitedRooms(const json& rooms, std::vector<SyncBatch>& batches);
    void processLeftRooms(const json& rooms, std::vector<SyncBatch>& batches);
    SyncBatch parseRoomSync(const std::string& roomId, const std::string& membership,
                             const json& roomData);
    void processTimelineEvents(const std::string& roomId, const json& timeline);
    void processStateEvents(const std::string& roomId, const json& state);
    void processEphemeralEvents(const std::string& roomId, const json& ephemeral);
    void processAccountData(const json& accountData);
    void processToDevice(const json& toDevice);
    void processPresence(const json& presence);
    void processDeviceLists(const json& deviceLists, const json& otkCounts);

    // ── State management ──
    void setState(SyncEngineState newState);
    void transitionTo(SyncEngineState target);
    bool canTransition(SyncEngineState from, SyncEngineState to);

    // ── Retry logic ──
    int64_t computeRetryDelay();
    bool shouldRetry(int attempt, const std::string& error);
    void resetRetryState();
    bool isRetryableError(int httpStatus);

    // ── Filter ──
    std::string ensureFilterUploaded();

    // ── URL building ──
    std::string buildSyncUrl(const std::string& since, int timeout,
                              const std::string& filterId, bool fullState);
    std::string buildFilterUrl(const std::string& userId = "");
    std::string buildFilterByIdUrl(const std::string& filterId);

    // ── Utility ──
    SyncStats computeStats() const;
    void recordSync(const SyncResult& result);
    void enforceRateLimit();
    int64_t nowMs() const;

    // ── Members ──
    SyncEngineConfig m_config;
    std::atomic<SyncEngineState> m_state{SyncEngineState::UNINITIALIZED};
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_paused{false};
    std::atomic<bool> m_shuttingDown{false};
    std::atomic<bool> m_networkAvailable{true};
    std::string m_nextBatch;
    std::string m_filterId;
    std::string m_lastError;
    std::string m_userId;

    std::thread m_syncThread;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::condition_variable m_pauseCv;

    // HTTP
    std::shared_ptr<MatrixHttpClient> m_httpClient;

    // Stats
    std::atomic<int64_t> m_totalSyncs{0};
    std::atomic<int64_t> m_successfulSyncs{0};
    std::atomic<int64_t> m_failedSyncs{0};
    std::atomic<int64_t> m_totalEvents{0};
    std::atomic<int64_t> m_lastSyncAt{0};
    std::atomic<int32_t> m_consecutiveErrors{0};
    std::chrono::steady_clock::time_point m_startTime;
    std::vector<int64_t> m_recentLatencies;

    // Retry
    int m_retryAttempt = 0;
    int m_currentRetryDelayMs = 0;
    int m_maxRetryDelayMs = 120000;

    // Event queue
    std::vector<json> m_pendingEvents;
    std::function<void(const json&)> m_eventProcessor;

    // Callbacks
    SyncResultCallback m_resultCallback;
    SyncBatchCallback m_batchCallback;
    SyncEventCallback m_eventCallback;
    SyncErrorCallback m_errorCallback;
    SyncStateCallback m_stateCallback;
    SyncStatsCallback m_statsCallback;
    GapDetectedCallback m_gapCallback;
    SyncFilterCallback m_filterIdCallback;
};

// ── Sync engine factory ──

std::shared_ptr<MatrixSyncEngine> createSyncEngine(const SyncEngineConfig& config);

// ── Sync utilities ──

namespace sync_utils {

    // Parse a /sync response and extract structured data
    SyncResult parseSyncResponse(const json& raw, int64_t startTime);

    // Extract all room IDs from a sync response
    std::vector<std::string> extractRoomIds(const json& syncResponse);

    // Detect if a sync response indicates the need for initial sync
    bool needsInitialSync(const std::string& nextBatch, int64_t lastSyncAge);

    // Validate a sync token
    bool isValidSyncToken(const std::string& token);

    // Estimate total event count from a sync chunk
    int estimateEventCount(const json& syncResponse);

    // Build a sync filter targeting specific rooms
    json buildTargetedFilter(const std::vector<std::string>& roomIds,
                              int timelineLimit = 20);

    // Merge two sync results (for catch-up)
    SyncResult mergeSyncResults(const SyncResult& first, const SyncResult& second);

} // namespace sync_utils

} // namespace matrix_sdk
