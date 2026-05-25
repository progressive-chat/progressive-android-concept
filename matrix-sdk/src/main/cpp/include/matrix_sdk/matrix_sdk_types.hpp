#ifndef MATRIX_SDK_TYPES_HPP
#define MATRIX_SDK_TYPES_HPP

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann::json;

// ── Event Types ──
enum class EventType {
    ROOM_MESSAGE,
    ROOM_MESSAGE_ENCRYPTED,
    ROOM_MEMBER,
    ROOM_CREATE,
    ROOM_JOIN_RULES,
    ROOM_GUEST_ACCESS,
    ROOM_HISTORY_VISIBILITY,
    ROOM_NAME,
    ROOM_TOPIC,
    ROOM_AVATAR,
    ROOM_CANONICAL_ALIAS,
    ROOM_ALIASES,
    ROOM_POWER_LEVELS,
    ROOM_TOMBSTONE,
    ROOM_ENCRYPTION,
    ROOM_REDACTION,
    ROOM_THIRD_PARTY_INVITE,
    ROOM_PINNED_EVENTS,
    ROOM_SERVER_ACL,
    TYPING,
    PRESENCE,
    RECEIPT,
    FULLY_READ,
    TAG,
    DIRECT,
    IGNORED_USER_LIST,
    PUSH_RULES,
    REACTION,
    CALL_INVITE,
    CALL_CANDIDATES,
    CALL_ANSWER,
    CALL_HANGUP,
    CALL_NEGOTIATE,
    CALL_REJECT,
    POLL_START,
    POLL_RESPONSE,
    POLL_END,
    LOCATION,
    STICKER,
    WIDGET,
    KEY_VERIFICATION_START,
    KEY_VERIFICATION_ACCEPT,
    KEY_VERIFICATION_KEY,
    KEY_VERIFICATION_MAC,
    KEY_VERIFICATION_CANCEL,
    KEY_VERIFICATION_DONE,
    ROOM_KEY,
    ROOM_KEY_REQUEST,
    FORWARDED_ROOM_KEY,
    MEGOLM_KEY,
    OLMLM_KEY,
    DEVICE_LIST,
    DUMMY,
    UNKNOWN
};

// ── Message Types ──
enum class MessageType {
    TEXT,
    NOTICE,
    EMOTE,
    IMAGE,
    FILE,
    AUDIO,
    VIDEO,
    LOCATION,
    NONE
};

// ── Membership States ──
enum class Membership {
    INVITE,
    JOIN,
    KNOCK,
    LEAVE,
    BAN,
    NONE
};

// ── Preset Values ──
enum class RoomPreset {
    PRIVATE_CHAT,
    TRUSTED_PRIVATE_CHAT,
    PUBLIC_CHAT
};

// ── Visibility ──
enum class RoomVisibility {
    PRIVATE,
    PUBLIC
};

// ── Join Rules ──
enum class JoinRule {
    PUBLIC,
    KNOCK,
    INVITE,
    PRIVATE,
    RESTRICTED
};

// ── History Visibility ──
enum class HistoryVisibility {
    INVITED,
    JOINED,
    SHARED,
    WORLD_READABLE
};

// ── Encryption Algorithm ──
enum class EncryptionAlgorithm {
    MEGOLM,
    OLM
};

// ── Sync State ──
enum class SyncState {
    IDLE,
    SYNCING,
    PAUSED,
    ERROR,
    TERMINATED
};

// ── Connection State ──
enum class ConnectionState {
    CONNECTED,
    CONNECTING,
    DISCONNECTED,
    RECONNECTING
};

// ── Push Rule Kind ──
enum class PushRuleKind {
    OVERRIDE,
    UNDER,
    SENDER,
    ROOM,
    CONTENT,
    USER
};

// ── Push Rule Action ──
enum class PushRuleActionType {
    NOTIFY,
    DONT_NOTIFY,
    COALESCE,
    SET_TWEAK
};

// ── Crypto Device Verification Status ──
enum class DeviceVerification {
    UNVERIFIED,
    VERIFIED,
    BLOCKED
};

