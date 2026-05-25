#ifndef MATRIX_SDK_DATABASE_HPP
#define MATRIX_SDK_DATABASE_HPP

#include "matrix_sdk_types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace matrix_sdk {

// ── Database Config ──
struct DatabaseConfig {
    std::string databasePath;
    std::string databaseName;
    int version = 1;
    bool encryptDatabase = false;
    std::string encryptionKey;
};

// ── Room Entity ──
struct RoomEntity {
    std::string roomId;
    std::string displayName;
    std::string topic;
    std::string canonicalAlias;
    std::string avatarUrl;
    std::string lastMessageBody;
    std::string lastMessageSenderId;
    int64_t lastMessageTs = 0;
    int joinedMembersCount = 0;
    int invitedMembersCount = 0;
    int notificationCount = 0;
    int highlightCount = 0;
    int unreadCount = 0;
    bool isDirect = false;
    bool isEncrypted = false;
    bool isPublic = false;
    bool isFavourite = false;
    bool isLowPriority = false;
    std::string membership;
    std::string roomType;
    std::string notificationState;
    std::string readMarkerEventId;
    std::string readReceiptEventId;
};

// ── Event Entity ──
struct EventEntity {
    std::string eventId;
    std::string roomId;
    std::string senderId;
    int64_t originServerTs = 0;
    int64_t age = 0;
    std::string type;
    std::string stateKey;
    std::string contentJson;
    std::string prevContentJson;
    std::string redactsEventId;
    std::string redactedBecause;
    std::string senderName;
    std::string senderAvatarUrl;
    bool isEncrypted = false;
    bool isRedacted = false;
    std::string decryptedContent;
    std::string decryptedType;
    std::string transactionId;
    std::string sendState;
    bool isLocalEcho = false;
};

// ── Room Member Entity ──
struct RoomMemberEntity {
    std::string roomId;
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    std::string membership;
    bool isDirect = false;
    int64_t originServerTs = 0;
};

// ── Cross Signing Entity ──
struct CrossSigningEntity {
    std::string userId;
    std::string masterKey;
    std::string selfSignedKey;
    std::string userSignedKey;
    bool isTrusted = false;
    int64_t lastUpdated = 0;
};

// ── Key Backup Entity ──
struct KeyBackupEntity {
    std::string version;
    std::string algorithm;
    std::string authData;
    int totalKeys = 0;
    int backedUpKeys = 0;
    int64_t lastBackupTimestamp = 0;
    bool isEnabled = false;
};

// ── Database Interface ──
class MatrixDatabase {
public:
    virtual ~MatrixDatabase() = default;

    // ── Initialization ──
    virtual void open(const DatabaseConfig& config, SimpleCallback callback) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual void clear(SimpleCallback callback) = 0;

    // ── Transactions ──
    virtual void beginTransaction() = 0;
    virtual void commitTransaction() = 0;
    virtual void rollbackTransaction() = 0;
    virtual bool isInTransaction() const = 0;

    // ── Room Operations ──
    virtual void insertRoom(const RoomEntity& room) = 0;
    virtual void updateRoom(const RoomEntity& room) = 0;
    virtual void deleteRoom(const std::string& roomId) = 0;
    virtual std::optional<RoomEntity> getRoom(const std::string& roomId) const = 0;
    virtual std::vector<RoomEntity> getAllRooms() const = 0;
    virtual std::vector<RoomEntity> getRoomsByType(const std::string& roomType) const = 0;
    virtual int getRoomCount() const = 0;
    virtual void updateRoomDisplayName(const std::string& roomId, const std::string& name) = 0;
    virtual void updateRoomAvatar(const std::string& roomId, const std::string& avatarUrl) = 0;
    virtual void updateRoomMembership(const std::string& roomId, const std::string& membership) = 0;
    virtual void updateRoomNotificationCount(const std::string& roomId, int notificationCount, int highlightCount) = 0;
    virtual void updateRoomReadMarker(const std::string& roomId, const std::string& eventId) = 0;

