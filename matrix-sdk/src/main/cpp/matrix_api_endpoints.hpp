#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace matrix_sdk::api {

using json = nlohmann::json;

// ==== Matrix API endpoint constants ====
namespace endpoints {
    constexpr auto LOGIN = "/_matrix/client/v3/login";
    constexpr auto LOGOUT = "/_matrix/client/v3/logout";
    constexpr auto REGISTER = "/_matrix/client/v3/register";
    constexpr auto REFRESH = "/_matrix/client/v3/refresh";
    constexpr auto SYNC = "/_matrix/client/v3/sync";
    constexpr auto WHOAMI = "/_matrix/client/v3/account/whoami";
    constexpr auto PROFILE = "/_matrix/client/v3/profile/{userId}";
    constexpr auto DISPLAYNAME = "/_matrix/client/v3/profile/{userId}/displayname";
    constexpr auto AVATAR_URL = "/_matrix/client/v3/profile/{userId}/avatar_url";
    constexpr auto CREATE_ROOM = "/_matrix/client/v3/createRoom";
    constexpr auto JOIN_ROOM = "/_matrix/client/v3/join/{roomId}";
    constexpr auto LEAVE_ROOM = "/_matrix/client/v3/rooms/{roomId}/leave";
    constexpr auto INVITE = "/_matrix/client/v3/rooms/{roomId}/invite";
    constexpr auto KICK = "/_matrix/client/v3/rooms/{roomId}/kick";
    constexpr auto BAN = "/_matrix/client/v3/rooms/{roomId}/ban";
    constexpr auto UNBAN = "/_matrix/client/v3/rooms/{roomId}/unban";
    constexpr auto SEND_EVENT = "/_matrix/client/v3/rooms/{roomId}/send/{eventType}/{txnId}";
    constexpr auto SEND_MESSAGE = "/_matrix/client/v3/rooms/{roomId}/send/m.room.message/{txnId}";
    constexpr auto REDACT = "/_matrix/client/v3/rooms/{roomId}/redact/{eventId}/{txnId}";
    constexpr auto ROOM_MESSAGES = "/_matrix/client/v3/rooms/{roomId}/messages";
    constexpr auto ROOM_EVENT = "/_matrix/client/v3/rooms/{roomId}/event/{eventId}";
    constexpr auto ROOM_STATE = "/_matrix/client/v3/rooms/{roomId}/state";
    constexpr auto ROOM_MEMBERS = "/_matrix/client/v3/rooms/{roomId}/members";
    constexpr auto ROOM_JOINED_MEMBERS = "/_matrix/client/v3/rooms/{roomId}/joined_members";
    constexpr auto TYPING = "/_matrix/client/v3/rooms/{roomId}/typing/{userId}";
    constexpr auto RECEIPT = "/_matrix/client/v3/rooms/{roomId}/receipt/m.read/{eventId}";
    constexpr auto READ_MARKERS = "/_matrix/client/v3/rooms/{roomId}/read_markers";
    constexpr auto UPLOAD = "/_matrix/client/v3/upload";
    constexpr auto DOWNLOAD = "/_matrix/client/v3/download/{serverName}/{mediaId}";
    constexpr auto THUMBNAIL = "/_matrix/client/v3/thumbnail/{serverName}/{mediaId}";
    constexpr auto CONFIG = "/_matrix/client/v3/media/config";
    constexpr auto PREVIEW_URL = "/_matrix/client/v3/media/preview_url";
    constexpr auto SEARCH = "/_matrix/client/v3/search";
    constexpr auto USER_DIRECTORY = "/_matrix/client/v3/user_directory/search";
    constexpr auto PUBLIC_ROOMS = "/_matrix/client/v3/publicRooms";
    constexpr auto JOINED_ROOMS = "/_matrix/client/v3/joined_rooms";
    constexpr auto DEVICES = "/_matrix/client/v3/devices";
    constexpr auto DELETE_DEVICE = "/_matrix/client/v3/devices/{deviceId}";
    constexpr auto DEVICE_KEYS = "/_matrix/client/v3/keys/upload";
    constexpr auto CLAIM_KEYS = "/_matrix/client/v3/keys/claim";
    constexpr auto QUERY_KEYS = "/_matrix/client/v3/keys/query";
    constexpr auto KEY_CHANGES = "/_matrix/client/v3/keys/changes";
    constexpr auto SEND_TO_DEVICE = "/_matrix/client/v3/sendToDevice/{eventType}/{txnId}";
    constexpr auto PUSH_RULES = "/_matrix/client/v3/pushrules/";
    constexpr auto PUSH_RULE = "/_matrix/client/v3/pushrules/{scope}/{kind}/{ruleId}";
    constexpr auto PUSH_ACTIONS = "/_matrix/client/v3/pushrules/{scope}/{kind}/{ruleId}/actions";
    constexpr auto PUSH_ENABLED = "/_matrix/client/v3/pushrules/{scope}/{kind}/{ruleId}/enabled";
    constexpr auto PUSHERS = "/_matrix/client/v3/pushers";
    constexpr auto NOTIFICATIONS = "/_matrix/client/v3/notifications";
    constexpr auto PRESENCE = "/_matrix/client/v3/presence/{userId}/status";
    constexpr auto ACCOUNT_3PID = "/_matrix/client/v3/account/3pid";
    constexpr auto ACCOUNT_DEACTIVATE = "/_matrix/client/v3/account/deactivate";
    constexpr auto CHANGE_PASSWORD = "/_matrix/client/v3/account/password";
    constexpr auto TERMS = "/_matrix/client/v3/terms";
    constexpr auto CAPABILITIES = "/_matrix/client/v3/capabilities";
    constexpr auto ROOM_UPGRADE = "/_matrix/client/v3/rooms/{roomId}/upgrade";
    constexpr auto REPORT = "/_matrix/client/v3/rooms/{roomId}/report/{eventId}";
    constexpr auto CONTEXT = "/_matrix/client/v3/rooms/{roomId}/context/{eventId}";
    constexpr auto RELATIONS = "/_matrix/client/v3/rooms/{roomId}/relations/{eventId}";
    constexpr auto RELATIONS_TYPE = "/_matrix/client/v3/rooms/{roomId}/relations/{eventId}/{relType}";
    constexpr auto THREADS = "/_matrix/client/v3/rooms/{roomId}/threads";
    constexpr auto ADMIN = "/_matrix/client/v3/admin";
    constexpr auto WELL_KNOWN = "/.well-known/matrix/client";
    constexpr auto SERVER_WELL_KNOWN = "/.well-known/matrix/server";
    constexpr auto THIRD_PARTY_PROTOCOLS = "/_matrix/client/v3/thirdparty/protocols";
    constexpr auto THIRD_PARTY_LOCATION = "/_matrix/client/v3/thirdparty/location/{protocol}";
    constexpr auto THIRD_PARTY_USER = "/_matrix/client/v3/thirdparty/user/{protocol}";
    constexpr auto OPENID = "/_matrix/client/v3/user/{userId}/openid/request_token";
    constexpr auto HIERARCHY = "/_matrix/client/v1/rooms/{roomId}/hierarchy";
    constexpr auto SPACE_SUMMARY = "/_matrix/client/v1/rooms/{roomId}/summary";
}

