#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <optional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann_json::json;

// ─────────────────────────────────────────────────────────────
//  Store schemas — extended DDL for Matrix sqlite store
// ─────────────────────────────────────────────────────────────

// Session metadata schema
static constexpr const char* SQL_SESSION_EXT_CREATE = R"SQL(
CREATE TABLE IF NOT EXISTS matrix_sessions_ext (
    user_id             TEXT PRIMARY KEY NOT NULL,
    access_token_enc    TEXT NOT NULL,
    refresh_token_enc   TEXT,
    device_id           TEXT NOT NULL,
    home_server         TEXT NOT NULL,
    identity_server     TEXT,
    next_batch          TEXT,
    olm_pickle          TEXT,
    olm_account_json    TEXT,
    megolm_inbound_sessions TEXT,
    cross_signing_keys  TEXT,
    last_active_ts      INTEGER DEFAULT 0,
    last_sync_ts        INTEGER DEFAULT 0,
    sync_filter_id      TEXT,
    sync_timeout_ms     INTEGER DEFAULT 30000,
    presence_status     TEXT DEFAULT 'online',
    avatar_url          TEXT,
    display_name        TEXT,
    notifications_enabled INTEGER DEFAULT 1,
    created_at          INTEGER NOT NULL,
    updated_at          INTEGER NOT NULL
);
)SQL";

// Room extended schema
static constexpr const char* SQL_ROOMS_EXT_CREATE = R"SQL(
CREATE TABLE IF NOT EXISTS matrix_rooms_ext (
    room_id                     TEXT PRIMARY KEY NOT NULL,
    name                        TEXT,
    canonical_alias             TEXT,
    topic                       TEXT,
    avatar_url                  TEXT,
    avatar_thumbnail_url        TEXT,
    join_rules                  TEXT DEFAULT 'invite',
    guest_access                TEXT DEFAULT 'forbidden',
    history_visibility          TEXT DEFAULT 'shared',
    encryption_algorithm        TEXT,
    encryption_rotation_period_ms INTEGER,
    encryption_rotation_period_msgs INTEGER,
    is_encrypted                INTEGER DEFAULT 0,
    is_dm                       INTEGER DEFAULT 0,
    is_space                    INTEGER DEFAULT 0,
    is_favorite                 INTEGER DEFAULT 0,
    is_low_priority             INTEGER DEFAULT 0,
    is_direct                   INTEGER DEFAULT 0,
    space_parent_id             TEXT,
    space_order                 TEXT,
    tombstone_replacement_room  TEXT,
    tombstone_body              TEXT,
    version                     TEXT DEFAULT '1',
    creator_user_id             TEXT,
    federated                   INTEGER DEFAULT 1,
    predecessor_room_id         TEXT,
    room_type                   TEXT,
    membership                  TEXT DEFAULT 'join',
    notification_count          INTEGER DEFAULT 0,
    highlight_count             INTEGER DEFAULT 0,
    total_event_count           INTEGER DEFAULT 0,
    last_message_preview        TEXT,
    last_message_sender         TEXT,
    last_message_ts             INTEGER DEFAULT 0,
    last_message_event_id       TEXT,
    last_read_event_id          TEXT,
    last_read_ts                INTEGER DEFAULT 0,
    fully_read_event_id         TEXT,
    unread_count                INTEGER DEFAULT 0,
    sort_order                  REAL DEFAULT 0.0,
    local_sort_order            INTEGER DEFAULT 0,
    draft_message               TEXT,
    draft_message_type          TEXT,
    pinned_event_ids            TEXT,
    join_ts                     INTEGER DEFAULT 0,
    leave_ts                    INTEGER DEFAULT 0,
    created_ts                  INTEGER DEFAULT 0,
    updated_ts                  INTEGER NOT NULL,
    sync_cursor                 TEXT
);
CREATE INDEX IF NOT EXISTS idx_rooms_ext_membership ON matrix_rooms_ext(membership);
CREATE INDEX IF NOT EXISTS idx_rooms_ext_dm ON matrix_rooms_ext(is_dm);
CREATE INDEX IF NOT EXISTS idx_rooms_ext_space ON matrix_rooms_ext(is_space);
CREATE INDEX IF NOT EXISTS idx_rooms_ext_fav ON matrix_rooms_ext(is_favorite);
CREATE INDEX IF NOT EXISTS idx_rooms_ext_space_parent ON matrix_rooms_ext(space_parent_id);
CREATE INDEX IF NOT EXISTS idx_rooms_ext_sort ON matrix_rooms_ext(sort_order DESC);
CREATE INDEX IF NOT EXISTS idx_rooms_ext_local_sort ON matrix_rooms_ext(local_sort_order);
CREATE INDEX IF NOT EXISTS idx_rooms_ext_last_msg ON matrix_rooms_ext(last_message_ts);
CREATE INDEX IF NOT EXISTS idx_rooms_ext_name ON matrix_rooms_ext(name COLLATE NOCASE);
)SQL";

