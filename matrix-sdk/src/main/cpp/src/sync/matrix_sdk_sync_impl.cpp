#include "matrix_sdk/sync/matrix_sdk_sync_service.hpp"
#include "matrix_sdk/matrix_sdk_types.hpp"
#include "matrix_sdk/network/matrix_sdk_network.hpp"
#include <nlohmann/json.hpp>
#include <android/log.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>

#define TAG "MatrixSync"

namespace matrix_sdk {

using json = nlohmann::json;

// ── SyncServiceImpl ──
class SyncServiceImpl : public SyncService {
public:
    explicit SyncServiceImpl(std::shared_ptr<HttpClient> httpClient)
        : httpClient_(std::move(httpClient))
        , state_(SyncState::IDLE)
        , active_(false)
        , syncing_(false)
        , syncTimeoutMs_(30000)
        , syncDelayMs_(0)
        , initialSyncDone_(false)
        , backgroundSyncEnabled_(false)
        , backgroundSyncPeriodMs_(60000)
        , pendingRoomCount_(0)
        , currentRetryDelayMs_(1000)
        , maxRetryDelayMs_(300000)  // 5 min max backoff
        , baseRetryDelayMs_(1000)
        , consecutiveFailures_(0) {
    }

    ~SyncServiceImpl() override {
        stop();
    }

    // ── Lifecycle ──
    void start() override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (active_) return;
        active_ = true;
        state_ = SyncState::IDLE;
        __android_log_print(ANDROID_LOG_INFO, TAG, "Sync service started");

        // Start the polling thread
        syncThread_ = std::thread(&SyncServiceImpl::pollingLoop, this);
    }

