#include <string>
#include <vector>
#include <map>
#include <set>
#include <nlohmann/json.hpp>
#include <android/log.h>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <mutex>
#include <thread>

#define LOG_TAG "MatrixStore"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {
using json = nlohmann::json;

// ── In-Memory Store ──

struct StoredSession {
    std::string userId; std::string accessToken; std::string deviceId;
    std::string homeserverUrl; std::string nextBatch; int64_t lastSyncMs = 0;
    bool isTokenLogin = false;
};

struct StoredRoom {
    std::string roomId; std::string name; std::string topic; std::string avatarUrl;
    std::string canonicalAlias; int joinedMembers = 0; int invitedMembers = 0;
    bool isDirect = false; bool isEncrypted = false; bool isSpace = false;
    int notificationCount = 0; int highlightCount = 0;
    int64_t lastEventTs = 0; std::string membership; bool isFavorite = false;
    std::vector<std::string> tags;
};

struct StoredEvent {
    std::string eventId; std::string roomId; std::string sender; std::string type;
    std::string contentJson; std::string stateKey; std::string redacts;
    int64_t originServerTs = 0; int64_t receivedTs = 0;
    bool isEncrypted = false; bool isRedacted = false; bool isOutgoing = false;
};

struct StoredUser {
    std::string userId; std::string displayName; std::string avatarUrl;
    std::string presence; std::string statusMsg; int64_t lastActiveTs = 0;
};

struct StoredDevice {
    std::string userId; std::string deviceId; std::string displayName;
    std::string lastSeenIp; int64_t lastSeenTs = 0;
    int verificationStatus = -1; // -1=unknown, 0=unverified, 1=verified, 2=blocked
};

struct StoredOlmSession {
    std::string sessionId; std::string senderKey; std::string sessionData;
    int64_t createdAtMs = 0; int64_t lastUsedMs = 0;
};

struct StoredMegolmSession {
    std::string sessionId; std::string roomId; std::string senderKey;
    std::string sessionData; int firstKnownIndex = 0;
    int64_t createdAtMs = 0; bool isInbound = true;
};

class MatrixStore {
public:
    MatrixStore() = default;