// Events schema
static constexpr const char* SQL_EVENTS_EXT_CREATE = R"SQL(
CREATE TABLE IF NOT EXISTS matrix_events_ext (
    event_id            TEXT NOT NULL,
    room_id             TEXT NOT NULL,
    sender              TEXT NOT NULL,
    event_type          TEXT NOT NULL,
    state_key           TEXT,
    content             TEXT NOT NULL,
    origin_server_ts    INTEGER DEFAULT 0,
    unsigned_data       TEXT,
    prev_content        TEXT,
    redacts             TEXT,
    redacted_because    TEXT,
    txn_id              TEXT,
    relates_to_event_id TEXT,
    relates_to_type     TEXT,
    relates_to_key      TEXT,
    decrypted_content   TEXT,
    decryption_error    TEXT,
    is_decrypted        INTEGER DEFAULT 0,
    is_encrypted        INTEGER DEFAULT 0,
    is_redacted         INTEGER DEFAULT 0,
    is_outgoing         INTEGER DEFAULT 0,
    send_state          TEXT DEFAULT 'sent',
    received_ts         INTEGER NOT NULL,
    age_ms              INTEGER DEFAULT 0,
    sort_key            INTEGER NOT NULL,
    PRIMARY KEY (event_id, room_id)
);
CREATE INDEX IF NOT EXISTS idx_events_ext_room ON matrix_events_ext(room_id, sort_key);
CREATE INDEX IF NOT EXISTS idx_events_ext_sender ON matrix_events_ext(sender);
CREATE INDEX IF NOT EXISTS idx_events_ext_type ON matrix_events_ext(room_id, event_type);
CREATE INDEX IF NOT EXISTS idx_events_ext_state ON matrix_events_ext(room_id, event_type, state_key);
CREATE INDEX IF NOT EXISTS idx_events_ext_related ON matrix_events_ext(relates_to_event_id);
CREATE INDEX IF NOT EXISTS idx_events_ext_txn ON matrix_events_ext(txn_id);
CREATE INDEX IF NOT EXISTS idx_events_ext_send_state ON matrix_events_ext(send_state);
)SQL";

// Users schema
static constexpr const char* SQL_USERS_EXT_CREATE = R"SQL(
CREATE TABLE IF NOT EXISTS matrix_users_ext (
    user_id             TEXT PRIMARY KEY NOT NULL,
    display_name        TEXT,
    avatar_url          TEXT,
    avatar_thumbnail    TEXT,
    status_msg          TEXT,
    presence            TEXT DEFAULT 'offline',
    last_active_ago_ms  INTEGER,
    currently_active    INTEGER DEFAULT 0,
    is_ignored          INTEGER DEFAULT 0,
    is_bot              INTEGER DEFAULT 0,
    power_level_norm    INTEGER DEFAULT 0,
    trust_level         INTEGER DEFAULT 0,
    last_seen_room_id   TEXT,
    last_seen_ts        INTEGER DEFAULT 0,
    first_seen_ts       INTEGER DEFAULT 0,
    profile_updated_ts  INTEGER DEFAULT 0,
    device_count        INTEGER DEFAULT 0,
    note                TEXT,
    display_name_ts     INTEGER DEFAULT 0,
    avatar_url_ts       INTEGER DEFAULT 0
);
CREATE INDEX IF NOT EXISTS idx_users_ext_name ON matrix_users_ext(display_name COLLATE NOCASE);
CREATE INDEX IF NOT EXISTS idx_users_ext_presence ON matrix_users_ext(presence);
)SQL";