// ── Cross Signing State ──
enum class CrossSigningState {
    NOT_BOOTSTRAPPED,
    CROSS_SIGNING_EXISTS,
    TRUSTED
};

// ── Key Backup State ──
enum class KeyBackupState {
    UNKNOWN,
    DISABLED,
    ENABLED,
    NOT_TRUSTED,
    ENABLING,
    DELETING
};

// ── Room Notification State ──
enum class RoomNotificationState {
    ALL_MESSAGES,
    ALL_MESSAGES_NO_SOUND,
    MENTIONS_ONLY,
    MUTE
};

// ── Send State ──
enum class SendState {
    UNSENT,
    SENDING,
    SENT,
    UNDELIVERABLE,
    FAILED_UNKNOWN_DEVICES
};

// ── Relation Type ──
enum class RelationType {
    THREAD,
    REPLACE,
    REFERENCE,
    ANNOTATION
};

// ── Forward declarations ──
class Session;
class Room;
class User;
class CryptoService;
class SyncService;
class EventService;
class RoomService;
class UserService;
class PushRuleService;
class RoomSummary;
class TimelineEvent;

// ── Common Callback Types ──
template<typename T>
using Callback = std::function<void(bool success, const std::string& error, const T& result)>;

using SimpleCallback = std::function<void(bool success, const std::string& error)>;
using ProgressCallback = std::function<void(int progress, int total)>;
using StateCallback = std::function<void(const std::string& state)>;

// ── Optional helper (C++17 std::optional already available) ──
template<typename T>
using Optional = std::optional<T>;

// ── User ID ──
struct UserId {
    std::string localpart;
    std::string server_name;

    std::string full() const { return "@" + localpart + ":" + server_name; }
    static UserId parse(const std::string& id);
    bool isValid() const { return !localpart.empty() && !server_name.empty(); }
};

// ── Room ID ──
struct RoomId {
    std::string localpart;
    std::string server_name;

    std::string full() const { return "!" + localpart + ":" + server_name; }
    static RoomId parse(const std::string& id);
    bool isValid() const { return !localpart.empty() && !server_name.empty(); }
};

// ── Event ID ──
struct EventId {
    std::string id;
    std::string server_name;

    std::string full() const { return "$" + id + (server_name.empty() ? "" : ":" + server_name); }
    static EventId parse(const std::string& id);
    bool isValid() const { return !id.empty(); }
};

// ── Session Params ──
struct SessionParams {
    std::string userId;
    std::string accessToken;
    std::string deviceId;
    std::string homeServerUrl;
    std::string identityServerUrl;
    std::string refreshToken;
    int64_t tokenExpiryTime = 0;
    bool isTokenValid = false;
};

// ── Matrix Error ──
struct MatrixError {
    int httpCode = 0;
    std::string errorCode;       // e.g., "M_FORBIDDEN", "M_UNKNOWN_TOKEN"
    std::string errorMessage;
    std::string softLogout;      // if token expired
    int retryAfterMs = 0;
    bool isTokenError = false;
    bool isNetworkError = false;
};

// ── Power Levels ──
struct PowerLevels {
    int usersDefault = 0;
    int eventsDefault = 0;
    int stateDefault = 50;
    int ban = 50;
    int kick = 50;
    int redact = 50;
    int invite = 0;
    std::map<std::string, int> users;
    std::map<std::string, int> events;

    int getUserLevel(const std::string& userId) const;
    int getEventLevel(const std::string& eventType) const;
};

// ── Room Create Content ──
struct CreateRoomParams {
    std::string name;
    std::string topic;
    RoomPreset preset = RoomPreset::PRIVATE_CHAT;
    RoomVisibility visibility = RoomVisibility::PRIVATE;
    bool isDirect = false;
    std::vector<std::string> invite;
    std::vector<std::string> invite3pid;
    std::string roomAliasName;
    std::string roomVersion;
    std::optional<PowerLevels> powerLevels;
    std::optional<std::string> creationContent;
    std::optional<std::string> initialStates;
};

