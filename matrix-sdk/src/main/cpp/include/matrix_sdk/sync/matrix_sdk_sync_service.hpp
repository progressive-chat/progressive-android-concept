#ifndef MATRIX_SDK_SYNC_SERVICE_HPP
#define MATRIX_SDK_SYNC_SERVICE_HPP

#include "matrix_sdk_types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace matrix_sdk {

// ── Sync Response ──
struct SyncResponse {
    std::string nextBatch;
    std::string presenceJson;
    std::string accountDataJson;
    std::string deviceListsJson;
    std::string deviceOneTimeKeysCount;
    std::string toDeviceJson;
    std::string groupsJson;
    std::map<std::string, std::string> joinedRooms;    // roomId -> json
    std::map<std::string, std::string> invitedRooms;   // roomId -> json
    std::map<std::string, std::string> leftRooms;      // roomId -> json
    int64_t timestamp = 0;
};

// ── Room Sync ──
struct RoomSync {
    std::string roomId;
    std::string stateJson;
    std::string timelineJson;
    std::string ephemeralJson;
    std::string accountDataJson;
    std::string unreadNotificationsJson;
    std::string summaryJson;
    std::string prevBatch;
    bool limited = false;
    int numJoinedMembers = 0;
};

// ── Invited Room Sync ──
struct InvitedRoomSync {
    std::string roomId;
    std::string inviteStateJson;
};

// ── Left Room Sync ──
struct LeftRoomSync {
    std::string roomId;
    std::string timelineJson;
    std::string stateJson;
    std::string accountDataJson;
};

// ── Sync Service ──
class SyncService {
public:
    using SyncCallback = std::function<void(const SyncResponse& response)>;
    using ErrorCallback = std::function<void(const MatrixError& error)>;
    using ProgressCallback = std::function<void(int progress, int total)>;

    virtual ~SyncService() = default;

    // ── Lifecycle ──
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual bool isActive() const = 0;
    virtual SyncState getState() const = 0;

    // ── Callbacks ──
    virtual void setSyncCallback(SyncCallback callback) = 0;
    virtual void setErrorCallback(ErrorCallback callback) = 0;

    // ── Sync Parameters ──
    virtual void setSyncTimeout(int timeoutMs) = 0;
    virtual int getSyncTimeout() const = 0;
    virtual void setSyncDelay(int delayMs) = 0;
    virtual int getSyncDelay() const = 0;
    virtual void setFilter(const SyncFilter& filter) = 0;
    virtual SyncFilter getFilter() const = 0;
    virtual void setFilterId(const std::string& filterId) = 0;
    virtual std::string getFilterId() const = 0;

    // ── Token ──
    virtual std::string getCurrentToken() const = 0;
    virtual void setNextBatchToken(const std::string& token) = 0;

    // ── Initial Sync ──
    virtual void initialSync(std::function<void(bool, const std::string&)> callback) = 0;
    virtual bool hasPerformedInitialSync() const = 0;

    // ── Filter ──
    virtual void uploadFilter(const SyncFilter& filter,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;

    // ── Statistics ──
    struct SyncStats {
        int totalSyncs = 0;
        int successfulSyncs = 0;
        int failedSyncs = 0;
        int64_t lastSyncTimestamp = 0;
        int64_t lastSuccessfulSyncTimestamp = 0;
        int64_t totalEventsProcessed = 0;
        int64_t averageSyncDurationMs = 0;
        int64_t lastSyncDurationMs = 0;
        int64_t totalSyncDurationMs = 0;
    };
    virtual SyncStats getStats() const = 0;
    virtual void resetStats() = 0;

    // ── Background Sync ──
    virtual void enableBackgroundSync(bool enabled) = 0;
    virtual bool isBackgroundSyncEnabled() const = 0;
    virtual void setBackgroundSyncPeriod(int periodMs) = 0;

    // ── Live Status ──
    virtual bool isSyncing() const = 0;
    virtual int getPendingRoomCount() const = 0;
    virtual int64_t getLastSyncTimestamp() const = 0;
};

} // namespace matrix_sdk

#endif