// Room members extended
static constexpr const char* SQL_MEMBERS_EXT_CREATE = R"SQL(
CREATE TABLE IF NOT EXISTS matrix_room_members_ext (
    room_id             TEXT NOT NULL,
    user_id             TEXT NOT NULL,
    display_name        TEXT,
    avatar_url          TEXT,
    membership          TEXT DEFAULT 'leave',
    reason              TEXT,
    power_level         INTEGER DEFAULT 0,
    is_room_creator     INTEGER DEFAULT 0,
    sender_id           TEXT,
    origin_server_ts    INTEGER DEFAULT 0,
    joined_at           INTEGER DEFAULT 0,
    invited_at          INTEGER DEFAULT 0,
    left_at             INTEGER DEFAULT 0,
    PRIMARY KEY (room_id, user_id)
);
CREATE INDEX IF NOT EXISTS idx_members_ext_room ON matrix_room_members_ext(room_id);
CREATE INDEX IF NOT EXISTS idx_members_ext_user ON matrix_room_members_ext(user_id);
CREATE INDEX IF NOT EXISTS idx_members_ext_membership ON matrix_room_members_ext(room_id, membership);
)SQL";

// Device keys
static constexpr const char* SQL_DEVICE_KEYS_EXT_CREATE = R"SQL(
CREATE TABLE IF NOT EXISTS matrix_device_keys_ext (
    user_id             TEXT NOT NULL,
    device_id           TEXT NOT NULL,
    display_name        TEXT,
    last_seen_ts        INTEGER DEFAULT 0,
    last_seen_ip        TEXT,
    algorithms          TEXT,
    keys_json           TEXT NOT NULL,
    signatures_json     TEXT,
    is_verified         INTEGER DEFAULT 0,
    is_blocked          INTEGER DEFAULT 0,
    verification_status TEXT DEFAULT 'unverified',
    PRIMARY KEY (user_id, device_id)
);
CREATE INDEX IF NOT EXISTS idx_device_keys_ext_verified ON matrix_device_keys_ext(is_verified);
)SQL";

// Outgoing key requests (megolm)
static constexpr const char* SQL_KEY_REQUESTS_EXT_CREATE = R"SQL(
CREATE TABLE IF NOT EXISTS matrix_key_requests_ext (
    request_id          TEXT PRIMARY KEY NOT NULL,
    room_id             TEXT NOT NULL,
    session_id          TEXT NOT NULL,
    sender_key          TEXT NOT NULL,
    algorithm           TEXT DEFAULT 'm.megolm.v1.aes-sha2',
    state               TEXT DEFAULT 'unsent',
    requested_at        INTEGER NOT NULL,
    sent_at             INTEGER DEFAULT 0,
    cancelled_at        INTEGER DEFAULT 0,
    cancellation_txn_id TEXT
);
CREATE INDEX IF NOT EXISTS idx_key_requests_room ON matrix_key_requests_ext(room_id);
)SQL";

// Read markers / receipts
static constexpr const char* SQL_RECEIPTS_EXT_CREATE = R"SQL(
CREATE TABLE IF NOT EXISTS matrix_receipts_ext (
    room_id             TEXT NOT NULL,
    event_id            TEXT NOT NULL,
    user_id             TEXT NOT NULL,
    receipt_type        TEXT DEFAULT 'm.read',
    thread_id           TEXT DEFAULT 'main',
    ts                  INTEGER NOT NULL,
    PRIMARY KEY (room_id, event_id, user_id, receipt_type)
);
CREATE INDEX IF NOT EXISTS idx_receipts_ext_room_event ON matrix_receipts_ext(room_id, event_id);
)SQL";

// Media references
static constexpr const char* SQL_MEDIA_EXT_CREATE = R"SQL(
CREATE TABLE IF NOT EXISTS matrix_media_ext (
    mxc_uri             TEXT PRIMARY KEY NOT NULL,
    local_path          TEXT,
    thumbnail_path      TEXT,
    content_type        TEXT,
    file_size           INTEGER DEFAULT 0,
    width               INTEGER DEFAULT 0,
    height              INTEGER DEFAULT 0,
    duration_ms         INTEGER DEFAULT 0,
    upload_status       TEXT DEFAULT 'unknown',
    download_status     TEXT DEFAULT 'unknown',
    uploaded_at         INTEGER DEFAULT 0,
    downloaded_at       INTEGER DEFAULT 0,
    last_accessed_at    INTEGER DEFAULT 0,
    access_count        INTEGER DEFAULT 0,
    etag                TEXT,
    encryption_info     TEXT
);
CREATE INDEX IF NOT EXISTS idx_media_ext_status ON matrix_media_ext(download_status);
)SQL";

// ─────────────────────────────────────────────────────────────
//  Store CRUD structures
// ─────────────────────────────────────────────────────────────

