#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <mutex>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────
//  SQLite-backed persistent store for sessions, rooms, events
//  Equivalent to Kotlin DefaultSessionStore / ReStore
// ─────────────────────────────────────────────────────────────

// Forward declarations (opaque SQLite handle)
struct sqlite3;
struct sqlite3_stmt;

// ── Session data ──

struct StoredSession {
    std::string userId;
    std::string accessToken;
    std::string deviceId;
    std::string homeServer;
    std::string nextBatch;        // sync token
    std::string refreshToken;
    std::string olmAccountJson;   // serialized OlmAccount
    int64_t lastSyncMs = 0;
    bool isActive = true;
    int64_t createdAt = 0;
};

struct StoredDeviceKeys {
    std::string userId;
    std::string deviceId;
    json deviceKeys;
    std::vector<std::string> algorithms;
    json oneTimeKeyCounts;
    json signatures;
    int64_t uploadedAt = 0;
};

struct StoredCrossSigningKeys {
    std::string userId;
    json masterKey;
    json selfSigningKey;
    json userSigningKey;
    std::string masterKeySignature;
    std::string selfSigningKeySignature;
    std::string userSigningKeySignature;
    bool isVerified = false;
    int64_t uploadedAt = 0;
};

struct StoredKeyBackup {
    std::string version;
    std::string algorithm;
    std::string authDataJson;
    std::string recoveryKey;
    int64_t keyCount = 0;
    int64_t expectedKeyCount = 0;
    bool isVerified = false;
    int64_t createdAt = 0;
    int64_t updatedAt = 0;
};

// ── Room data ──

struct StoredRoom {
    std::string roomId;
    std::string name;
    std::string topic;
    std::string canonicalAlias;
    std::string avatarUrl;
    std::string joinRules;
    std::string guestAccess;
    std::string historyVisibility;
    std::string encryptionAlgorithm;
    bool isEncrypted = false;
    bool isDM = false;
    bool isSpace = false;
    std::string spaceParent;
    int64_t notificationCount = 0;
    int64_t highlightCount = 0;
    std::string lastEventId;
    std::string lastMessagePreview;
    std::string lastMessageSender;
    int64_t lastMessageTs = 0;
    int32_t memberCount = 0;
    int32_t joinedMemberCount = 0;
    std::string membership;
    double sortOrder = 0.0;
    int64_t lastUpdatedAt = 0;
};

struct StoredRoomMember {
    std::string roomId;
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    std::string membership;
    std::string reason;
    int64_t powerLevel = 0;
    int64_t joinedAt = 0;
    std::string senderId;
};

struct StoredRoomStateEvent {
    std::string roomId;
    std::string eventId;
    std::string eventType;
    std::string stateKey;
    std::string sender;
    json content;
    int64_t originServerTs = 0;
    int64_t age = 0;
    std::string prevContentJson;
};

struct StoredRoomEvent {
    int64_t id = 0;
    std::string roomId;
    std::string eventId;
    std::string sender;
    std::string eventType;
    std::string stateKey;
    json content;
    json decryptedContent;
    std::string prevContentJson;
    int64_t originServerTs = 0;
    int64_t age = 0;
    std::string redacts;
    std::string redactedBecause;
    std::string transactionId;
    std::string relationType;
    std::string relationEventId;
    bool isEncrypted = false;
    bool isRedacted = false;
    bool isPending = false;
    bool isFailed = false;
    int64_t receivedAt = 0;
    int64_t sentAt = 0;
};

struct StoredUserProfile {
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    std::string presence;
    std::string statusMsg;
    int64_t lastActiveAt = 0;
    int64_t lastUpdatedAt = 0;
    int64_t createdAt = 0;
};

struct StoredAccountData {
    std::string eventType;
    json content;
    int64_t updatedAt = 0;
};

struct StoredRoomTag {
    std::string roomId;
    std::string tag;         // m.lowpriority, m.favourite, u.*
    double order = 0.0;
    json additionalInfo;
};

