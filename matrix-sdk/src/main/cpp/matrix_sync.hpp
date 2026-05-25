#pragma once
#include <string>
#include <functional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann::json;

enum class SyncState {
    IDLE,
    RUNNING,
    PAUSED,
    ERRORED,
    STOPPING
};

struct SyncFilter {
    bool includePresence = true;
    bool includeAccountData = true;
    bool includeRoomMembers = true;
    bool includeTyping = false;
    std::vector<std::string> roomIds;
    std::vector<std::string> notRoomIds;
    int timelineLimit = 50;
};

struct SyncResponse {
    std::string nextBatch;
    json presence;
    json accountData;
    json rooms;
    json toDevice;
    json deviceLists;
    json deviceOneTimeKeysCount;
};

class MatrixSync {
public:
    MatrixSync();
    ~MatrixSync();

    void start(const std::string& since = "", int timeoutMs = 30000);
    void stop();
    void pause();
    void resume();
    SyncState state() const { return m_state; }

    void setFilter(const SyncFilter& filter) { m_filter = filter; }
    void setHomeserverUrl(const std::string& url) { m_homeserverUrl = url; }
    void setAccessToken(const std::string& token) { m_accessToken = token; }

    void onSyncResponse(std::function<void(const SyncResponse&)> cb) { m_syncCallback = std::move(cb); }
    void onSyncError(std::function<void(const std::string&)> cb) { m_errorCallback = std::move(cb); }

private:
    SyncState m_state = SyncState::IDLE;
    SyncFilter m_filter;
    std::string m_homeserverUrl;
    std::string m_accessToken;
    std::string m_nextBatch;
    std::string m_filterId;

    std::function<void(const SyncResponse&)> m_syncCallback;
    std::function<void(const std::string&)> m_errorCallback;

    void syncLoop();
    bool fetchSync(int timeoutMs);
    std::string buildFilterJson();
};

} // namespace matrix_sdk