struct SessionRecord {
    std::string userId;
    std::string accessToken;
    std::string refreshToken;
    std::string deviceId;
    std::string homeServer;
    std::string identityServer;
    std::string nextBatch;
    std::string syncFilterId;
    int syncTimeoutMs = 30000;
    std::string presenceStatus = "online";
    std::string avatarUrl;
    std::string displayName;
    bool notificationsEnabled = true;
    int64_t lastActiveTs = 0;
    int64_t lastSyncTs = 0;
    int64_t createdAt = 0;
    int64_t updatedAt = 0;
};

struct RoomRecord {
    std::string roomId;
    std::string name;
    std::string canonicalAlias;
    std::string topic;
    std::string avatarUrl;
    std::string joinRule;
    std::string guestAccess;
    std::string historyVisibility;
    std::string encryptionAlgorithm;
    bool isEncrypted = false;
    bool isDM = false;
    bool isSpace = false;
    bool isFavorite = false;
    bool isLowPriority = false;
    bool isDirect = false;
    std::string spaceParentId;
    std::string roomType;
    std::string membership = "join";
    int notificationCount = 0;
    int highlightCount = 0;
    int totalEventCount = 0;
    std::string lastMessagePreview;
    std::string lastMessageSender;
    int64_t lastMessageTs = 0;
    std::string lastMessageEventId;
    std::string lastReadEventId;
    int64_t lastReadTs = 0;
    std::string fullyReadEventId;
    int unreadCount = 0;
    double sortOrder = 0.0;
    int localSortOrder = 0;
    std::string draftMessage;
    std::vector<std::string> pinnedEventIds;
    std::string tombstoneReplacementRoom;
    std::string version = "1";
    std::string creatorUserId;
    bool federated = true;
    std::string predecessorRoomId;
    int64_t joinTs = 0;
    int64_t leaveTs = 0;
    int64_t createdTs = 0;
    int64_t updatedTs = 0;
};

struct EventRecord {
    std::string eventId;
    std::string roomId;
    std::string sender;
    std::string eventType;
    std::string stateKey;
    std::string content;
    int64_t originServerTs = 0;
    std::string unsignedData;
    std::string prevContent;
    std::string redacts;
    std::string redactedBecause;
    std::string txnId;
    std::string relatesToEventId;
    std::string relatesToType;
    std::string relatesToKey;
    std::string decryptedContent;
    std::string decryptionError;
    bool isDecrypted = false;
    bool isEncrypted = false;
    bool isRedacted = false;
    bool isOutgoing = false;
    std::string sendState = "sent";
    int64_t receivedTs = 0;
    int ageMs = 0;
    int64_t sortKey = 0;
};

struct UserRecord {
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    std::string avatarThumbnail;
    std::string statusMsg;
    std::string presence = "offline";
    int64_t lastActiveAgoMs = 0;
    bool currentlyActive = false;
    bool isIgnored = false;
    bool isBot = false;
    int powerLevelNorm = 0;
    int trustLevel = 0;
    std::string lastSeenRoomId;
    int64_t lastSeenTs = 0;
    int64_t firstSeenTs = 0;
    int64_t profileUpdatedTs = 0;
    int deviceCount = 0;
    std::string note;
};

struct MemberRecord {
    std::string roomId;
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    std::string membership = "leave";
    std::string reason;
    int powerLevel = 0;
    bool isRoomCreator = false;
    std::string senderId;
    int64_t originServerTs = 0;
    int64_t joinedAt = 0;
    int64_t invitedAt = 0;
    int64_t leftAt = 0;
};

struct DeviceKeyRecord {
    std::string userId;
    std::string deviceId;
    std::string displayName;
    int64_t lastSeenTs = 0;
    std::string lastSeenIp;
    std::string algorithms;
    std::string keysJson;
    std::string signaturesJson;
    bool isVerified = false;
    bool isBlocked = false;
    std::string verificationStatus = "unverified";
};

struct MediaRecord {
    std::string mxcUri;
    std::string localPath;
    std::string thumbnailPath;
    std::string contentType;
    int64_t fileSize = 0;
    int width = 0;
    int height = 0;
    int durationMs = 0;
    std::string uploadStatus;
    std::string downloadStatus;
    int64_t uploadedAt = 0;
    int64_t downloadedAt = 0;
    int64_t lastAccessedAt = 0;
    int accessCount = 0;
    std::string etag;
    std::string encryptionInfo;
};