struct StoredRoomAccountData {
    std::string roomId;
    std::string eventType;
    json content;
    int64_t updatedAt = 0;
};

struct StoredPushRule {
    std::string ruleId;
    std::string scope;       // global, device
    std::string kind;        // override, content, room, sender, underride
    json actions;
    json conditions;
    std::string pattern;
    bool enabled = true;
    bool isDefault = false;
    int priorityClass = 0;
};

struct StoredOutgoingKeyRequest {
    std::string requestId;
    std::string roomId;
    std::string sessionId;
    std::string algorithm;
    std::string senderKey;
    std::string requestBody;
    std::string recipientsJson;
    int64_t requestedAt = 0;
    int64_t lastRetryAt = 0;
    int retryCount = 0;
    bool isCancelled = false;
};

struct StoredIncomingKeyRequest {
    std::string requestId;
    std::string roomId;
    std::string sessionId;
    std::string algorithm;
    std::string senderKey;
    std::string fromDevice;
    std::string fromUser;
    std::string requestBody;
    int64_t receivedAt = 0;
    bool isForwarded = false;
    bool isCancelled = false;
};

// ── Read markers & receipts ──

struct StoredReadReceipt {
    std::string roomId;
    std::string userId;
    std::string eventId;
    int64_t ts = 0;
};

struct StoredReadMarker {
    std::string roomId;
    std::string eventId;            // fully_read
    std::string readReceiptEventId; // m.read
    int64_t updatedAt = 0;
};

struct StoredSendingEvent {
    int64_t id = 0;
    std::string roomId;
    std::string eventId;
    std::string eventType;
    std::string transactionId;
    json content;
    int status = 0;       // 0=pending, 1=sending, 2=sent, 3=failed
    int64_t queuedAt = 0;
    int64_t sentAt = 0;
    int retryCount = 0;
    std::string errorMsg;
};

struct StoreStats {
    int64_t totalRooms = 0;
    int64_t totalEvents = 0;
    int64_t totalMembers = 0;
    int64_t totalProfiles = 0;
    int64_t dbSizeBytes = 0;
    int64_t oldestEventTs = 0;
    int64_t newestEventTs = 0;
};

// ── Migration callback ──

struct StoreMigration {
    int fromVersion;
    int toVersion;
    std::string description;
    std::function<bool(sqlite3*)> up;
};

// ── Main Store class ──

class MatrixStore {
public:
    MatrixStore();
    explicit MatrixStore(const std::string& dbPath);
    ~MatrixStore();

    // Lifecycle
    bool open(const std::string& dbPath);
    void close();
    bool isOpen() const;
    const std::string& path() const { return m_dbPath; }

    // Schema
    bool createSchema();
    bool migrateSchema();
    int schemaVersion() const { return m_schemaVersion; }
    void addMigration(const StoreMigration& migration);
    bool runVacuum();
    bool runIntegrityCheck(std::string& result);

    // ── Session operations ──
    bool saveSession(const StoredSession& session);
    std::optional<StoredSession> loadSession(const std::string& userId);
    bool updateNextBatch(const std::string& userId, const std::string& nextBatch);
    bool updateLastSync(const std::string& userId, int64_t ts);
    bool updateAccessToken(const std::string& userId, const std::string& token);
    bool updateRefreshToken(const std::string& userId, const std::string& refreshToken);
    bool updateOlmAccount(const std::string& userId, const std::string& olmJson);
    bool deleteSession(const std::string& userId);
    std::vector<StoredSession> getAllSessions();