// API version info
struct ApiVersion { std::string version; bool stable = false; };

class ApiVersions {
public:
    static std::vector<ApiVersion> getSupported() {
        return {{"r0.0.1"}, {"r0.1.0"}, {"r0.2.0"}, {"r0.3.0"}, {"r0.4.0"},
                {"r0.5.0"}, {"r0.6.0"}, {"v1.1"}, {"v1.2"}, {"v1.3"}, {"v1.4"},
                {"v1.5"}, {"v1.6"}};
    }
    static std::string latest() { return "v1.6"; }
    static std::string latestStable() { return "v1.6"; }
};

// HTTP methods
enum class HttpMethod { GET, POST, PUT, DELETE, PATCH };

struct ApiRequest {
    HttpMethod method = HttpMethod::GET;
    std::string path;
    json body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queryParams;
    int timeoutMs = 30000;
};

struct ApiResponse {
    int statusCode = 0;
    json body;
    std::map<std::string, std::string> headers;
    bool success = false;
    std::string error;
    int64_t latencyMs = 0;
};

// Rate limit tracking
class RateLimiter {
public:
    RateLimiter() = default;
    bool canRequest(const std::string& endpoint) {
        auto now = std::chrono::steady_clock::now();
        auto& window = m_windows[endpoint];
        if (now - window.start > std::chrono::seconds(1)) {
            window.start = now; window.count = 0;
        }
        if (window.count >= m_maxPerSecond) return false;
        window.count++; return true;
    }
    void updateFromResponse(const std::string& endpoint, int retryAfterMs) {
        m_retryAfter[endpoint] = std::chrono::steady_clock::now() +
            std::chrono::milliseconds(retryAfterMs);
    }
    void setMaxPerSecond(int max) { m_maxPerSecond = max; }
private:
    struct Window { std::chrono::steady_clock::time_point start; int count = 0; };
    std::map<std::string, Window> m_windows;
    std::map<std::string, std::chrono::steady_clock::time_point> m_retryAfter;
    int m_maxPerSecond = 100;
};