    void stop() override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!active_) return;
            active_ = false;
            state_ = SyncState::TERMINATED;
            __android_log_print(ANDROID_LOG_INFO, TAG, "Sync service stopping");
        }
        cv_.notify_all();

        if (syncThread_.joinable()) {
            syncThread_.join();
        }
    }

    void pause() override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == SyncState::SYNCING) {
            state_ = SyncState::PAUSED;
            __android_log_print(ANDROID_LOG_INFO, TAG, "Sync service paused");
        }
    }

    void resume() override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == SyncState::PAUSED) {
            state_ = SyncState::IDLE;
            __android_log_print(ANDROID_LOG_INFO, TAG, "Sync service resumed");
            cv_.notify_all();
        }
    }

    bool isActive() const override {
        return active_;
    }

    SyncState getState() const override {
        return state_.load();
    }

    // ── Callbacks ──
    void setSyncCallback(SyncCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        syncCallback_ = std::move(callback);
    }

    void setErrorCallback(ErrorCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        errorCallback_ = std::move(callback);
    }

    // ── Sync Parameters ──
    void setSyncTimeout(int timeoutMs) override {
        std::lock_guard<std::mutex> lock(mutex_);
        syncTimeoutMs_ = std::max(1000, timeoutMs);
    }

    int getSyncTimeout() const override {
        return syncTimeoutMs_;
    }

    void setSyncDelay(int delayMs) override {
        std::lock_guard<std::mutex> lock(mutex_);
        syncDelayMs_ = std::max(0, delayMs);
    }

    int getSyncDelay() const override {
        return syncDelayMs_;
    }

    void setFilter(const SyncFilter& filter) override {
        std::lock_guard<std::mutex> lock(mutex_);
        currentFilter_ = filter;
    }

    SyncFilter getFilter() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return currentFilter_;
    }

    void setFilterId(const std::string& filterId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        currentFilterId_ = filterId;
    }

    std::string getFilterId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return currentFilterId_;
    }

    // ── Token ──
    std::string getCurrentToken() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return nextBatchToken_;
    }

    void setNextBatchToken(const std::string& token) override {
        std::lock_guard<std::mutex> lock(mutex_);
        nextBatchToken_ = token;
    }

    // ── Initial Sync ──
    void initialSync(std::function<void(bool, const std::string&)> callback) override {
        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client");
            return;
        }

        __android_log_print(ANDROID_LOG_INFO, TAG, "Performing initial sync...");
        syncing_ = true;
        state_ = SyncState::SYNCING;

        json params;
        params["timeout"] = syncTimeoutMs_;
        if (!currentFilterId_.empty()) {
            params["filter"] = currentFilterId_;
        }
        // Initial sync: don't send 'since' token
        std::string url = buildSyncUrl(params);

        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = url,
            .timeoutMs = syncTimeoutMs_ + 5000
        };

        httpClient_->execute(request, [this, callback](const HttpResponse& response) {
            processSyncResponse(response, [callback, this](bool success, const std::string& error) {
                syncing_ = false;
                state_ = SyncState::IDLE;
                if (success) {
                    initialSyncDone_ = true;
                }
                if (callback) callback(success, error);
            });
        });
    }

    bool hasPerformedInitialSync() const override {
        return initialSyncDone_;
    }

    // ── Filter ──
    void uploadFilter(const SyncFilter& filter,
            std::function<void(bool, const std::string&, const std::string&)> callback) override {
        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client", "");
            return;
        }

        json filterJson = filter.toJson();
        std::string currentUserId;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            currentUserId = userId_;
        }

        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = "/_matrix/client/r0/user/" + currentUserId + "/filter",
            .timeoutMs = 15000
        };
        request.body = filterJson.dump();
        request.contentType = "application/json";

        httpClient_->execute(request, [this, callback](const HttpResponse& response) {
            if (!response.isSuccess()) {
                if (callback) callback(false, "HTTP " + std::to_string(response.httpCode), "");
                return;
            }
            try {
                auto j = json::parse(response.body);
                std::string filterId = j.value("filter_id", "");
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    currentFilterId_ = filterId;
                }
                if (callback) callback(true, "", filterId);
            } catch (const std::exception& e) {
                if (callback) callback(false, std::string("Parse error: ") + e.what(), "");
            }
        });
    }

    // ── Statistics ──
    SyncStats getStats() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return stats_;
    }

    void resetStats() override {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_ = SyncStats{};
    }

    // ── Background Sync ──
    void enableBackgroundSync(bool enabled) override {
        std::lock_guard<std::mutex> lock(mutex_);
        backgroundSyncEnabled_ = enabled;
        __android_log_print(ANDROID_LOG_INFO, TAG, "Background sync %s", enabled ? "enabled" : "disabled");
    }

    bool isBackgroundSyncEnabled() const override {
        return backgroundSyncEnabled_;
    }

    void setBackgroundSyncPeriod(int periodMs) override {
        std::lock_guard<std::mutex> lock(mutex_);
        backgroundSyncPeriodMs_ = std::max(10000, periodMs); // minimum 10 seconds
    }

    // ── Live Status ──
    bool isSyncing() const override {
        return syncing_;
    }

    int getPendingRoomCount() const override {
        return pendingRoomCount_;
    }

    int64_t getLastSyncTimestamp() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return stats_.lastSyncTimestamp;
    }

    // ── Additional configuration for Session integration ──
    void setUserId(const std::string& userId) {
        std::lock_guard<std::mutex> lock(mutex_);
        userId_ = userId;
    }

    void setAccessToken(const std::string& token) {
        std::lock_guard<std::mutex> lock(mutex_);
        accessToken_ = token;
    }

    void setHomeserverUrl(const std::string& url) {
        std::lock_guard<std::mutex> lock(mutex_);
        homeserverUrl_ = url;
        // Ensure URL doesn't end with slash
        if (!homeserverUrl_.empty() && homeserverUrl_.back() == '/') {
            homeserverUrl_.pop_back();
        }
    }

    void setCryptoService(std::shared_ptr<CryptoService> crypto) {
        std::lock_guard<std::mutex> lock(mutex_);
        cryptoService_ = std::move(crypto);
    }

