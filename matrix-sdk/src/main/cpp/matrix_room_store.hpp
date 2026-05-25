#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <mutex>
#include <functional>
#include <nlohmann/json.hpp>
#include "matrix_common_types.hpp"

namespace matrix_sdk {

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────
//  In-memory room state cache with LRU eviction and delta updates
//  Complement to MatrixStore; optimized for hot-path reads
// ─────────────────────────────────────────────────────────────

// ── Room summary (lightweight) ──

struct RoomSummary {
    std::string roomId;
    std::string name;
    std::string topic;
    std::string canonicalAlias;
    std::string avatarUrl;
    std::string membership;          // invite, join, leave, ban
    std::string joinRule;
    std::string guestAccess;
    std::string historyVisibility;
    std::string roomVersion;
    bool isEncrypted = false;
    bool isDirect = false;
    bool isSpace = false;
    std::string spaceParentId;
    int32_t memberCount = 0;
    int32_t joinedMemberCount = 0;
    int64_t lastEventTimestamp = 0;
    std::string lastEventId;
    std::string lastMessagePreview;
    std::string lastMessageSender;
    int64_t notificationCount = 0;
    int64_t highlightCount = 0;
    int64_t localUnreadCount = 0;
    std::optional<std::string> tombstone;
    std::optional<std::string> successorRoomId;
    int64_t createdAt = 0;
    int64_t updatedAt = 0;
    int64_t lastReadEventTs = 0;
};

// ── Room power levels ──

struct RoomPowerLevels {
    int32_t usersDefault = 0;
    int32_t eventsDefault = 0;
    int32_t stateDefault = 50;
    int32_t ban = 50;
    int32_t kick = 50;
    int32_t redact = 50;
    int32_t invite = 50;

    std::map<std::string, int32_t> userOverrides;
    std::map<std::string, int32_t> eventOverrides;

    int32_t getUserLevel(const std::string& userId) const;
    int32_t getEventLevel(const std::string& eventType) const;
    bool canSendEvent(const std::string& userId, const std::string& eventType) const;
    bool canSendState(const std::string& userId, const std::string& eventType) const;

    json toJson() const;
    static RoomPowerLevels fromJson(const json& j);
    static RoomPowerLevels defaults();
};

// ── Room tombstone ──

struct RoomTombstone {
    std::string body;
    std::string replacementRoomId;
    bool isActive = true;
};

// ── Room create event info ──

struct RoomCreateInfo {
    std::string creator;
    std::string roomVersion;
    bool isFederated = true;
    std::string predecessorRoomId;
    std::string predecessorEventId;
};

// ── Room encryption info ──

struct RoomEncryptionInfo {
    std::string algorithm = "m.megolm.v1.aes-sha2";
    int32_t rotationPeriodMs = 604800000;  // 7 days
    int32_t rotationPeriodMessages = 100;
    bool algorithmVerified = false;
};

// ── Room member ──

struct CachedRoomMember {
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    std::string membership;     // join, invite, leave, ban
    std::string reason;
    int32_t powerLevel = 0;
    bool isOnline = false;
    std::string presence;       // online, offline, unavailable
    std::string statusMessage;
    int64_t lastActiveAgo = 0;
    int64_t joinedAt = 0;
    int64_t updatedAt = 0;
};

// ── Room typing ──

struct RoomTypingUsers {
    std::vector<std::string> userIds;
    int64_t expiresAt = 0;
    bool isTyping(const std::string& userId) const;
};

// ── Room read receipts ──

struct RoomReceipts {
    std::map<std::string, std::string> receipts; // userId -> eventId
    std::map<std::string, int64_t> timestamps;   // userId -> ts
    std::string ownReceipt;
    int64_t ownReceiptTs = 0;

    std::optional<std::string> getReceipt(const std::string& userId) const;
    void updateReceipt(const std::string& userId, const std::string& eventId, int64_t ts);
};

// ── Full room state ──

struct RoomState {
    RoomSummary summary;
    RoomCreateInfo createInfo;
    RoomEncryptionInfo encryptionInfo;
    RoomPowerLevels powerLevels;
    RoomTombstone tombstone;
    RoomTypingUsers typing;
    RoomReceipts receipts;

    // Live state events
    json joinRules;
    json guestAccess;
    json historyVisibility;
    json roomName;
    json roomTopic;
    json roomAvatar;
    json roomCanonicalAlias;
    json roomPinnedEvents;
    json roomServerAcl;
    json relatedGroups;