// ─────────────────────────────────────────────────────────────
//  StoreExt — extended SQLite store with full CRUD
// ─────────────────────────────────────────────────────────────

class StoreExt {
public:
    StoreExt();
    explicit StoreExt(const std::string& dbPath);
    ~StoreExt();

    // ── Database lifecycle ──
    bool open(const std::string& dbPath);
    void close();
    bool isOpen() const { return m_db != nullptr; }
    const std::string& path() const { return m_dbPath; }

    // ── Schema ──
    bool createTables();
    bool migrateSchema(int fromVersion, int toVersion);
    int getSchemaVersion();
    bool vacuum();
    int64_t getDatabaseSize();

    // ── Sessions ──
    bool saveSession(const SessionRecord& session);
    std::optional<SessionRecord> loadSession(const std::string& userId);
    bool deleteSession(const std::string& userId);
    std::vector<SessionRecord> listSessions();
    bool updateSessionLastActive(const std::string& userId);
    bool updateSessionNextBatch(const std::string& userId, const std::string& nextBatch);
    bool updateSessionFilter(const std::string& userId, const std::string& filterId);
    bool updateSessionPresence(const std::string& userId, const std::string& presence);
    bool updateSessionTokens(const std::string& userId, const std::string& accessToken,
                              const std::string& refreshToken);

    // ── Rooms ──
    bool saveRoom(const RoomRecord& room);
    bool saveRooms(const std::vector<RoomRecord>& rooms);
    std::optional<RoomRecord> loadRoom(const std::string& roomId);
    std::vector<RoomRecord> loadRooms(const std::string& membership = "join");
    std::vector<RoomRecord> loadDMRooms();
    std::vector<RoomRecord> loadSpaceRooms();
    std::vector<RoomRecord> loadFavoriteRooms();
    std::vector<RoomRecord> searchRooms(const std::string& query, int limit = 20);
    bool updateRoomMembership(const std::string& roomId, const std::string& membership);
    bool updateRoomNotifications(const std::string& roomId, int notificationCount,
                                  int highlightCount);
    bool updateRoomLastMessage(const std::string& roomId, const std::string& preview,
                                const std::string& sender, int64_t ts,
                                const std::string& eventId);
    bool updateRoomReadMarker(const std::string& roomId, const std::string& eventId);
    bool updateRoomSortOrder(const std::string& roomId, double sortOrder);
    bool updateRoomFavorite(const std::string& roomId, bool isFavorite);
    bool updateRoomLowPriority(const std::string& roomId, bool isLowPriority);
    bool updateRoomDraftMessage(const std::string& roomId, const std::string& draft,
                                 const std::string& msgType);
    bool deleteRoom(const std::string& roomId);
    int countRooms(const std::string& membership = "");
    std::vector<std::string> getRoomIdsBySpace(const std::string& spaceId);
    bool setRoomPinnedEvents(const std::string& roomId,
                              const std::vector<std::string>& eventIds);
    std::vector<std::string> getRoomPinnedEvents(const std::string& roomId);

    // ── Events ──
    bool saveEvent(const EventRecord& event);
    bool saveEvents(const std::vector<EventRecord>& events);
    std::optional<EventRecord> loadEvent(const std::string& eventId,
                                          const std::string& roomId);
    std::vector<EventRecord> loadRoomEvents(const std::string& roomId,
                                             int64_t fromSortKey = 0,
                                             int limit = 50,
                                             bool ascending = false);
    std::vector<EventRecord> loadRoomEventsByType(const std::string& roomId,
                                                   const std::string& eventType,
                                                   int limit = 20);
    std::vector<EventRecord> loadRelatedEvents(const std::string& relatesToEventId);
    std::vector<EventRecord> loadPendingEvents(); // unsent outgoing
    bool updateEventSendState(const std::string& eventId, const std::string& roomId,
                               const std::string& sendState);
    bool updateEventDecrypted(const std::string& eventId, const std::string& roomId,
                               const std::string& decryptedContent, bool success = true,
                               const std::string& error = "");
    bool redactEvent(const std::string& eventId, const std::string& roomId,
                     const std::string& redactedBecause);
    bool deleteEvent(const std::string& eventId, const std::string& roomId);
    bool deleteRoomEvents(const std::string& roomId);
    int countRoomEvents(const std::string& roomId, const std::string& eventType = "");
    std::optional<EventRecord> getLastRoomEvent(const std::string& roomId);
    std::vector<EventRecord> searchEvents(const std::string& query, int limit = 20);

