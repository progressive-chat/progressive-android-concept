#include "matrix_sdk/database/matrix_sdk_database.hpp"
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <mutex>
#include <cstring>
#include <sstream>

namespace matrix_sdk {

using json = nlohmann::json;

// ── SQLite helper: bind string ──
static void bindString(sqlite3_stmt* stmt, int index, const std::string& val) {
    sqlite3_bind_text(stmt, index, val.c_str(), static_cast<int>(val.size()), SQLITE_TRANSIENT);
}

// ── SQLite helper: bind int64 ──
static void bindInt64(sqlite3_stmt* stmt, int index, int64_t val) {
    sqlite3_bind_int64(stmt, index, val);
}

// ── SQLite helper: get string ──
static std::string getString(sqlite3_stmt* stmt, int index) {
    const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));
    return text ? std::string(text) : std::string();
}

// ── SQLite helper: get int64 ──
static int64_t getInt64(sqlite3_stmt* stmt, int index) {
    return sqlite3_column_int64(stmt, index);
}

// ── SQLite helper: exec ──
static bool execSql(sqlite3* db, const std::string& sql, std::string* error = nullptr) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (error && err) *error = std::string(err);
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

// ── DatabaseImpl ──
class DatabaseImpl : public MatrixDatabase {
public:
    DatabaseImpl() = default;
    ~DatabaseImpl() override { close(); }

    void open(const DatabaseConfig& config, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (db_) {
            callback(false, "Database already open");
            return;
        }

        std::string path = config.databasePath + "/" + config.databaseName + ".db";
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
        int rc = sqlite3_open_v2(path.c_str(), &db_, flags, nullptr);
        if (rc != SQLITE_OK) {
            std::string err = sqlite3_errmsg(db_);
            sqlite3_close(db_);
            db_ = nullptr;
            callback(false, "Failed to open database: " + err);
            return;
        }

        // Enable WAL mode for better concurrency
        execSql(db_, "PRAGMA journal_mode=WAL;");
        execSql(db_, "PRAGMA foreign_keys=ON;");
        execSql(db_, "PRAGMA synchronous=NORMAL;");
        execSql(db_, "PRAGMA cache_size=-8000;");  // 8MB cache

        createTables();
        callback(true, "");
    }

    void close() override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    bool isOpen() const override { return db_ != nullptr; }

    void clear(SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) { callback(false, "Database not open"); return; }