    // Raw state map: (eventType, stateKey) -> content
    std::map<std::string, json> stateMap;

    // Room members (lazy-loaded)
    std::map<std::string, CachedRoomMember> members;
    bool membersFullyLoaded = false;
    int32_t totalMemberEstimate = 0;

    // Account data for this room
    json accountData;

    // Event cache (most recent N timeline events)
    std::vector<json> recentEvents;
    int maxRecentEvents = 50;

    // Room tags
    std::map<std::string, double> tags;  // tagName -> order

    // Pending local echoes
    std::vector<json> pendingEvents;

    // Metadata
    bool stale = false;
    int64_t lastStateUpdate = 0;
    int64_t lastTimelineUpdate = 0;
    int64_t stateVersion = 0;
    std::string prevBatch;
};

// ── LRU cache entry ──

struct RoomCacheEntry {
    RoomState state;
    int64_t lastAccess = 0;
    int accessCount = 0;
};

// ── Room store configuration ──

struct RoomStoreConfig {
    size_t maxCachedRooms = 500;
    size_t maxRecentEventsPerRoom = 100;
    size_t maxCachedMembersPerRoom = 5000;
    int memberLoadBatchSize = 100;
    int64_t cacheTtlMs = 30 * 60 * 1000;       // 30 minutes
    int64_t memberCacheTtlMs = 60 * 60 * 1000;  // 1 hour
    bool autoEvictStale = true;
    int evictionCheckIntervalMs = 60000;
    bool persistToDatabase = true;
    bool lazyLoadMembers = true;
};

// ── Callbacks ──

using RoomUpdateCallback = std::function<void(const std::string& roomId,
                                                const std::string& fieldChanged)>;
using MemberUpdateCallback = std::function<void(const std::string& roomId,
                                                  const std::string& userId)>;
using RoomEvictionCallback = std::function<void(const std::string& roomId,
                                                  const RoomState& finalState)>;

// ── Room store ──

class MatrixRoomStore {
public:
    MatrixRoomStore();
    explicit MatrixRoomStore(const RoomStoreConfig& config);
    ~MatrixRoomStore();

    // ── Configuration ──
    void configure(const RoomStoreConfig& config);
    const RoomStoreConfig& config() const { return m_config; }

    // ── Room lifecycle ──
    bool createRoom(const std::string& roomId, const RoomSummary& summary);
    bool loadRoom(const std::string& roomId);
    bool evictRoom(const std::string& roomId);
    bool hasRoom(const std::string& roomId) const;
    size_t roomCount() const;
    std::vector<std::string> getAllRoomIds() const;

    // ── Room state access ──
    std::optional<RoomState> getRoomState(const std::string& roomId);
    std::optional<RoomSummary> getRoomSummary(const std::string& roomId);
    bool updateRoomSummary(const std::string& roomId, const RoomSummary& summary);

    // ── State events ──
    bool applyStateEvent(const std::string& roomId, const std::string& eventType,
                          const std::string& stateKey, const json& content,
                          const std::string& sender = "");
    bool applyStateEvents(const std::string& roomId,
                           const std::vector<json>& stateEvents);
    std::optional<json> getStateEvent(const std::string& roomId,
                                       const std::string& eventType,
                                       const std::string& stateKey = "");
    std::vector<json> getStateEventsByType(const std::string& roomId,
                                             const std::string& eventType);

    // ── Timeline ──
    bool addTimelineEvent(const std::string& roomId, const json& event);
    bool addTimelineEvents(const std::string& roomId,
                            const std::vector<json>& events);
    std::vector<json> getRecentEvents(const std::string& roomId,
                                       int limit = 50);
    bool setPrevBatch(const std::string& roomId, const std::string& prevBatch);
    std::optional<std::string> getPrevBatch(const std::string& roomId);

