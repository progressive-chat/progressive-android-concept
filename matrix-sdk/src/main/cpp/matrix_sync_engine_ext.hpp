#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────
//  Sync Filter — constructs Matrix filter JSON for /sync
// ─────────────────────────────────────────────────────────────

struct SyncFilter {
    // Timeline filter
    std::vector<std::string> notTypes;          // exclude event types
    std::vector<std::string> notSenders;        // exclude senders
    int limit = 50;                              // events per room
    bool lazyLoadMembers = true;

    // Room filter
    std::vector<std::string> rooms;              // specific rooms to sync
    std::vector<std::string> notRooms;           // rooms to exclude
    bool includeLeave = false;                   // include left rooms
    bool includeArchived = false;                // archived/forgotten

    // Account data filter
    std::vector<std::string> accountDataTypes;   // specific account data types

    // Presence filter
    std::vector<std::string> presenceSenders;    // filter presence by user
    bool includePresence = true;
    std::vector<std::string> presenceTypes;      // "m.presence", etc.

    // State filter
    std::vector<std::string> stateTypes;         // specific state event types
    bool includeState = true;

    // Ephemeral filter
    bool includeTyping = true;
    bool includeReceipts = true;
    bool includeFullyRead = true;
    int receiptLimit = 20;
};

// ─────────────────────────────────────────────────────────────
//  Sync Engine Configuration
// ─────────────────────────────────────────────────────────────

struct SyncEngineExtConfig {
    std::string homeserverUrl;
    std::string accessToken;
    std::string userId;
    std::string deviceId;

    // Timing
    int initialSyncTimeoutMs = 0;         // 0 = long-poll, >0 = timeout
    int longPollTimeoutMs = 30000;        // default long-poll timeout
    int minBackoffMs = 1000;              // minimum backoff
    int maxBackoffMs = 300000;            // maximum backoff (5 min)
    int backoffMultiplier = 2;            // exponential multiplier
    int maxConsecutiveErrors = 10;        // after this, fully stop

    // Retry behavior
    bool retryOnNetworkError = true;
    bool retryOnServerError = true;       // 5xx
    bool retryOnRateLimit = true;         // 429
    int maxRateLimitWaitMs = 120000;      // max wait on 429

    // Sync scope
    std::string initialFilterId;
    SyncFilter filter;
    bool fullState = false;               // request full state
    bool setPresenceOnline = true;        // set presence=online during sync
    std::string presenceStatus = "online";

    // Room discovery
    bool autoDiscoverRooms = true;        // discover joined rooms
    bool autoDiscoverSpaces = true;       // discover spaces
    bool trackLeftRooms = true;           // track left/banned rooms
    bool fetchRoomSummaries = true;       // fetch room hero/summary data

    // Recovery
    bool storeNextBatchOnDisk = true;     // persist next_batch
    bool recoverFromStoredBatch = true;   // restart from stored batch
    std::string nextBatchStoragePath;
};

// ─────────────────────────────────────────────────────────────
//  Sync State
// ─────────────────────────────────────────────────────────────

enum class SyncEngineExtState : uint8_t {
    UNINITIALIZED,
    INITIALIZING,
    CONNECTING,
    SYNCING,
    PROCESSING,
    IDLE,
    BACKING_OFF,
    ERROR,
    PAUSED,
    SHUTDOWN,
    RECONNECTING
};

struct SyncStats {
    int64_t requestCount = 0;
    int64_t successCount = 0;
    int64_t errorCount = 0;
    int64_t retryCount = 0;
    int64_t eventsProcessed = 0;
    int64_t roomsUpdated = 0;
    int64_t totalLatencyMs = 0;
    int64_t lastSyncDurationMs = 0;
    int64_t lastSyncTimestamp = 0;
    int64_t bytesReceived = 0;
    int currentBackoffMs = 0;
    int consecutiveErrors = 0;
    std::string lastError;
    std::string lastRequestId;
    std::chrono::steady_clock::time_point syncStartTime;
    std::chrono::steady_clock::time_point lastSuccessfulSync;
};

// ─────────────────────────────────────────────────────────────
//  Room Discovery — structures for discovered rooms
// ─────────────────────────────────────────────────────────────

struct DiscoveredRoom {
    std::string roomId;
    std::string name;
    std::string topic;
    std::string avatarUrl;
    std::string canonicalAlias;
    std::string joinRule;
    std::string guestAccess;
    std::string historyVisibility;
    std::string roomType;               // "m.space" or empty
    bool isEncrypted = false;
    bool isDM = false;
    bool isSpace = false;
    std::string spaceParent;
    int numJoinedMembers = 0;
    std::vector<std::string> heroes;    // room hero user IDs
    int notificationCount = 0;
    int highlightCount = 0;
    int64_t lastActivityTs = 0;
};