    // ── Session ──
    void saveSession(const StoredSession& session) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_session = session;
        LOGI("Session saved for %s", session.userId.c_str());
    }

    StoredSession* getSession() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_session.userId.empty() ? nullptr : &m_session;
    }

    void clearSession() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_session = StoredSession{};
        LOGI("Session cleared");
    }

    // ── Rooms ──
    void saveRoom(const StoredRoom& room) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_rooms[room.roomId] = room;
    }

    StoredRoom* getRoom(const std::string& roomId) {
        auto it = m_rooms.find(roomId);
        return it != m_rooms.end() ? &it->second : nullptr;
    }

    std::vector<StoredRoom> getAllRooms() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<StoredRoom> rooms;
        for (auto& [id, room] : m_rooms) rooms.push_back(room);
        return rooms;
    }

    std::vector<StoredRoom> getRoomsByMembership(const std::string& membership) {
        std::vector<StoredRoom> result;
        for (auto& [id, room] : m_rooms) {
            if (room.membership == membership) result.push_back(room);
        }
        return result;
    }

    std::vector<StoredRoom> getFavoriteRooms() {
        std::vector<StoredRoom> result;
        for (auto& [id, room] : m_rooms) {
            if (room.isFavorite) result.push_back(room);
        }
        return result;
    }

    std::vector<StoredRoom> getDirectChats() {
        std::vector<StoredRoom> result;
        for (auto& [id, room] : m_rooms) {
            if (room.isDirect) result.push_back(room);
        }
        return result;
    }

    std::vector<StoredRoom> getSpaces() {
        std::vector<StoredRoom> result;
        for (auto& [id, room] : m_rooms) {
            if (room.isSpace) result.push_back(room);
        }
        return result;
    }

    void removeRoom(const std::string& roomId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_rooms.erase(roomId);
        // Also remove events for this room
        m_events.erase(
            std::remove_if(m_events.begin(), m_events.end(),
                [&](auto& e) { return e.roomId == roomId; }),
            m_events.end());
    }

    void setRoomFavorite(const std::string& roomId, bool fav) {
        auto* room = getRoom(roomId);
        if (room) room->isFavorite = fav;
    }

    int roomCount() const { return static_cast<int>(m_rooms.size()); }

    int totalUnreadCount() const {
        int total = 0;
        for (auto& [id, room] : m_rooms) total += room.notificationCount;
        return total;
    }

    int totalHighlightCount() const {
        int total = 0;
        for (auto& [id, room] : m_rooms) total += room.highlightCount;
        return total;
    }

    // ── Events ──
    void saveEvent(const StoredEvent& event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        // Check duplicate
        for (auto& e : m_events) {
            if (e.eventId == event.eventId) return;
        }
        m_events.push_back(event);
        // Limit per room
        int roomCount = 0;
        for (auto& e : m_events) if (e.roomId == event.roomId) roomCount++;
        if (roomCount > 1000) {
            // Remove oldest events for this room
            auto it = m_events.begin();
            while (it != m_events.end() && roomCount > 500) {
                if (it->roomId == event.roomId) {
                    it = m_events.erase(it);
                    roomCount--;
                } else { ++it; }
            }
        }
    }

    std::vector<StoredEvent> getRoomEvents(const std::string& roomId, int limit = 50) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<StoredEvent> result;
        for (auto it = m_events.rbegin(); it != m_events.rend() && static_cast<int>(result.size()) < limit; ++it) {
            if (it->roomId == roomId) result.push_back(*it);
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    StoredEvent* getEvent(const std::string& eventId) {
        for (auto& e : m_events) {
            if (e.eventId == eventId) return &e;
        }
        return nullptr;
    }

    void redactEvent(const std::string& eventId) {
        auto* e = getEvent(eventId);
        if (e) e->isRedacted = true;
    }

    // ── Users ──
    void saveUser(const StoredUser& user) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_users[user.userId] = user;
    }

    StoredUser* getUser(const std::string& userId) {
        auto it = m_users.find(userId);
        return it != m_users.end() ? &it->second : nullptr;
    }

    std::string getUserDisplayName(const std::string& userId) const {
        auto it = m_users.find(userId);
        if (it != m_users.end() && !it->second.displayName.empty()) return it->second.displayName;
        return userId;
    }

    // ── Devices ──
    void saveDevice(const StoredDevice& device) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_devices[device.userId + "/" + device.deviceId] = device;
    }

    StoredDevice* getDevice(const std::string& userId, const std::string& deviceId) {
        auto it = m_devices.find(userId + "/" + deviceId);
        return it != m_devices.end() ? &it->second : nullptr;
    }

    std::vector<StoredDevice> getUserDevices(const std::string& userId) {
        std::vector<StoredDevice> result;
        std::string prefix = userId + "/";
        for (auto& [key, device] : m_devices) {
            if (key.find(prefix) == 0) result.push_back(device);
        }
        return result;
    }

    // ── Crypto Sessions ──
    void saveOlmSession(const StoredOlmSession& session) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_olmSessions[session.senderKey + "/" + session.sessionId] = session;
    }

    StoredOlmSession* getOlmSession(const std::string& senderKey, const std::string& sessionId) {
        auto it = m_olmSessions.find(senderKey + "/" + sessionId);
        return it != m_olmSessions.end() ? &it->second : nullptr;
    }

    void saveMegolmSession(const StoredMegolmSession& session) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_megolmSessions[session.sessionId] = session;
    }

    StoredMegolmSession* getMegolmSession(const std::string& sessionId) {
        auto it = m_megolmSessions.find(sessionId);
        return it != m_megolmSessions.end() ? &it->second : nullptr;
    }

    std::vector<StoredMegolmSession> getRoomMegolmSessions(const std::string& roomId) {
        std::vector<StoredMegolmSession> result;
        for (auto& [id, session] : m_megolmSessions) {
            if (session.roomId == roomId) result.push_back(session);
        }
        return result;
    }

    // ── Statistics ──
    struct StoreStats {
        int roomCount = 0; int eventCount = 0; int userCount = 0;
        int deviceCount = 0; int olmSessionCount = 0; int megolmSessionCount = 0;
        size_t estimatedMemoryBytes = 0;
    };

    StoreStats getStats() const {
        StoreStats s;
        s.roomCount = static_cast<int>(m_rooms.size());
        s.eventCount = static_cast<int>(m_events.size());
        s.userCount = static_cast<int>(m_users.size());
        s.deviceCount = static_cast<int>(m_devices.size());
        s.olmSessionCount = static_cast<int>(m_olmSessions.size());
        s.megolmSessionCount = static_cast<int>(m_megolmSessions.size());
        s.estimatedMemoryBytes = sizeof(*this) +
            m_rooms.size() * sizeof(StoredRoom) +
            m_events.size() * sizeof(StoredEvent) +
            m_users.size() * sizeof(StoredUser);
        return s;
    }

    // ── Bulk Operations ──
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_rooms.clear(); m_events.clear(); m_users.clear();
        m_devices.clear(); m_olmSessions.clear(); m_megolmSessions.clear();
    }

    void clearRoomData(const std::string& roomId) {
        m_rooms.erase(roomId);
        m_events.erase(std::remove_if(m_events.begin(), m_events.end(),
            [&](auto& e) { return e.roomId == roomId; }), m_events.end());
    }

private:
    std::mutex m_mutex;
    StoredSession m_session;
    std::map<std::string, StoredRoom> m_rooms;
    std::vector<StoredEvent> m_events;
    std::map<std::string, StoredUser> m_users;
    std::map<std::string, StoredDevice> m_devices;
    std::map<std::string, StoredOlmSession> m_olmSessions;
    std::map<std::string, StoredMegolmSession> m_megolmSessions;
};

} // namespace matrix_sdk