// ── Room Avatar ──
struct RoomAvatar {
    std::string url;
    std::string thumbnailUrl;
    int width = 0;
    int height = 0;
    int thumbnailWidth = 0;
    int thumbnailHeight = 0;
};

// ── Location Data ──
struct LocationData {
    double latitude = 0.0;
    double longitude = 0.0;
    double uncertainty = 0.0;
    std::string description;
    std::string geoUri;
};

// ── Poll Data ──
struct PollQuestion {
    std::string question;
    int maxSelections = 1;
    std::vector<std::string> answers;
    bool isClosed = false;
};

struct PollResponse {
    std::string eventId;
    std::vector<int> selectedAnswers;
};

// ── Room Member ──
struct RoomMember {
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    Membership membership = Membership::LEAVE;
    std::string reason;
    std::string thirdPartyInvite;
    bool isDirect = false;
    int64_t originServerTs = 0;
};

// ── Room Info ──
struct RoomInfo {
    std::string roomId;
    std::string name;
    std::string topic;
    std::string canonicalAlias;
    std::vector<std::string> aliases;
    std::string roomAvatarUrl;
    int numJoinedMembers = 0;
    int numInvitedMembers = 0;
    bool isDirect = false;
    bool isPublic = false;
    bool isEncrypted = false;
    bool isSpace = false;
    std::string joinRule;
    std::string guestAccess;
    std::string historyVisibility;
    std::string roomVersion;
    int64_t creationTs = 0;
};

// ── Media Info ──
struct MediaInfo {
    std::string mimeType;
    int width = 0;
    int height = 0;
    int64_t size = 0;
    int duration = 0;
    std::string thumbnailUrl;
    int thumbnailWidth = 0;
    int thumbnailHeight = 0;
    std::string thumbnailMimeType;
    int64_t thumbnailSize = 0;
    std::string blurhash;
    std::string waveform;
    std::string encryptionInfo;
};

// ── Thumbnail Size ──
struct ThumbnailSize {
    int width;
    int height;
    std::string method;  // "scale" or "crop"

    std::string toQueryString() const {
        return std::to_string(width) + "x" + std::to_string(height);
    }
};

// ── Public Room ──
struct PublicRoom {
    std::string roomId;
    std::string name;
    std::string topic;
    std::string canonicalAlias;
    std::string avatarUrl;
    int numJoinedMembers = 0;
    bool worldReadable = false;
    bool guestCanJoin = false;
};

// ── Third Party Protocol ──
struct ThirdPartyProtocol {
    std::string name;
    std::vector<std::string> fields;
    std::vector<std::string> locationFields;
    std::vector<std::string> userFields;
    std::string icon;
    std::vector<std::string> instances;
};

// ── Third Party User ──
struct ThirdPartyUser {
    std::string userId;
    std::string displayName;
    std::vector<std::string> fields;
};

// ── Third Party Location ──
struct ThirdPartyLocation {
    std::string alias;
    std::string protocol;
    std::vector<std::string> fields;
};

// ── Widget ──
struct Widget {
    std::string widgetId;
    std::string widgetType;  // e.g., "m.custom", "m.jitsi"
    std::string name;
    std::string url;
    bool waitForIframeLoad = false;
    std::string creatorUserId;
    bool isActive = true;
    std::optional<std::string> avatarUrl;
};

// ── Space Hierarchy ──
struct SpaceChildInfo {
    std::string roomId;
    std::string name;
    std::string topic;
    std::string avatarUrl;
    bool isSpace = false;
    bool isSuggested = false;
    bool isKnown = false;
    std::string order;
    int numJoinedMembers = 0;
    std::string roomType;
    std::vector<std::string> childrenStateEvents;
};

// ── Authentication Data ──
struct AuthenticationData {
    std::string userId;
    std::string homeServer;
    std::string identityServer;
    std::string accessToken;
    std::string refreshToken;
    std::string deviceId;
    bool isOidc = false;
    json toJson() const;
    static AuthenticationData fromJson(const json& j);
};

// ── Well-Known ──
struct WellKnownConfig {
    std::string homeServerBaseUrl;
    std::string identityServerBaseUrl;
    bool isValid = false;
};