// Request builder
class RequestBuilder {
public:
    RequestBuilder() = default;
    RequestBuilder& method(HttpMethod m) { m_request.method = m; return *this; }
    RequestBuilder& path(const std::string& p) { m_request.path = p; return *this; }
    RequestBuilder& body(const json& b) { m_request.body = b; return *this; }
    RequestBuilder& header(const std::string& k, const std::string& v) {
        m_request.headers[k] = v; return *this;
    }
    RequestBuilder& authToken(const std::string& token) {
        return header("Authorization", "Bearer " + token);
    }
    RequestBuilder& queryParam(const std::string& k, const std::string& v) {
        m_request.queryParams[k] = v; return *this;
    }
    RequestBuilder& timeout(int ms) { m_request.timeoutMs = ms; return *this; }
    ApiRequest build() const { return m_request; }
private:
    ApiRequest m_request;
};

// HTTP status codes
namespace HttpStatus {
    constexpr int OK = 200;
    constexpr int CREATED = 201;
    constexpr int ACCEPTED = 202;
    constexpr int NO_CONTENT = 204;
    constexpr int BAD_REQUEST = 400;
    constexpr int UNAUTHORIZED = 401;
    constexpr int FORBIDDEN = 403;
    constexpr int NOT_FOUND = 404;
    constexpr int METHOD_NOT_ALLOWED = 405;
    constexpr int CONFLICT = 409;
    constexpr int GONE = 410;
    constexpr int TOO_LARGE = 413;
    constexpr int UNSUPPORTED_MEDIA = 415;
    constexpr int TOO_MANY_REQUESTS = 429;
    constexpr int SERVER_ERROR = 500;
    constexpr int BAD_GATEWAY = 502;
    constexpr int SERVICE_UNAVAILABLE = 503;
    constexpr int GATEWAY_TIMEOUT = 504;

    static bool isSuccess(int code) { return code >= 200 && code < 300; }
    static bool isClientError(int code) { return code >= 400 && code < 500; }
    static bool isServerError(int code) { return code >= 500 && code < 600; }
    static bool isRetryable(int code) {
        return code == 429 || code >= 500;
    }
    static std::string toString(int code) {
        switch (code) {
            case 200: return "OK";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 429: return "Rate Limited";
            case 500: return "Internal Server Error";
            default: return "HTTP " + std::to_string(code);
        }
    }
}