    // ── Users ──
    bool saveUser(const UserRecord& user);
    std::optional<UserRecord> loadUser(const std::string& userId);
    std::vector<UserRecord> loadUsers(int limit = 100, int offset = 0);
    bool updateUserPresence(const std::string& userId, const std::string& presence,
                             const std::string& statusMsg = "",
                             int64_t lastActiveAgoMs = 0);
    bool updateUserProfile(const std::string& userId, const std::string& displayName,
                            const std::string& avatarUrl);
    bool ignoreUser(const std::string& userId, bool ignore);
    bool deleteUser(const std::string& userId);
    std::vector<UserRecord> searchUsers(const std::string& query, int limit = 20);
    std::vector<UserRecord> getOnlineUsers(int limit = 50);

    // ── Room Members ──
    bool saveMember(const MemberRecord& member);
    bool saveMembers(const std::vector<MemberRecord>& members);
    std::vector<MemberRecord> loadRoomMembers(const std::string& roomId,
                                               const std::string& membership = "");
    std::optional<MemberRecord> loadRoomMember(const std::string& roomId,
                                                const std::string& userId);
    int countRoomMembers(const std::string& roomId, const std::string& membership = "join");
    bool deleteMember(const std::string& roomId, const std::string& userId);
    bool deleteRoomMembers(const std::string& roomId);

    // ── Device Keys ──
    bool saveDeviceKey(const DeviceKeyRecord& key);
    bool saveDeviceKeys(const std::vector<DeviceKeyRecord>& keys);
    std::vector<DeviceKeyRecord> loadDeviceKeys(const std::string& userId);
    std::optional<DeviceKeyRecord> loadDeviceKey(const std::string& userId,
                                                   const std::string& deviceId);
    bool updateDeviceVerification(const std::string& userId, const std::string& deviceId,
                                   bool verified, const std::string& status);
    bool blockDevice(const std::string& userId, const std::string& deviceId, bool block);
    bool deleteDeviceKey(const std::string& userId, const std::string& deviceId);
    bool deleteUserDeviceKeys(const std::string& userId);

    // ── Media ──
    bool saveMedia(const MediaRecord& media);
    std::optional<MediaRecord> loadMedia(const std::string& mxcUri);
    bool updateMediaDownload(const std::string& mxcUri, const std::string& localPath,
                              const std::string& thumbnailPath, int64_t size,
                              const std::string& contentType);
    bool updateMediaUpload(const std::string& mxcUri, const std::string& status);
    bool touchMedia(const std::string& mxcUri); // update last_accessed
    bool deleteMedia(const std::string& mxcUri);
    std::vector<MediaRecord> getMediaByStatus(const std::string& downloadStatus,
                                               int limit = 100);
    int64_t getTotalMediaSize();

    // ── Key Requests ──
    bool saveKeyRequest(const std::string& requestId, const std::string& roomId,
                         const std::string& sessionId, const std::string& senderKey,
                         const std::string& algorithm = "m.megolm.v1.aes-sha2");
    bool updateKeyRequestState(const std::string& requestId, const std::string& state);
    bool cancelKeyRequest(const std::string& requestId, const std::string& txnId);
    std::vector<std::string> getPendingKeyRequests();

    // ── Receipts ──
    bool saveReceipt(const std::string& roomId, const std::string& eventId,
                      const std::string& userId, const std::string& receiptType = "m.read",
                      const std::string& threadId = "main");
    std::vector<std::string> getReadReceipts(const std::string& roomId,
                                              const std::string& eventId);

    // ── Maintenance ──
    bool clearExpiredData(int64_t olderThanMs);
    bool trimDatabase(int maxEventsPerRoom = 1000);
    bool exportDatabase(const std::string& exportPath);

private:
    std::string m_dbPath;
    void* m_db = nullptr; // sqlite3*

    // Internal helpers
    bool execSql(const std::string& sql);
    bool execSql(const char* sql);
    bool prepareAndBind(const std::string& sql,
                        std::function<bool(void* stmt)> binder);
    std::string escapeString(const std::string& input) const;
    int64_t nowMs() const;

    // Schema version management
    static constexpr int SCHEMA_VERSION = 3;
    bool ensureSchemaVersion();
    bool runMigration(int fromVersion, int toVersion);
    bool migrateV1ToV2();
    bool migrateV2ToV3();
};

} // namespace matrix_sdk