    // ── Room operations ──
    bool saveRoom(const StoredRoom& room);
    std::optional<StoredRoom> loadRoom(const std::string& roomId);
    bool updateRoom(const std::string& roomId, const StoredRoom& room);
    bool deleteRoom(const std::string& roomId);
    std::vector<StoredRoom> getAllRooms();
    std::vector<StoredRoom> getRoomsByMembership(const std::string& membership);
    std::vector<StoredRoom> getDMRooms();
    std::vector<StoredRoom> getSpaceRooms();
    std::vector<StoredRoom> getRoomsBySpace(const std::string& spaceRoomId);
    bool updateRoomName(const std::string& roomId, const std::string& name);
    bool updateRoomTopic(const std::string& roomId, const std::string& topic);
    bool updateRoomAvatar(const std::string& roomId, const std::string& url);
    bool updateRoomMembership(const std::string& roomId, const std::string& membership);
    bool updateRoomNotificationCounts(const std::string& roomId,
                                       int64_t notificationCount, int64_t highlightCount);
    bool updateRoomLastMessage(const std::string& roomId,
                                const std::string& eventId, const std::string& preview,
                                const std::string& sender, int64_t ts);
    int64_t getTotalRoomCount();
    int64_t getUnreadRoomCount();

    // ── Room members ──
    bool saveMember(const StoredRoomMember& member);
    bool saveMembers(const std::vector<StoredRoomMember>& members);
    std::optional<StoredRoomMember> loadMember(const std::string& roomId,
                                                const std::string& userId);
    std::vector<StoredRoomMember> getRoomMembers(const std::string& roomId);
    std::vector<StoredRoomMember> getRoomMembersByMembership(
        const std::string& roomId, const std::string& membership);
    bool updateMemberDisplayName(const std::string& roomId, const std::string& userId,
                                  const std::string& displayName);
    bool updateMemberAvatar(const std::string& roomId, const std::string& userId,
                             const std::string& avatarUrl);
    bool updateMemberPowerLevel(const std::string& roomId, const std::string& userId,
                                 int64_t level);
    bool removeMember(const std::string& roomId, const std::string& userId);
    bool removeAllMembers(const std::string& roomId);
    int64_t getRoomMemberCount(const std::string& roomId);
    std::vector<StoredRoomMember> findUsersInRooms(const std::string& userId);

    // ── Room events ──
    bool saveEvent(const StoredRoomEvent& event);
    bool saveEvents(const std::vector<StoredRoomEvent>& events);
    std::optional<StoredRoomEvent> loadEvent(const std::string& eventId);
    std::vector<StoredRoomEvent> getRoomEvents(const std::string& roomId,
                                                int limit = 50, int offset = 0);
    std::vector<StoredRoomEvent> getRoomEventsBefore(const std::string& roomId,
                                                      int64_t beforeTs, int limit = 50);
    std::vector<StoredRoomEvent> getRoomEventsAfter(const std::string& roomId,
                                                     int64_t afterTs, int limit = 50);
    std::vector<StoredRoomEvent> getRoomEventsByType(const std::string& roomId,
                                                      const std::string& eventType,
                                                      int limit = 50);
    std::vector<StoredRoomEvent> getRoomEventsBySender(const std::string& roomId,
                                                        const std::string& sender,
                                                        int limit = 50);
    std::vector<StoredRoomEvent> searchEvents(const std::string& query,
                                               const std::string& roomId = "",
                                               int limit = 50);
    bool redactEvent(const std::string& eventId, const std::string& reason = "");
    bool deleteEvent(const std::string& eventId);
    bool deleteRoomEvents(const std::string& roomId);
    int64_t getRoomEventCount(const std::string& roomId);
    std::optional<StoredRoomEvent> getLastEvent(const std::string& roomId);
    std::optional<StoredRoomEvent> getEventByTxnId(const std::string& roomId,
                                                     const std::string& txnId);
    std::vector<StoredRoomEvent> getRelatedEvents(const std::string& roomId,
                                                    const std::string& relationEventId,
                                                    const std::string& relationType = "",
                                                    int limit = 50);