    // ── Members ──
    bool addMember(const std::string& roomId, const CachedRoomMember& member);
    bool addMembers(const std::string& roomId,
                     const std::vector<CachedRoomMember>& members);
    bool removeMember(const std::string& roomId, const std::string& userId);
    bool updateMember(const std::string& roomId, const std::string& userId,
                       const CachedRoomMember& member);
    std::optional<CachedRoomMember> getMember(const std::string& roomId,
                                                const std::string& userId);
    std::vector<CachedRoomMember> getMembers(const std::string& roomId,
                                               int limit = 100, int offset = 0);
    std::vector<CachedRoomMember> getMembersByMembership(
        const std::string& roomId, const std::string& membership);
    int32_t getMemberCount(const std::string& roomId) const;
    bool areMembersFullyLoaded(const std::string& roomId) const;
    void setMembersFullyLoaded(const std::string& roomId, bool loaded);

    // ── Typing ──
    bool setTypingUsers(const std::string& roomId,
                         const std::vector<std::string>& userIds,
                         int64_t expiresAt = 0);
    std::vector<std::string> getTypingUsers(const std::string& roomId);

    // ── Receipts ──
    bool updateReceipt(const std::string& roomId, const std::string& userId,
                        const std::string& eventId, int64_t ts = 0);
    std::optional<std::string> getReadReceipt(const std::string& roomId,
                                                const std::string& userId);

    // ── Read markers ──
    bool setReadMarker(const std::string& roomId, const std::string& eventId);
    std::optional<std::string> getReadMarker(const std::string& roomId);

    // ── Account data ──
    bool setRoomAccountData(const std::string& roomId,
                             const std::string& eventType,
                             const json& content);
    std::optional<json> getRoomAccountData(const std::string& roomId,
                                             const std::string& eventType);

    // ── Tags ──
    bool setRoomTag(const std::string& roomId, const std::string& tag,
                     double order = 0.0);
    bool removeRoomTag(const std::string& roomId, const std::string& tag);
    std::map<std::string, double> getRoomTags(const std::string& roomId);

    // ── Local echoes ──
    bool addPendingEvent(const std::string& roomId, const json& event);
    bool removePendingEvent(const std::string& roomId, const std::string& txnId);
    std::vector<json> getPendingEvents(const std::string& roomId);

    // ── Queries ──
    std::vector<std::string> getRoomsByMembership(const std::string& membership);
    std::vector<std::string> getDirectRooms();
    std::vector<std::string> getSpaceRooms();
    std::vector<std::string> getRoomsBySpace(const std::string& spaceRoomId);
    std::vector<std::string> getRoomsByTag(const std::string& tag);
    std::vector<std::string> searchRooms(const std::string& query, int limit = 20);
    std::vector<std::string> getUnreadRooms();
    std::vector<std::string> getRoomsWithNotifications();

    // ── Cache management ──
    void evictStale();
    void evictAll();
    void evictLRU(size_t targetSize);
    size_t getCacheSize() const;
    int64_t getCacheMemoryEstimate() const;

    // ── Callbacks ──
    void setOnRoomUpdate(RoomUpdateCallback cb) { m_roomUpdateCallback = std::move(cb); }
    void setOnMemberUpdate(MemberUpdateCallback cb) { m_memberUpdateCallback = std::move(cb); }
    void setOnRoomEviction(RoomEvictionCallback cb) { m_evictionCallback = std::move(cb); }

    // ── Persistence callback ──
    using PersistFunc = std::function<bool(const std::string& roomId,
                                            const RoomState& state)>;
    void setPersistCallback(PersistFunc cb) { m_persistCallback = std::move(cb); }

    // ── Diagnostics ──
    json getDiagnostics() const;
    void dumpRoomState(const std::string& roomId, json& out) const;

private:
    // ── Internal helpers ──
    RoomState* getOrCreateRoom(const std::string& roomId);
    RoomState* getRoom(const std::string& roomId);
    const RoomState* getRoom(const std::string& roomId) const;
    void touchRoom(const std::string& roomId);
    void updateStateVersion(const std::string& roomId);
    bool persistRoomIfNeeded(const std::string& roomId);
    void checkAutoEviction();
    std::string findLRURoom() const;

    RoomStoreConfig m_config;
    mutable std::mutex m_mutex;

    std::map<std::string, RoomCacheEntry> m_rooms;

    // Callbacks
    RoomUpdateCallback m_roomUpdateCallback;
    MemberUpdateCallback m_memberUpdateCallback;
    RoomEvictionCallback m_evictionCallback;
    PersistFunc m_persistCallback;

    // Eviction timer
    int64_t m_lastEvictionCheck = 0;
    int64_t m_evictionCheckRunning = 0;
};

} // namespace matrix_sdk