private:
    // ── Polling Loop ──
    void pollingLoop() {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Polling loop started");

        while (active_) {
            {
                std::unique_lock<std::mutex> lock(mutex_);

                // Wait if paused
                cv_.wait(lock, [this] {
                    return !active_ || state_ != SyncState::PAUSED;
                });

                if (!active_) break;

                // Skip if already syncing
                if (syncing_) {
                    cv_.wait_for(lock, std::chrono::milliseconds(100));
                    continue;
                }

                // Respect delay between syncs
                if (syncDelayMs_ > 0 && lastSyncTime_.time_since_epoch().count() > 0) {
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - lastSyncTime_).count();
                    if (elapsed < syncDelayMs_) {
                        cv_.wait_for(lock, std::chrono::milliseconds(syncDelayMs_ - elapsed));
                        if (!active_) break;
                    }
                }

                // Apply retry backoff delay
                if (currentRetryDelayMs_ > baseRetryDelayMs_) {
                    __android_log_print(ANDROID_LOG_INFO, TAG,
                        "Backoff wait: %d ms", currentRetryDelayMs_);
                    cv_.wait_for(lock, std::chrono::milliseconds(currentRetryDelayMs_));
                    if (!active_) break;
                }

                syncing_ = true;
                state_ = SyncState::SYNCING;
            }

            // Perform sync
            performSync();

            lastSyncTime_ = std::chrono::steady_clock::now();

            // Wait before next poll (background sync mode)
            if (active_ && backgroundSyncEnabled_ && !syncing_) {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait_for(lock, std::chrono::milliseconds(backgroundSyncPeriodMs_));
            }
        }

        __android_log_print(ANDROID_LOG_INFO, TAG, "Polling loop ended");
    }

    void performSync() {
        if (!httpClient_) {
            syncing_ = false;
            state_ = SyncState::ERROR;
            return;
        }

        json params;
        params["timeout"] = syncTimeoutMs_;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!nextBatchToken_.empty()) {
                params["since"] = nextBatchToken_;
            }
            if (!currentFilterId_.empty()) {
                params["filter"] = currentFilterId_;
            }
        }

        std::string url = buildSyncUrl(params);
        __android_log_print(ANDROID_LOG_VERBOSE, TAG, "Sync request: %s", url.c_str());

        auto startTime = std::chrono::steady_clock::now();
        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = url,
            .timeoutMs = syncTimeoutMs_ + 10000,
        };

        httpClient_->execute(request, [this, startTime](const HttpResponse& response) {
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

            processSyncResponse(response, [this, duration](bool success, const std::string& error) {
                // Update statistics
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    stats_.totalSyncs++;
                    stats_.lastSyncTimestamp = std::chrono::system_clock::now().time_since_epoch().count() / 1000000;
                    stats_.lastSyncDurationMs = duration;
                    stats_.totalSyncDurationMs += duration;

                    if (stats_.totalSyncs > 0) {
                        stats_.averageSyncDurationMs = stats_.totalSyncDurationMs / stats_.totalSyncs;
                    }

                    if (success) {
                        stats_.successfulSyncs++;
                        stats_.lastSuccessfulSyncTimestamp = stats_.lastSyncTimestamp;
                        // Reset backoff on success
                        currentRetryDelayMs_ = baseRetryDelayMs_;
                        consecutiveFailures_ = 0;
                    } else {
                        stats_.failedSyncs++;
                        // Exponential backoff
                        consecutiveFailures_++;
                        currentRetryDelayMs_ = std::min(
                            baseRetryDelayMs_ * (1 << std::min(consecutiveFailures_, 10)),
                            maxRetryDelayMs_);
                        // Add jitter (±25%)
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<> jitter(-currentRetryDelayMs_ / 4, currentRetryDelayMs_ / 4);
                        currentRetryDelayMs_ = std::max(baseRetryDelayMs_,
                            currentRetryDelayMs_ + jitter(gen));

                        if (errorCallback_) {
                            MatrixError me;
                            me.isNetworkError = true;
                            me.errorMessage = error;
                            me.retryAfterMs = currentRetryDelayMs_;
                            errorCallback_(me);
                        }
                        state_ = SyncState::ERROR;
                        __android_log_print(ANDROID_LOG_WARN, TAG,
                            "Sync failed (attempt %d), backoff: %d ms, error: %s",
                            consecutiveFailures_, currentRetryDelayMs_, error.c_str());
                    }
                }

                syncing_ = false;
                if (state_ == SyncState::SYNCING) {
                    state_ = SyncState::IDLE;
                }
            });
        });
    }

    void processSyncResponse(const HttpResponse& response,
            std::function<void(bool, const std::string&)> callback) {
        if (!response.isSuccess()) {
            int httpCode = response.httpCode;
            // Handle rate limiting
            if (httpCode == 429) {
                try {
                    auto j = json::parse(response.body);
                    int retryAfter = j.value("retry_after_ms", 5000);
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        currentRetryDelayMs_ = std::max(currentRetryDelayMs_, retryAfter);
                    }
                } catch (...) {}
            }
            if (callback) callback(false, "HTTP " + std::to_string(httpCode));
            return;
        }

        try {
            auto j = json::parse(response.body);
            SyncResponse syncResponse;
            syncResponse.timestamp = std::chrono::system_clock::now().time_since_epoch().count() / 1000000;
            syncResponse.nextBatch = j.value("next_batch", "");

            // Parse presence
            if (j.contains("presence")) {
                syncResponse.presenceJson = j["presence"].dump();
            }

            // Parse account data
            if (j.contains("account_data")) {
                syncResponse.accountDataJson = j["account_data"].dump();
            }

            // Parse device lists
            if (j.contains("device_lists")) {
                syncResponse.deviceListsJson = j["device_lists"].dump();
            }

            // Parse device one-time keys count
            if (j.contains("device_one_time_keys_count")) {
                syncResponse.deviceOneTimeKeysCount = j["device_one_time_keys_count"].dump();
            }

            // Parse to_device events
            if (j.contains("to_device")) {
                syncResponse.toDeviceJson = j["to_device"].dump();
                int64_t toDeviceEvents = 0;
                if (j["to_device"].contains("events")) {
                    toDeviceEvents = j["to_device"]["events"].size();
                }
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    stats_.totalEventsProcessed += toDeviceEvents;
                }
            }

            // Parse rooms
            if (j.contains("rooms")) {
                json rooms = j["rooms"];
                int64_t totalRoomEvents = 0;

                // Joined rooms
                if (rooms.contains("join")) {
                    for (auto& [roomId, roomData] : rooms["join"].items()) {
                        syncResponse.joinedRooms[roomId] = roomData.dump();
                        if (roomData.contains("timeline") && roomData["timeline"].contains("events")) {
                            totalRoomEvents += roomData["timeline"]["events"].size();
                        }
                        // Count pending (unread) rooms
                        if (roomData.contains("unread_notifications")) {
                            auto unread = roomData["unread_notifications"];
                            if (unread.contains("notification_count") && unread["notification_count"].get<int>() > 0) {
                                pendingRoomCount_++;
                            }
                        }
                    }
                }

                // Invited rooms
                if (rooms.contains("invite")) {
                    for (auto& [roomId, roomData] : rooms["invite"].items()) {
                        syncResponse.invitedRooms[roomId] = roomData.dump();
                    }
                    pendingRoomCount_ += rooms["invite"].size();
                }

                // Left rooms
                if (rooms.contains("leave")) {
                    for (auto& [roomId, roomData] : rooms["leave"].items()) {
                        syncResponse.leftRooms[roomId] = roomData.dump();
                    }
                }

                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    stats_.totalEventsProcessed += totalRoomEvents;
                }
            }

            // Update next batch token
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (!syncResponse.nextBatch.empty()) {
                    nextBatchToken_ = syncResponse.nextBatch;
                }
            }

            // Notify crypto service
            if (cryptoService_) {
                cryptoService_->onSyncCompleted(syncResponse.nextBatch);
            }

            // Deliver to callback
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (syncCallback_) {
                    syncCallback_(syncResponse);
                }
            }

            if (callback) callback(true, "");
        } catch (const std::exception& e) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Sync parse error: %s", e.what());
            if (callback) callback(false, std::string("Parse error: ") + e.what());
        }
    }

    std::string buildSyncUrl(const json& params) {
        std::string url = "/_matrix/client/r0/sync?";
        bool first = true;
        for (auto& [key, value] : params.items()) {
            if (!first) url += "&";
            first = false;
            if (value.is_number()) {
                url += key + "=" + std::to_string(value.get<int>());
            } else if (value.is_string()) {
                url += key + "=" + value.get<std::string>();
            }
        }
        return url;
    }

    std::shared_ptr<HttpClient> httpClient_;
    std::shared_ptr<CryptoService> cryptoService_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::thread syncThread_;

    std::atomic<SyncState> state_;
    std::atomic<bool> active_;
    std::atomic<bool> syncing_;
    std::atomic<bool> initialSyncDone_;
    std::atomic<bool> backgroundSyncEnabled_;
    std::atomic<int> pendingRoomCount_;

    int syncTimeoutMs_;
    int syncDelayMs_;
    int backgroundSyncPeriodMs_;

    // Retry / backoff
    int currentRetryDelayMs_;
    int maxRetryDelayMs_;
    int baseRetryDelayMs_;
    int consecutiveFailures_;

    std::string userId_;
    std::string accessToken_;
    std::string homeserverUrl_;
    std::string nextBatchToken_;
    std::string currentFilterId_;
    SyncFilter currentFilter_;

    SyncCallback syncCallback_;
    ErrorCallback errorCallback_;
    SyncStats stats_;

    std::chrono::steady_clock::time_point lastSyncTime_;
};

// ── Factory function ──
std::shared_ptr<SyncService> createSyncService(std::shared_ptr<HttpClient> httpClient) {
    return std::make_shared<SyncServiceImpl>(std::move(httpClient));
}

} // namespace matrix_sdk