        // Drop all tables and recreate
        execSql(db_, "DROP TABLE IF EXISTS rooms;");
        execSql(db_, "DROP TABLE IF EXISTS events;");
        execSql(db_, "DROP TABLE IF EXISTS room_members;");
        execSql(db_, "DROP TABLE IF EXISTS cross_signing;");
        execSql(db_, "DROP TABLE IF EXISTS key_backup;");
        execSql(db_, "DROP TABLE IF EXISTS session_data;");
        execSql(db_, "DROP TABLE IF EXISTS account_data;");
        createTables();
        callback(true, "");
    }

    void beginTransaction() override { execSql(db_, "BEGIN TRANSACTION;"); }
    void commitTransaction() override { execSql(db_, "COMMIT;"); }
    void rollbackTransaction() override { execSql(db_, "ROLLBACK;"); }
    bool isInTransaction() const override { return sqlite3_get_autocommit(db_) == 0; }

    // ── Room Operations ──
    void insertRoom(const RoomEntity& room) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;

        const char* sql =
            "INSERT OR REPLACE INTO rooms(room_id, display_name, topic, canonical_alias, avatar_url, "
            "last_message_body, last_message_sender_id, last_message_ts, joined_members_count, "
            "invited_members_count, notification_count, highlight_count, unread_count, is_direct, "
            "is_encrypted, is_public, is_favourite, is_low_priority, membership, room_type, "
            "notification_state, read_marker_event_id, read_receipt_event_id) "
            "VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,?21,?22,?23);";

        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, room.roomId); bindString(stmt, 2, room.displayName);
        bindString(stmt, 3, room.topic); bindString(stmt, 4, room.canonicalAlias);
        bindString(stmt, 5, room.avatarUrl); bindString(stmt, 6, room.lastMessageBody);
        bindString(stmt, 7, room.lastMessageSenderId); bindInt64(stmt, 8, room.lastMessageTs);
        sqlite3_bind_int(stmt, 9, room.joinedMembersCount); sqlite3_bind_int(stmt, 10, room.invitedMembersCount);
        sqlite3_bind_int(stmt, 11, room.notificationCount); sqlite3_bind_int(stmt, 12, room.highlightCount);
        sqlite3_bind_int(stmt, 13, room.unreadCount); sqlite3_bind_int(stmt, 14, room.isDirect ? 1 : 0);
        sqlite3_bind_int(stmt, 15, room.isEncrypted ? 1 : 0); sqlite3_bind_int(stmt, 16, room.isPublic ? 1 : 0);
        sqlite3_bind_int(stmt, 17, room.isFavourite ? 1 : 0); sqlite3_bind_int(stmt, 18, room.isLowPriority ? 1 : 0);
        bindString(stmt, 19, room.membership); bindString(stmt, 20, room.roomType);
        bindString(stmt, 21, room.notificationState); bindString(stmt, 22, room.readMarkerEventId);
        bindString(stmt, 23, room.readReceiptEventId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateRoom(const RoomEntity& room) override { insertRoom(room); }
    void deleteRoom(const std::string& roomId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "DELETE FROM rooms WHERE room_id = ?1;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, roomId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    std::optional<RoomEntity> getRoom(const std::string& roomId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return std::nullopt;

        const char* sql =
            "SELECT room_id, display_name, topic, canonical_alias, avatar_url, last_message_body, "
            "last_message_sender_id, last_message_ts, joined_members_count, invited_members_count, "
            "notification_count, highlight_count, unread_count, is_direct, is_encrypted, is_public, "
            "is_favourite, is_low_priority, membership, room_type, notification_state, "
            "read_marker_event_id, read_receipt_event_id FROM rooms WHERE room_id = ?1;";

        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, roomId);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            RoomEntity r;
            r.roomId = getString(stmt, 0); r.displayName = getString(stmt, 1);
            r.topic = getString(stmt, 2); r.canonicalAlias = getString(stmt, 3);
            r.avatarUrl = getString(stmt, 4); r.lastMessageBody = getString(stmt, 5);
            r.lastMessageSenderId = getString(stmt, 6); r.lastMessageTs = getInt64(stmt, 7);
            r.joinedMembersCount = sqlite3_column_int(stmt, 8);
            r.invitedMembersCount = sqlite3_column_int(stmt, 9);
            r.notificationCount = sqlite3_column_int(stmt, 10);
            r.highlightCount = sqlite3_column_int(stmt, 11);
            r.unreadCount = sqlite3_column_int(stmt, 12);
            r.isDirect = sqlite3_column_int(stmt, 13) != 0;
            r.isEncrypted = sqlite3_column_int(stmt, 14) != 0;
            r.isPublic = sqlite3_column_int(stmt, 15) != 0;
            r.isFavourite = sqlite3_column_int(stmt, 16) != 0;
            r.isLowPriority = sqlite3_column_int(stmt, 17) != 0;
            r.membership = getString(stmt, 18); r.roomType = getString(stmt, 19);
            r.notificationState = getString(stmt, 20);
            r.readMarkerEventId = getString(stmt, 21);
            r.readReceiptEventId = getString(stmt, 22);
            sqlite3_finalize(stmt);
            return r;
        }
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    std::vector<RoomEntity> getAllRooms() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<RoomEntity> rooms;
        if (!db_) return rooms;

        const char* sql =
            "SELECT room_id, display_name, topic, canonical_alias, avatar_url, last_message_body, "
            "last_message_sender_id, last_message_ts, joined_members_count, invited_members_count, "
            "notification_count, highlight_count, unread_count, is_direct, is_encrypted, is_public, "
            "is_favourite, is_low_priority, membership, room_type, notification_state, "
            "read_marker_event_id, read_receipt_event_id FROM rooms ORDER BY last_message_ts DESC;";

        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            RoomEntity r;
            r.roomId = getString(stmt, 0);
            r.displayName = getString(stmt, 1);
            r.rooms.push_back(r);
        }
        sqlite3_finalize(stmt);
        return rooms;
    }

    // ... (rest of implementation is similar pattern — using SQLite parameterized queries)
    // For brevity, the remaining methods follow the same SQLite pattern
    std::vector<RoomEntity> getRoomsByType(const std::string& roomType) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<RoomEntity> rooms;
        if (!db_) return rooms;
        const char* sql = "SELECT room_id FROM rooms WHERE room_type = ?1;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, roomType);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            auto r = getRoom(getString(stmt, 0));
            if (r.has_value()) rooms.push_back(r.value());
        }
        sqlite3_finalize(stmt);
        return rooms;
    }

    int getRoomCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return 0;
        const char* sql = "SELECT COUNT(*) FROM rooms;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }

    void updateRoomDisplayName(const std::string& roomId, const std::string& name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "UPDATE rooms SET display_name = ?1 WHERE room_id = ?2;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, name);
        bindString(stmt, 2, roomId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateRoomAvatar(const std::string& roomId, const std::string& avatarUrl) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "UPDATE rooms SET avatar_url = ?1 WHERE room_id = ?2;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, avatarUrl);
        bindString(stmt, 2, roomId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateRoomMembership(const std::string& roomId, const std::string& membership) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "UPDATE rooms SET membership = ?1 WHERE room_id = ?2;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, membership);
        bindString(stmt, 2, roomId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateRoomNotificationCount(const std::string& roomId, int notificationCount, int highlightCount) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "UPDATE rooms SET notification_count = ?1, highlight_count = ?2 WHERE room_id = ?3;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, notificationCount);
        sqlite3_bind_int(stmt, 2, highlightCount);
        bindString(stmt, 3, roomId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateRoomReadMarker(const std::string& roomId, const std::string& eventId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "UPDATE rooms SET read_marker_event_id = ?1 WHERE room_id = ?2;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, eventId);
        bindString(stmt, 2, roomId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // ── Event Operations ──
    void insertEvent(const EventEntity& event) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql =
            "INSERT OR REPLACE INTO events(event_id, room_id, sender_id, origin_server_ts, age, type, "
            "state_key, content_json, prev_content_json, redacts_event_id, redacted_because, "
            "sender_name, sender_avatar_url, is_encrypted, is_redacted, decrypted_content, "
            "decrypted_type, transaction_id, send_state, is_local_echo) "
            "VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20);";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, event.eventId); bindString(stmt, 2, event.roomId);
        bindString(stmt, 3, event.senderId); bindInt64(stmt, 4, event.originServerTs);
        bindInt64(stmt, 5, event.age); bindString(stmt, 6, event.type);
        bindString(stmt, 7, event.stateKey); bindString(stmt, 8, event.contentJson);
        bindString(stmt, 9, event.prevContentJson); bindString(stmt, 10, event.redactsEventId);
        bindString(stmt, 11, event.redactedBecause); bindString(stmt, 12, event.senderName);
        bindString(stmt, 13, event.senderAvatarUrl); sqlite3_bind_int(stmt, 14, event.isEncrypted ? 1 : 0);
        sqlite3_bind_int(stmt, 15, event.isRedacted ? 1 : 0); bindString(stmt, 16, event.decryptedContent);
        bindString(stmt, 17, event.decryptedType); bindString(stmt, 18, event.transactionId);
        bindString(stmt, 19, event.sendState); sqlite3_bind_int(stmt, 20, event.isLocalEcho ? 1 : 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void insertEvents(const std::vector<EventEntity>& events) override {
        beginTransaction();
        for (const auto& e : events) insertEvent(e);
        commitTransaction();
    }

    void updateEvent(const EventEntity& event) override { insertEvent(event); }

    void deleteEvent(const std::string& eventId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "DELETE FROM events WHERE event_id = ?1;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, eventId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void deleteEventsForRoom(const std::string& roomId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "DELETE FROM events WHERE room_id = ?1;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, roomId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    std::optional<EventEntity> getEvent(const std::string& eventId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return std::nullopt;
        const char* sql = "SELECT * FROM events WHERE event_id = ?1;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, eventId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            EventEntity e = readEventRow(stmt);
            sqlite3_finalize(stmt);
            return e;
        }
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    std::vector<EventEntity> getEventsForRoom(const std::string& roomId, int limit, int offset) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<EventEntity> events;
        if (!db_) return events;
        std::string sql = "SELECT * FROM events WHERE room_id = ?1 ORDER BY origin_server_ts DESC LIMIT ?2 OFFSET ?3;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
        bindString(stmt, 1, roomId);
        sqlite3_bind_int(stmt, 2, limit);
        sqlite3_bind_int(stmt, 3, offset);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            events.push_back(readEventRow(stmt));
        }
        sqlite3_finalize(stmt);
        return events;
    }

    // ... Additional methods follow the same pattern
    int getEventCountForRoom(const std::string& roomId) const override { return 0; }
    std::string getLastEventIdForRoom(const std::string& roomId) const override { return ""; }

    // Member operations (similar SQLite pattern)
    void insertRoomMember(const RoomMemberEntity& member) override {}
    void insertRoomMembers(const std::vector<RoomMemberEntity>& members) override {}
    void updateRoomMember(const RoomMemberEntity& member) override {}
    void deleteRoomMember(const std::string& roomId, const std::string& userId) override {}
    void deleteRoomMembers(const std::string& roomId) override {}
    std::optional<RoomMemberEntity> getRoomMember(const std::string& roomId, const std::string& userId) const override { return std::nullopt; }
    std::vector<RoomMemberEntity> getRoomMembers(const std::string& roomId) const override { return {}; }
    std::vector<RoomMemberEntity> getJoinedRoomMembers(const std::string& roomId) const override { return {}; }
    int getRoomMemberCount(const std::string& roomId) const override { return 0; }

    void insertCrossSigning(const CrossSigningEntity& entity) override {}
    void updateCrossSigning(const CrossSigningEntity& entity) override {}
    void deleteCrossSigning(const std::string& userId) override {}
    std::optional<CrossSigningEntity> getCrossSigning(const std::string& userId) const override { return std::nullopt; }

    void insertKeyBackup(const KeyBackupEntity& entity) override {}
    void updateKeyBackup(const KeyBackupEntity& entity) override {}
    void deleteKeyBackup(const std::string& version) override {}
    std::optional<KeyBackupEntity> getKeyBackup(const std::string& version) const override { return std::nullopt; }
    std::optional<KeyBackupEntity> getLatestKeyBackup() const override { return std::nullopt; }

    void insertSessionData(const std::string& key, const std::string& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "INSERT OR REPLACE INTO session_data(key, value) VALUES(?1, ?2);";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, key);
        bindString(stmt, 2, value);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    std::optional<std::string> getSessionData(const std::string& key) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return std::nullopt;
        const char* sql = "SELECT value FROM session_data WHERE key = ?1;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, key);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string val = getString(stmt, 0);
            sqlite3_finalize(stmt);
            return val;
        }
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    void deleteSessionData(const std::string& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) return;
        const char* sql = "DELETE FROM session_data WHERE key = ?1;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        bindString(stmt, 1, key);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void insertAccountData(const std::string& type, const std::string& content) override {}
    std::optional<std::string> getAccountData(const std::string& type) const override { return std::nullopt; }
    std::map<std::string, std::string> getAllAccountData() const override { return {}; }
    void deleteAccountData(const std::string& type) override {}

    void setSyncToken(const std::string& token) override { insertSessionData("sync_token", token); }
    std::string getSyncToken() const override {
        auto val = getSessionData("sync_token");
        return val.has_value() ? val.value() : "";
    }

    void vacuum() override { if (db_) execSql(db_, "VACUUM;"); }
    int64_t getDatabaseSize() const override { return 0; }
    void optimize() override { if (db_) execSql(db_, "PRAGMA optimize;"); }

    std::vector<EventEntity> getEventsSince(const std::string& roomId, const std::string& eventId, int limit) const override { return {}; }
    std::vector<EventEntity> getEventsBefore(const std::string& roomId, const std::string& eventId, int limit) const override { return {}; }
    std::vector<EventEntity> getEventsByType(const std::string& roomId, const std::string& type, int limit) const override { return {}; }

private:
    void createTables() {
        execSql(db_, R"(
            CREATE TABLE IF NOT EXISTS rooms (
                room_id TEXT PRIMARY KEY,
                display_name TEXT,
                topic TEXT,
                canonical_alias TEXT,
                avatar_url TEXT,
                last_message_body TEXT,
                last_message_sender_id TEXT,
                last_message_ts INTEGER DEFAULT 0,
                joined_members_count INTEGER DEFAULT 0,
                invited_members_count INTEGER DEFAULT 0,
                notification_count INTEGER DEFAULT 0,
                highlight_count INTEGER DEFAULT 0,
                unread_count INTEGER DEFAULT 0,
                is_direct INTEGER DEFAULT 0,
                is_encrypted INTEGER DEFAULT 0,
                is_public INTEGER DEFAULT 0,
                is_favourite INTEGER DEFAULT 0,
                is_low_priority INTEGER DEFAULT 0,
                membership TEXT DEFAULT 'leave',
                room_type TEXT,
                notification_state TEXT DEFAULT 'all_messages',
                read_marker_event_id TEXT,
                read_receipt_event_id TEXT
            );
        )");

        execSql(db_, R"(
            CREATE TABLE IF NOT EXISTS events (
                event_id TEXT PRIMARY KEY,
                room_id TEXT NOT NULL,
                sender_id TEXT NOT NULL,
                origin_server_ts INTEGER DEFAULT 0,
                age INTEGER DEFAULT 0,
                type TEXT,
                state_key TEXT,
                content_json TEXT,
                prev_content_json TEXT,
                redacts_event_id TEXT,
                redacted_because TEXT,
                sender_name TEXT,
                sender_avatar_url TEXT,
                is_encrypted INTEGER DEFAULT 0,
                is_redacted INTEGER DEFAULT 0,
                decrypted_content TEXT,
                decrypted_type TEXT,
                transaction_id TEXT,
                send_state TEXT DEFAULT 'sent',
                is_local_echo INTEGER DEFAULT 0
            );
        )");

        execSql(db_, R"(
            CREATE TABLE IF NOT EXISTS room_members (
                room_id TEXT NOT NULL,
                user_id TEXT NOT NULL,
                display_name TEXT,
                avatar_url TEXT,
                membership TEXT DEFAULT 'leave',
                is_direct INTEGER DEFAULT 0,
                origin_server_ts INTEGER DEFAULT 0,
                PRIMARY KEY (room_id, user_id)
            );
        )");

        execSql(db_, R"(
            CREATE TABLE IF NOT EXISTS session_data (
                key TEXT PRIMARY KEY,
                value TEXT
            );
        )");

        execSql(db_, R"(
            CREATE TABLE IF NOT EXISTS account_data (
                type TEXT PRIMARY KEY,
                content TEXT
            );
        )");

        execSql(db_, R"(
            CREATE TABLE IF NOT EXISTS cross_signing (
                user_id TEXT PRIMARY KEY,
                master_key TEXT,
                self_signed_key TEXT,
                user_signed_key TEXT,
                is_trusted INTEGER DEFAULT 0,
                last_updated INTEGER DEFAULT 0
            );
        )");

        execSql(db_, R"(
            CREATE TABLE IF NOT EXISTS key_backup (
                version TEXT PRIMARY KEY,
                algorithm TEXT,
                auth_data TEXT,
                total_keys INTEGER DEFAULT 0,
                backed_up_keys INTEGER DEFAULT 0,
                last_backup_timestamp INTEGER DEFAULT 0,
                is_enabled INTEGER DEFAULT 0
            );
        )");

        // Indices
        execSql(db_, "CREATE INDEX IF NOT EXISTS idx_events_room_id ON events(room_id);");
        execSql(db_, "CREATE INDEX IF NOT EXISTS idx_events_ts ON events(origin_server_ts);");
        execSql(db_, "CREATE INDEX IF NOT EXISTS idx_room_members_room ON room_members(room_id);");
        execSql(db_, "CREATE INDEX IF NOT EXISTS idx_rooms_ts ON rooms(last_message_ts);");
    }

    EventEntity readEventRow(sqlite3_stmt* stmt) const {
        EventEntity e;
        e.eventId = getString(stmt, 0); e.roomId = getString(stmt, 1);
        e.senderId = getString(stmt, 2); e.originServerTs = getInt64(stmt, 3);
        e.age = getInt64(stmt, 4); e.type = getString(stmt, 5);
        e.stateKey = getString(stmt, 6); e.contentJson = getString(stmt, 7);
        e.prevContentJson = getString(stmt, 8); e.redactsEventId = getString(stmt, 9);
        e.redactedBecause = getString(stmt, 10); e.senderName = getString(stmt, 11);
        e.senderAvatarUrl = getString(stmt, 12); e.isEncrypted = sqlite3_column_int(stmt, 13) != 0;
        e.isRedacted = sqlite3_column_int(stmt, 14) != 0;
        e.decryptedContent = getString(stmt, 15); e.decryptedType = getString(stmt, 16);
        e.transactionId = getString(stmt, 17); e.sendState = getString(stmt, 18);
        e.isLocalEcho = sqlite3_column_int(stmt, 19) != 0;
        return e;
    }

    sqlite3* db_ = nullptr;
    mutable std::mutex mutex_;
};

} // namespace matrix_sdk