    // ── Event Operations ──
    virtual void insertEvent(const EventEntity& event) = 0;
    virtual void insertEvents(const std::vector<EventEntity>& events) = 0;
    virtual void updateEvent(const EventEntity& event) = 0;
    virtual void deleteEvent(const std::string& eventId) = 0;
    virtual void deleteEventsForRoom(const std::string& roomId) = 0;
    virtual std::optional<EventEntity> getEvent(const std::string& eventId) const = 0;
    virtual std::vector<EventEntity> getEventsForRoom(const std::string& roomId, int limit, int offset = 0) const = 0;
    virtual std::vector<EventEntity> getEventsSince(const std::string& roomId, const std::string& eventId, int limit) const = 0;
    virtual std::vector<EventEntity> getEventsBefore(const std::string& roomId, const std::string& eventId, int limit) const = 0;
    virtual std::vector<EventEntity> getEventsByType(const std::string& roomId, const std::string& type, int limit) const = 0;
    virtual int getEventCountForRoom(const std::string& roomId) const = 0;
    virtual std::string getLastEventIdForRoom(const std::string& roomId) const = 0;

    // ── Room Member Operations ──
    virtual void insertRoomMember(const RoomMemberEntity& member) = 0;
    virtual void insertRoomMembers(const std::vector<RoomMemberEntity>& members) = 0;
    virtual void updateRoomMember(const RoomMemberEntity& member) = 0;
    virtual void deleteRoomMember(const std::string& roomId, const std::string& userId) = 0;
    virtual void deleteRoomMembers(const std::string& roomId) = 0;
    virtual std::optional<RoomMemberEntity> getRoomMember(const std::string& roomId, const std::string& userId) const = 0;
    virtual std::vector<RoomMemberEntity> getRoomMembers(const std::string& roomId) const = 0;
    virtual std::vector<RoomMemberEntity> getJoinedRoomMembers(const std::string& roomId) const = 0;
    virtual int getRoomMemberCount(const std::string& roomId) const = 0;

    // ── Cross-Signing Operations ──
    virtual void insertCrossSigning(const CrossSigningEntity& entity) = 0;
    virtual void updateCrossSigning(const CrossSigningEntity& entity) = 0;
    virtual void deleteCrossSigning(const std::string& userId) = 0;
    virtual std::optional<CrossSigningEntity> getCrossSigning(const std::string& userId) const = 0;

    // ── Key Backup Operations ──
    virtual void insertKeyBackup(const KeyBackupEntity& entity) = 0;
    virtual void updateKeyBackup(const KeyBackupEntity& entity) = 0;
    virtual void deleteKeyBackup(const std::string& version) = 0;
    virtual std::optional<KeyBackupEntity> getKeyBackup(const std::string& version) const = 0;
    virtual std::optional<KeyBackupEntity> getLatestKeyBackup() const = 0;

    // ── Session Data ──
    virtual void insertSessionData(const std::string& key, const std::string& value) = 0;
    virtual std::optional<std::string> getSessionData(const std::string& key) const = 0;
    virtual void deleteSessionData(const std::string& key) = 0;

    // ── Account Data ──
    virtual void insertAccountData(const std::string& type, const std::string& content) = 0;
    virtual std::optional<std::string> getAccountData(const std::string& type) const = 0;
    virtual std::map<std::string, std::string> getAllAccountData() const = 0;
    virtual void deleteAccountData(const std::string& type) = 0;

    // ── Sync Token ──
    virtual void setSyncToken(const std::string& token) = 0;
    virtual std::string getSyncToken() const = 0;

    // ── Maintenance ──
    virtual void vacuum() = 0;
    virtual int64_t getDatabaseSize() const = 0;
    virtual void optimize() = 0;
};

// ── Database Factory ──
class DatabaseFactory {
public:
    virtual ~DatabaseFactory() = default;
    virtual std::shared_ptr<MatrixDatabase> createDatabase(const DatabaseConfig& config) = 0;
};

} // namespace matrix_sdk

#endif