// ── Login Flow ──
struct LoginFlow {
    std::string type;  // "m.login.password", "m.login.token", "m.login.sso", etc.
    std::vector<std::string> identityProviders;
    bool isSso = false;
};

// ── Registration Flow ──
struct RegistrationFlow {
    std::string type;
    std::vector<std::string> stages;
};

// ── User Profile ──
struct UserProfile {
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    int64_t lastUpdated = 0;
};

// ── Device Info ──
struct DeviceInfo {
    std::string deviceId;
    std::string displayName;
    std::string lastSeenIp;
    int64_t lastSeenTs = 0;
    DeviceVerification verificationStatus = DeviceVerification::UNVERIFIED;
    bool isCurrentDevice = false;
    std::string lastSeenUserAgent;
};

// ── Crypto Device ──
struct CryptoDeviceInfo {
    std::string userId;
    std::string deviceId;
    std::vector<std::string> algorithms;
    std::map<std::string, std::string> keys;
    std::map<std::string, std::string> signatures;
    bool isUnsigned = false;
    bool isVerified = false;
    bool isBlocked = false;
};

// ── Identity Server Info ──
struct IdentityServerInfo {
    std::string baseUrl;
    bool isValid = false;
};

// ── OpenId Token ──
struct OpenIdToken {
    std::string accessToken;
    std::string tokenType;
    std::string matrixServerName;
    int64_t expiresIn = 0;
};

// ── Search Result ──
struct SearchResult {
    std::string eventId;
    std::string roomId;
    std::string senderId;
    std::string senderName;
    std::string body;
    int64_t originServerTs = 0;
    double rank = 0.0;
    std::string context;
};

// ── Room Preview ──
struct RoomPreview {
    std::string roomId;
    std::string roomName;
    std::string roomTopic;
    std::string roomAvatarUrl;
    std::string canonicalAlias;
    int numJoinedMembers = 0;
    bool worldReadable = false;
    bool guestCanJoin = false;
    bool isDirect = false;
    std::string roomType;
};

// ── Invite Data ──
struct InviteData {
    std::string roomId;
    std::string roomName;
    std::string roomAvatarUrl;
    std::string inviterId;
    std::string inviterName;
    bool isDirect = false;
    bool isSpace = false;
    std::string reason;
    int64_t timestamp = 0;
};

// ── Sync Filter ──
struct SyncFilter {
    std::vector<std::string> notTypes;
    std::vector<std::string> notSenders;
    std::vector<std::string> types;
    std::vector<std::string> senders;
    int limit = 20;
    bool lazyLoadMembers = true;
    bool includeRedundantMembers = false;
    bool unreadThreadNotifications = false;
    json toJson() const;
};

// ── Event Annotations (Reactions) ──
struct EventAnnotation {
    std::string eventId;
    std::string key;           // the emoji/string
    std::string senderId;
    int count = 0;
    bool isLocalEcho = false;
};

// ── Relation Info ──
struct RelationInfo {
    RelationType type = RelationType::REFERENCE;
    std::string eventId;       // relates_to event_id
    std::string key;           // for annotations
    bool isFallingBack = false;
};

// ── Thread Info ──
struct ThreadInfo {
    std::string threadId;      // root event id
    std::string latestEventId;
    int numberOfReplies = 0;
    std::string rootEventSenderId;
};

// ── Beacon Info ──
struct BeaconInfo {
    std::string eventId;
    std::string description;
    double latitude = 0.0;
    double longitude = 0.0;
    double uncertainty = 0.0;
    int64_t timeout = 0;
    bool isLive = true;
    int64_t timestamp = 0;
};

// ── Pagination ──
struct PaginationOptions {
    int limit = 20;
    std::string fromToken;
    std::string toToken;
    std::string direction;  // "b" for backward, "f" for forward
};

// ── Helpers ──
EventType eventTypeFromString(const std::string& s);
std::string eventTypeToString(EventType t);
MessageType messageTypeFromString(const std::string& s);
std::string messageTypeToString(MessageType t);
Membership membershipFromString(const std::string& s);
std::string membershipToString(Membership m);

} // namespace matrix_sdk

#endif