// Matrix error codes
namespace ErrorCode {
    constexpr auto FORBIDDEN = "M_FORBIDDEN";
    constexpr auto UNKNOWN_TOKEN = "M_UNKNOWN_TOKEN";
    constexpr auto MISSING_TOKEN = "M_MISSING_TOKEN";
    constexpr auto BAD_JSON = "M_BAD_JSON";
    constexpr auto NOT_JSON = "M_NOT_JSON";
    constexpr auto NOT_FOUND = "M_NOT_FOUND";
    constexpr auto LIMIT_EXCEEDED = "M_LIMIT_EXCEEDED";
    constexpr auto UNKNOWN = "M_UNKNOWN";
    constexpr auto UNRECOGNIZED = "M_UNRECOGNIZED";
    constexpr auto UNAUTHORIZED = "M_UNAUTHORIZED";
    constexpr auto USER_IN_USE = "M_USER_IN_USE";
    constexpr auto INVALID_USERNAME = "M_INVALID_USERNAME";
    constexpr auto ROOM_IN_USE = "M_ROOM_IN_USE";
    constexpr auto INVALID_ROOM_STATE = "M_INVALID_ROOM_STATE";
    constexpr auto THREEPID_IN_USE = "M_THREEPID_IN_USE";
    constexpr auto THREEPID_NOT_FOUND = "M_THREEPID_NOT_FOUND";
    constexpr auto SERVER_NOT_TRUSTED = "M_SERVER_NOT_TRUSTED";
    constexpr auto TOO_LARGE = "M_TOO_LARGE";
    constexpr auto EXCLUSIVE = "M_EXCLUSIVE";
    constexpr auto GUEST_ACCESS_FORBIDDEN = "M_GUEST_ACCESS_FORBIDDEN";
    constexpr auto WEAK_PASSWORD = "M_WEAK_PASSWORD";
    constexpr auto PASSWORD_NO_DIRECTORY = "M_PASSWORD_NO_DIRECTORY";
    constexpr auto PASSWORD_IN_DICTIONARY = "M_PASSWORD_IN_DICTIONARY";
    constexpr auto CAPTCHA_NEEDED = "M_CAPTCHA_NEEDED";
    constexpr auto CAPTCHA_INVALID = "M_CAPTCHA_INVALID";
    constexpr auto TERMS_NOT_SIGNED = "M_TERMS_NOT_SIGNED";
}

} // namespace matrix_sdk::api


// ── API Request Dispatcher ──

class ApiDispatcher {
public:
    ApiDispatcher(const std::string& homeserver, const std::string& token)
        : m_homeserver(homeserver), m_token(token) {}

    ApiResponse dispatch(const ApiRequest& req) {
        ApiResponse resp;
        auto start = std::chrono::steady_clock::now();
        // Build full URL
        std::string url = m_homeserver + req.path;
        if (!req.queryParams.empty()) {
            url += "?";
            bool first = true;
            for (auto& [k, v] : req.queryParams) {
                if (!first) url += "&"; url += k + "=" + v; first = false;
            }
        }
        (void)req; (void)url; // unused in stub
        auto end = std::chrono::steady_clock::now();
        resp.latencyMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return resp;
    }

    void setRetryPolicy(int maxRetries, int baseDelayMs) {
        m_maxRetries = maxRetries; m_baseDelayMs = baseDelayMs;
    }

    ApiResponse dispatchWithRetry(const ApiRequest& req) {
        ApiResponse resp;
        for (int attempt = 0; attempt <= m_maxRetries; attempt++) {
            resp = dispatch(req);
            if (resp.success) break;
            if (HttpStatus::isRetryable(resp.statusCode)) {
                int delay = m_baseDelayMs * (1 << attempt);
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            } else break;
        }
        return resp;
    }

private:
    std::string m_homeserver; std::string m_token;
    int m_maxRetries = 3; int m_baseDelayMs = 1000;
};

// ── Content-Type helpers ──

namespace ContentType {
    constexpr auto JSON = "application/json";
    constexpr auto OCTET_STREAM = "application/octet-stream";
    constexpr auto FORM_URLENCODED = "application/x-www-form-urlencoded";
    constexpr auto MULTIPART_FORM = "multipart/form-data";
    constexpr auto TEXT_PLAIN = "text/plain";
    constexpr auto TEXT_HTML = "text/html";

    static std::string fromExtension(const std::string& ext) {
        if (ext == ".json") return JSON;
        if (ext == ".html" || ext == ".htm") return TEXT_HTML;
        if (ext == ".txt") return TEXT_PLAIN;
        return OCTET_STREAM;
    }
}

// ── Event type constants ──

namespace EventTypes {
    // Room events
    constexpr auto ROOM_MESSAGE = "m.room.message";
    constexpr auto ROOM_NAME = "m.room.name";
    constexpr auto ROOM_TOPIC = "m.room.topic";
    constexpr auto ROOM_AVATAR = "m.room.avatar";
    constexpr auto ROOM_MEMBER = "m.room.member";
    constexpr auto ROOM_CREATE = "m.room.create";
    constexpr auto ROOM_JOIN_RULES = "m.room.join_rules";
    constexpr auto ROOM_GUEST_ACCESS = "m.room.guest_access";
    constexpr auto ROOM_POWER_LEVELS = "m.room.power_levels";
    constexpr auto ROOM_TOMBSTONE = "m.room.tombstone";
    constexpr auto ROOM_CANONICAL_ALIAS = "m.room.canonical_alias";
    constexpr auto ROOM_HISTORY_VISIBILITY = "m.room.history_visibility";
    constexpr auto ROOM_ENCRYPTION = "m.room.encryption";
    constexpr auto ROOM_SERVER_ACL = "m.room.server_acl";
    constexpr auto ROOM_PINNED_EVENTS = "m.room.pinned_events";