struct DiscoveredSpace {
    std::string spaceId;
    std::string name;
    std::string topic;
    std::string avatarUrl;
    std::vector<std::string> childRooms;
    std::vector<std::string> childSpaces;
    int totalChildRooms = 0;
    bool restricted = false;
};

// ─────────────────────────────────────────────────────────────
//  Sync Response — parsed /sync response structures
// ─────────────────────────────────────────────────────────────

struct SyncResponseRoom {
    std::string roomId;
    struct Timeline {
        std::vector<json> events;
        bool limited = false;
        std::string prevBatch;
    };
    struct State {
        std::vector<json> events;
    };
    struct Ephemeral {
        std::vector<json> events;
    };
    struct AccountData {
        std::vector<json> events;
    };
    Timeline timeline;
    State state;
    Ephemeral ephemeral;
    AccountData accountData;
    int notificationCount = 0;
    int highlightCount = 0;
    uint64_t unreadNotifications = 0;
};

struct ParsedSyncResponse {
    std::string nextBatch;
    std::unordered_map<std::string, SyncResponseRoom> joined;
    std::unordered_map<std::string, SyncResponseRoom> invited;
    std::unordered_map<std::string, SyncResponseRoom> knocked;
    std::unordered_map<std::string, SyncResponseRoom> left;
    std::vector<json> presence;
    std::vector<json> accountData;
    std::vector<json> toDevice;
    std::vector<json> deviceLists;
    std::vector<std::string> deviceOneTimeKeysCount;
    int64_t syncTimestamp = 0;
    int64_t serverTimestamp = 0;
};

// ─────────────────────────────────────────────────────────────
//  Sync dispatcher interface — callbacks for sync events
// ─────────────────────────────────────────────────────────────

struct SyncDispatcher {
    // Room events
    std::function<void(const std::string& roomId, const json& event)> onRoomEvent;
    std::function<void(const std::string& roomId, const std::vector<json>& stateEvents)> onRoomState;
    std::function<void(const std::string& roomId, const std::string& type, const json& ephemeral)> onEphemeral;

    // Room lifecycle
    std::function<void(const DiscoveredRoom& room)> onRoomJoined;
    std::function<void(const DiscoveredRoom& room)> onRoomCreated;
    std::function<void(const std::string& roomId, const std::string& membership)> onRoomLeft;
    std::function<void(const std::string& roomId, const json& inviteEvent)> onInvite;

    // Space discovery
    std::function<void(const DiscoveredSpace& space)> onSpaceDiscovered;
    std::function<void(const std::string& spaceId, const std::string& childRoomId)> onSpaceChildAdded;

    // Presence
    std::function<void(const std::string& userId, const json& presenceEvent)> onPresence;

    // Account data
    std::function<void(const std::string& type, const json& data)> onAccountData;

    // To-device
    std::function<void(const json& toDeviceEvent)> onToDevice;

    // Device list changes
    std::function<void(const std::vector<std::string>& changed)> onDeviceListChanged;
    std::function<void(const std::vector<std::string>& left)> onDeviceListLeft;

    // Sync lifecycle
    std::function<void(const ParsedSyncResponse& response)> onSyncComplete;
    std::function<void(const SyncStats& stats)> onStatsUpdate;
    std::function<void(SyncEngineExtState from, SyncEngineExtState to)> onStateChange;
    std::function<void(const std::string& error)> onError;
    std::function<void(int64_t retryAfterMs)> onBackoff;
};

// ─────────────────────────────────────────────────────────────
//  SyncEngineExt — main engine class
// ─────────────────────────────────────────────────────────────

class SyncEngineExt {
public:
    SyncEngineExt();
    explicit SyncEngineExt(const SyncEngineExtConfig& config);
    ~SyncEngineExt();

    // ── Configuration ──

    void configure(const SyncEngineExtConfig& config);
    void setAccessToken(const std::string& token);
    void setHomeserverUrl(const std::string& url);
    void setUserId(const std::string& userId);
    void setNextBatch(const std::string& nextBatch);
    void setFilter(const SyncFilter& filter);
    void setFilterId(const std::string& filterId);
    void setPresence(const std::string& presence, const std::string& statusMsg = "");
    void updateBackoffLimits(int minMs, int maxMs, int multiplier);