    // ── Room state events ──
    bool saveStateEvent(const StoredRoomStateEvent& event);
    std::optional<StoredRoomStateEvent> loadStateEvent(const std::string& roomId,
                                                         const std::string& eventType,
                                                         const std::string& stateKey = "");
    std::vector<StoredRoomStateEvent> getRoomStateEvents(const std::string& roomId);
    std::vector<StoredRoomStateEvent> getRoomStateEventsByType(const std::string& roomId,
                                                                const std::string& eventType);
    bool deleteStateEvent(const std::string& roomId, const std::string& eventType,
                          const std::string& stateKey = "");

    // ── Device keys ──
    bool saveDeviceKeys(const StoredDeviceKeys& keys);
    std::optional<StoredDeviceKeys> loadDeviceKeys(const std::string& userId,
                                                     const std::string& deviceId);
    std::vector<StoredDeviceKeys> getAllDeviceKeys(const std::string& userId);
    bool updateDeviceKeys(const std::string& userId, const std::string& deviceId,
                           const json& keys);
    bool deleteDeviceKeys(const std::string& userId, const std::string& deviceId);
    bool deleteAllDeviceKeys(const std::string& userId);

    // ── Cross-signing keys ──
    bool saveCrossSigningKeys(const StoredCrossSigningKeys& keys);
    std::optional<StoredCrossSigningKeys> loadCrossSigningKeys(const std::string& userId);
    bool updateCrossSigningVerified(const std::string& userId, bool verified);
    bool deleteCrossSigningKeys(const std::string& userId);

    // ── Key backup ──
    bool saveKeyBackup(const StoredKeyBackup& backup);
    std::optional<StoredKeyBackup> loadKeyBackup();
    std::vector<StoredKeyBackup> getAllKeyBackups();
    bool updateKeyBackupVersion(const std::string& version);
    bool deleteKeyBackup(const std::string& version);

    // ── User profiles ──
    bool saveUserProfile(const StoredUserProfile& profile);
    std::optional<StoredUserProfile> loadUserProfile(const std::string& userId);
    bool updateUserDisplayName(const std::string& userId, const std::string& name);
    bool updateUserAvatar(const std::string& userId, const std::string& url);
    bool updateUserPresence(const std::string& userId, const std::string& presence,
                             const std::string& statusMsg = "");
    bool deleteUserProfile(const std::string& userId);
    std::vector<StoredUserProfile> searchUserProfiles(const std::string& query,
                                                        int limit = 20);

    // ── Account data ──
    bool saveAccountData(const StoredAccountData& data);
    std::optional<StoredAccountData> loadAccountData(const std::string& eventType);
    std::vector<StoredAccountData> getAllAccountData();
    bool deleteAccountData(const std::string& eventType);

    // ── Room tags ──
    bool saveRoomTag(const StoredRoomTag& tag);
    std::vector<StoredRoomTag> getRoomTags(const std::string& roomId);
    std::vector<StoredRoomTag> getRoomsByTag(const std::string& tag,
                                              bool ordered = true);
    bool deleteRoomTag(const std::string& roomId, const std::string& tag);
    bool deleteAllRoomTags(const std::string& roomId);

    // ── Room account data ──
    bool saveRoomAccountData(const StoredRoomAccountData& data);
    std::optional<StoredRoomAccountData> loadRoomAccountData(
        const std::string& roomId, const std::string& eventType);
    std::vector<StoredRoomAccountData> getAllRoomAccountData(const std::string& roomId);
    bool deleteRoomAccountData(const std::string& roomId, const std::string& eventType);

    // ── Push rules ──
    bool savePushRule(const StoredPushRule& rule);
    std::vector<StoredPushRule> getAllPushRules();
    std::vector<StoredPushRule> getPushRulesByScope(const std::string& scope);
    std::optional<StoredPushRule> loadPushRule(const std::string& scope,
                                                const std::string& kind,
                                                const std::string& ruleId);
    bool updatePushRuleEnabled(const std::string& scope, const std::string& kind,
                                const std::string& ruleId, bool enabled);
    bool updatePushRuleActions(const std::string& scope, const std::string& kind,
                                const std::string& ruleId, const json& actions);
    bool deletePushRule(const std::string& scope, const std::string& kind,
                         const std::string& ruleId);
    bool deleteAllPushRules();