    // Message types
    constexpr auto MSG_TEXT = "m.text";
    constexpr auto MSG_NOTICE = "m.notice";
    constexpr auto MSG_EMOTE = "m.emote";
    constexpr auto MSG_IMAGE = "m.image";
    constexpr auto MSG_AUDIO = "m.audio";
    constexpr auto MSG_VIDEO = "m.video";
    constexpr auto MSG_FILE = "m.file";
    constexpr auto MSG_LOCATION = "m.location";

    // Encryption
    constexpr auto ROOM_ENCRYPTED = "m.room.encrypted";
    constexpr auto ROOM_KEY = "m.room_key";
    constexpr auto ROOM_KEY_REQUEST = "m.room_key_request";
    constexpr auto FORWARDED_ROOM_KEY = "m.forwarded_room_key";

    // Device management
    constexpr auto DEVICE_LIST = "m.device_list_update";

    // Presence & typing
    constexpr auto PRESENCE = "m.presence";
    constexpr auto TYPING = "m.typing";
    constexpr auto RECEIPT = "m.receipt";
    constexpr auto FULLY_READ = "m.fully_read";

    // Reactions & relations
    constexpr auto REACTION = "m.reaction";
    constexpr auto REDACTION = "m.room.redaction";

    // Calls
    constexpr auto CALL_INVITE = "m.call.invite";
    constexpr auto CALL_CANDIDATES = "m.call.candidates";
    constexpr auto CALL_ANSWER = "m.call.answer";
    constexpr auto CALL_HANGUP = "m.call.hangup";
    constexpr auto CALL_NEGOTIATE = "m.call.negotiate";
    constexpr auto CALL_REJECT = "m.call.reject";
    constexpr auto CALL_SELECT_ANSWER = "m.call.select_answer";

    // Spaces
    constexpr auto SPACE_CHILD = "m.space.child";
    constexpr auto SPACE_PARENT = "m.space.parent";

    // Other
    constexpr auto STICKER = "m.sticker";
    constexpr auto TAG = "m.tag";
    constexpr auto DIRECT = "m.direct";
    constexpr auto IGNORED_USER_LIST = "m.ignored_user_list";
    constexpr auto PUSH_RULES = "m.push_rules";
    constexpr auto PUSHER = "m.pusher";
    constexpr auto WIDGET = "m.widget";
    constexpr auto ELEMENT_FEATURE = "im.vector.modular.widgets";

    static bool isMessageType(const std::string& type) {
        return type == ROOM_MESSAGE || type == STICKER;
    }
    static bool isStateType(const std::string& type) {
        return type == ROOM_NAME || type == ROOM_TOPIC || type == ROOM_AVATAR ||
               type == ROOM_MEMBER || type == ROOM_CREATE || type == ROOM_JOIN_RULES ||
               type == ROOM_GUEST_ACCESS || type == ROOM_POWER_LEVELS ||
               type == ROOM_TOMBSTONE || type == ROOM_CANONICAL_ALIAS ||
               type == ROOM_HISTORY_VISIBILITY || type == ROOM_ENCRYPTION ||
               type == ROOM_SERVER_ACL || type == ROOM_PINNED_EVENTS;
    }
}

// ── Membership constants ──

namespace Membership {
    constexpr auto JOIN = "join";
    constexpr auto LEAVE = "leave";
    constexpr auto INVITE = "invite";
    constexpr auto BAN = "ban";
    constexpr auto KNOCK = "knock";

    static bool isValid(const std::string& m) {
        return m == JOIN || m == LEAVE || m == INVITE || m == BAN || m == KNOCK;
    }
    static std::string displayName(const std::string& m) {
        if (m == JOIN) return "Joined";
        if (m == LEAVE) return "Left";
        if (m == INVITE) return "Invited";
        if (m == BAN) return "Banned";
        if (m == KNOCK) return "Knocked";
        return m;
    }
}