    // ── Filter management ──

    std::string buildFilterJson() const;
    std::string uploadFilter(const std::string& filterJson);  // returns filter_id
    bool downloadFilter(const std::string& filterId, SyncFilter& outFilter);
    bool deleteFilter(const std::string& filterId);
    std::string getActiveFilterId() const { return m_activeFilterId; }

    // ── Lifecycle ──

    bool initialize();
    bool start(const std::string& since = "");
    void pause();
    void resume();
    void shutdown();
    bool isRunning() const;
    SyncEngineExtState state() const { return m_state.load(); }

    // ── Sync control ──

    void requestImmediateSync();          // wake from backoff immediately
    void requestFullStateSync();           // next sync includes full state
    void requestIncrementalSync();         // normal incremental sync
    void setSyncTimeout(int timeoutMs);
    void forceReconnect();

    // ── Room discovery ──

    std::vector<DiscoveredRoom> getDiscoveredRooms() const;
    DiscoveredRoom getDiscoveredRoom(const std::string& roomId) const;
    std::vector<DiscoveredSpace> getDiscoveredSpaces() const;
    std::vector<std::string> getJoinedRoomIds() const;
    std::vector<std::string> getInvitedRoomIds() const;
    std::vector<std::string> getLeftRoomIds() const;
    size_t getRoomCount() const;
    void refreshRoomList();

    // ── Stats ──

    SyncStats getStats() const;
    std::string getNextBatch() const { return m_nextBatch; }
    int64_t getLastSyncTimestamp() const { return m_lastSyncTimestamp; }
    int getCurrentBackoffMs() const;
    double getSuccessRate() const;

    // ── Dispatcher ──

    SyncDispatcher& dispatcher() { return m_dispatcher; }
    void setDispatcher(const SyncDispatcher& d) { m_dispatcher = d; }

private:
    SyncEngineExtConfig m_config;
    SyncFilter m_activeFilterObj;
    std::string m_userId;
    std::string m_nextBatch;
    std::string m_activeFilterId;
    std::atomic<SyncEngineExtState> m_state{SyncEngineExtState::UNINITIALIZED};
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_paused{false};
    std::atomic<bool> m_fullStateRequested{false};
    std::atomic<bool> m_immediateSyncRequested{false};
    std::atomic<int64_t> m_lastSyncTimestamp{0};
    std::atomic<int64_t> m_requestIdCounter{0};

    SyncStats m_stats;
    SyncDispatcher m_dispatcher;

    // HTTP transport callback
    std::function<json(const std::string& method, const std::string& path,
                       const json& body, const std::map<std::string, std::string>& params,
                       int timeoutMs)> m_httpTransport;

    // Threading
    std::thread m_syncThread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_wakeRequested{false};

    // Thread-safe room tracking
    mutable std::mutex m_roomMutex;
    std::unordered_map<std::string, DiscoveredRoom> m_discoveredRooms;
    std::unordered_map<std::string, DiscoveredSpace> m_discoveredSpaces;
    std::vector<std::string> m_joinedRoomIds;
    std::vector<std::string> m_invitedRoomIds;
    std::vector<std::string> m_leftRoomIds;

    // ── Internal methods ──

    void syncLoop();
    bool executeSync(const std::string& since, int timeoutMs, const std::string& filter);
    std::string buildSyncUrl(const std::string& since, int timeoutMs, const std::string& filter) const;
    int computeBackoffDelay(int currentBackoffMs);
    void processSyncResponse(const json& responseJson);
    void handleSyncError(int statusCode, const std::string& errorBody);
    void discoverRooms(const json& rooms, const std::string& membership);
    void discoverSpaces();
    void persistNextBatch(const std::string& nextBatch);
    std::string loadStoredNextBatch() const;
    void updateStatsSuccess(int64_t latencyMs, int64_t bytesReceived);
    void updateStatsError(const std::string& error);
    void updateStatsRetry();
    void notifyStateChange(SyncEngineExtState from, SyncEngineExtState to);

    // ── HTTP transport setter ──

public:
    void setHttpTransport(
        std::function<json(const std::string&, const std::string&, const json&,
                           const std::map<std::string, std::string>&, int)> transport) {
        m_httpTransport = std::move(transport);
    }

    // ── Static helpers ──

    static std::string stateToString(SyncEngineExtState s);
    static SyncFilter filterFromJson(const json& j);
    static json filterToJson(const SyncFilter& f);
};

} // namespace matrix_sdk