    // ── Read receipts & markers ──
    bool saveReadReceipt(const StoredReadReceipt& receipt);
    std::vector<StoredReadReceipt> getReadReceipts(const std::string& roomId);
    std::optional<StoredReadReceipt> getReadReceipt(const std::string& roomId,
                                                      const std::string& userId);
    bool saveReadMarker(const StoredReadMarker& marker);
    std::optional<StoredReadMarker> loadReadMarker(const std::string& roomId);
    std::vector<StoredReadMarker> getAllReadMarkers();

    // ── Sending events queue ──
    bool queueSendingEvent(const StoredSendingEvent& event);
    std::vector<StoredSendingEvent> getPendingEvents(const std::string& roomId = "");
    std::optional<StoredSendingEvent> getSendingEvent(int64_t id);
    bool updateSendingEventStatus(int64_t id, int status,
                                   const std::string& errorMsg = "");
    bool markSendingEventSent(int64_t id, const std::string& eventId);
    bool deleteSendingEvent(int64_t id);
    bool deleteSentEvents();
    int64_t getPendingEventCount();

    // ── Outgoing key requests ──
    bool saveOutgoingKeyRequest(const StoredOutgoingKeyRequest& req);
    std::vector<StoredOutgoingKeyRequest> getPendingOutgoingKeyRequests();
    std::optional<StoredOutgoingKeyRequest> getOutgoingKeyRequest(
        const std::string& requestId);
    bool cancelOutgoingKeyRequest(const std::string& requestId);
    bool updateOutgoingKeyRequestRetry(const std::string& requestId);
    bool deleteOutgoingKeyRequest(const std::string& requestId);

    // ── Incoming key requests ──
    bool saveIncomingKeyRequest(const StoredIncomingKeyRequest& req);
    std::vector<StoredIncomingKeyRequest> getPendingIncomingKeyRequests();
    std::optional<StoredIncomingKeyRequest> getIncomingKeyRequest(
        const std::string& requestId);
    bool markIncomingKeyRequestForwarded(const std::string& requestId);
    bool cancelIncomingKeyRequest(const std::string& requestId);
    bool deleteIncomingKeyRequest(const std::string& requestId);

    // ── Transactions ──
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool isInTransaction() const;

    // ── Statistics ──
    StoreStats getStats();

    // ── Maintenance ──
    bool pruneOldEvents(int64_t olderThanMs, int maxEventsPerRoom = 1000);
    bool clearAllData();
    bool vacuum();

private:
    // SQLite helpers
    bool executeSQL(const std::string& sql);
    bool prepareStatement(const std::string& sql, sqlite3_stmt** stmt);
    bool bindString(sqlite3_stmt* stmt, int index, const std::string& value);
    bool bindInt64(sqlite3_stmt* stmt, int index, int64_t value);
    bool bindNull(sqlite3_stmt* stmt, int index);
    bool bindJson(sqlite3_stmt* stmt, int index, const json& value);
    std::string getColumnString(sqlite3_stmt* stmt, int index);
    int64_t getColumnInt64(sqlite3_stmt* stmt, int index);
    bool getColumnBool(sqlite3_stmt* stmt, int index);
    json getColumnJson(sqlite3_stmt* stmt, int index);
    double getColumnDouble(sqlite3_stmt* stmt, int index);
    bool stepAndFinalize(sqlite3_stmt* stmt);
    int64_t lastInsertRowId();

    // Migration helpers
    int readSchemaVersion();
    bool setSchemaVersion(int version);
    bool runMigrations();
    bool createBaseSchema();

    // Thread safety
    mutable std::mutex m_mutex;
    sqlite3* m_db = nullptr;
    std::string m_dbPath;
    int m_schemaVersion = 0;
    bool m_inTransaction = false;
};

} // namespace matrix_sdk