// ── Presence constants ──

namespace Presence {
    constexpr auto ONLINE = "online";
    constexpr auto OFFLINE = "offline";
    constexpr auto UNAVAILABLE = "unavailable";
    constexpr auto BUSY = "busy";

    static std::string displayName(const std::string& p) {
        if (p == ONLINE) return "Online";
        if (p == OFFLINE) return "Offline";
        if (p == UNAVAILABLE) return "Away";
        if (p == BUSY) return "Busy";
        return p;
    }
}

// ── Message formatting helpers ──

class MessageFormatter {
public:
    static std::string plainText(const json& content) {
        return content.value("body", "");
    }

    static std::string formattedText(const json& content) {
        if (content.contains("formatted_body")) {
            return content["formatted_body"];
        }
        return content.value("body", "");
    }

    static std::string formatString(const std::string& s) {
        return "message";
    }
};

// ── URL builders ──

class UrlBuilder {
public:
    UrlBuilder(const std::string& homeserver) : m_homeserver(homeserver) {}

    std::string login() { return m_homeserver + api::endpoints::LOGIN; }
    std::string registerUser() { return m_homeserver + api::endpoints::REGISTER; }
    std::string logout() { return m_homeserver + api::endpoints::LOGOUT; }
    std::string sync() { return m_homeserver + api::endpoints::SYNC; }
    std::string profile(const std::string& uid) {
        return m_homeserver + "/_matrix/client/v3/profile/" + uid;
    }
    std::string rooms(const std::string& roomId) {
        return m_homeserver + "/_matrix/client/v3/rooms/" + roomId;
    }
    std::string join(const std::string& roomIdOrAlias) {
        return m_homeserver + "/_matrix/client/v3/join/" + roomIdOrAlias;
    }
    std::string messages(const std::string& roomId, const std::string& from = "",
                         const std::string& dir = "b", int limit = 20) {
        std::ostringstream os;
        os << m_homeserver << "/_matrix/client/v3/rooms/" << roomId
           << "/messages?dir=" << dir << "&limit=" << limit;
        if (!from.empty()) os << "&from=" << from;
        return os.str();
    }
    std::string upload() { return m_homeserver + api::endpoints::UPLOAD; }
    std::string download(const std::string& mxcUri) {
        // mxc://serverName/mediaId
        size_t pos = mxcUri.find("mxc://");
        if (pos == 0) {
            std::string rest = mxcUri.substr(6);
            size_t slash = rest.find('/');
            if (slash != std::string::npos) {
                return m_homeserver + "/_matrix/client/v3/download/"
                       + rest.substr(0, slash) + "/" + rest.substr(slash + 1);
            }
        }
        return m_homeserver + "/_matrix/client/v3/download/unknown/file";
    }

private:
    std::string m_homeserver;
};

// ── ID validators ──

class IdValidator {
public:
    static bool isValidUserId(const std::string& id) {
        return id.size() >= 4 && id[0] == '@' && id.find(':') != std::string::npos;
    }
    static bool isValidRoomId(const std::string& id) {
        return id.size() >= 4 && id[0] == '!' && id.find(':') != std::string::npos;
    }
    static bool isValidEventId(const std::string& id) {
        return id.size() >= 4 && id[0] == '$' && id.find(':') != std::string::npos;
    }
    static bool isValidRoomAlias(const std::string& alias) {
        return alias.size() >= 4 && alias[0] == '#' && alias.find(':') != std::string::npos;
    }
    static bool isValidMxcUri(const std::string& uri) {
        return uri.find("mxc://") == 0;
    }
    static std::string extractServerName(const std::string& id) {
        size_t colon = id.rfind(':');
        return colon != std::string::npos ? id.substr(colon + 1) : "";
    }
    static std::string extractLocalpart(const std::string& userId) {
        if (!isValidUserId(userId)) return "";
        size_t start = 1; // after @
        size_t colon = userId.find(':', start);
        return colon != std::string::npos ? userId.substr(start, colon - start) : "";
    }
};

} // namespace matrix_sdk::api
