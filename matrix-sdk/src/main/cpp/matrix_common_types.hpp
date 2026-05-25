#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann::json;

// ── Core Matrix types ──

struct MatrixUserId { std::string localpart; std::string serverName; std::string full; };

struct RoomId { std::string id; std::string serverName; };
struct EventId { std::string id; };
struct RoomAlias { std::string alias; std::string serverName; };
struct MxcUri { std::string serverName; std::string mediaId; std::string full; };

struct PaginationToken { std::string token; std::string direction; };

// ── Parsing helpers ──

class MatrixIdParser {
public:
    static MatrixUserId parseUserId(const std::string& id) {
        MatrixUserId result; result.full = id;
        if (id.size() >= 4 && id[0] == '@') {
            size_t colon = id.find(':');
            if (colon != std::string::npos) {
                result.localpart = id.substr(1, colon - 1);
                result.serverName = id.substr(colon + 1);
            }
        }
        return result;
    }
    static RoomId parseRoomId(const std::string& id) {
        RoomId result; result.id = id;
        if (id.size() >= 4 && id[0] == '!') {
            size_t colon = id.find(':');
            if (colon != std::string::npos) result.serverName = id.substr(colon + 1);
        }
        return result;
    }
    static EventId parseEventId(const std::string& id) { return {id}; }
    static MxcUri parseMxcUri(const std::string& uri) {
        MxcUri result; result.full = uri;
        if (uri.find("mxc://") == 0) {
            std::string rest = uri.substr(6);
            size_t slash = rest.find('/');
            if (slash != std::string::npos) {
                result.serverName = rest.substr(0, slash);
                result.mediaId = rest.substr(slash + 1);
            }
        }
        return result;
    }
    static RoomAlias parseAlias(const std::string& alias) {
        RoomAlias result; result.alias = alias;
        if (alias.size() >= 4 && alias[0] == '#') {
            size_t colon = alias.find(':');
            if (colon != std::string::npos) result.serverName = alias.substr(colon + 1);
        }
        return result;
    }
};

// ── Power levels ──

struct PowerLevels {
    int users_default = 0;
    int events_default = 0;
    int state_default = 50;
    int ban = 50;
    int kick = 50;
    int redact = 50;
    int invite = 50;
    std::map<std::string, int> users;
    std::map<std::string, int> events;
    int notifications_room = 50;

    json toJson() const {
        json j;
        j["users_default"] = users_default;
        j["events_default"] = events_default;
        j["state_default"] = state_default;
        j["ban"] = ban;
        j["kick"] = kick;
        j["redact"] = redact;
        j["invite"] = invite;
        j["users"] = users;
        j["events"] = events;
        if (notifications_room != 50) j["notifications"]["room"] = notifications_room;
        return j;
    }
    static PowerLevels fromJson(const json& j) {
        PowerLevels p;
        p.users_default = j.value("users_default", 0);
        p.events_default = j.value("events_default", 0);
        p.state_default = j.value("state_default", 50);
        p.ban = j.value("ban", 50);
        p.kick = j.value("kick", 50);
        p.redact = j.value("redact", 50);
        p.invite = j.value("invite", 50);
        if (j.contains("users") && j["users"].is_object())
            for (auto& [k, v] : j["users"].items()) p.users[k] = v;
        if (j.contains("events") && j["events"].is_object())
            for (auto& [k, v] : j["events"].items()) p.events[k] = v;
        if (j.contains("notifications") && j["notifications"].contains("room"))
            p.notifications_room = j["notifications"]["room"];
        return p;
    }
    int getUserLevel(const std::string& userId) const {
        auto it = users.find(userId);
        return it != users.end() ? it->second : users_default;
    }
    int getEventLevel(const std::string& eventType) const {
        auto it = events.find(eventType);
        return it != events.end() ? it->second : events_default;
    }
    bool canSendEvent(const std::string& userId, const std::string& eventType) const {
        return getUserLevel(userId) >= getEventLevel(eventType);
    }
    bool canSendStateEvent(const std::string& userId, const std::string& eventType) const {
        return getUserLevel(userId) >= (eventType == "m.room.name" || eventType == "m.room.topic"
            ? state_default : getEventLevel(eventType));
    }
    bool canBan(const std::string& userId) const { return getUserLevel(userId) >= ban; }
    bool canKick(const std::string& userId) const { return getUserLevel(userId) >= kick; }
    bool canRedact(const std::string& userId) const { return getUserLevel(userId) >= redact; }
    bool canInvite(const std::string& userId) const { return getUserLevel(userId) >= invite; }
};

// ── Unsigned event data ──

struct UnsignedData {
    uint64_t age = 0;
    std::string redactedBecause;
    std::string prevContent;
    std::string transactionId;
};

// ── Event base ──

struct MatrixEvent {
    std::string eventId; std::string roomId; std::string sender;
    std::string type; json content; std::string stateKey;
    uint64_t originServerTs = 0; UnsignedData unsignedData;
    std::string redacts;

    bool isState() const { return !stateKey.empty(); }
    bool isMessage() const { return type == "m.room.message"; }
    bool isEncrypted() const { return type == "m.room.encrypted"; }
    bool isRedaction() const { return type == "m.room.redaction"; }

    json toJson() const {
        json j;
        j["event_id"] = eventId; j["room_id"] = roomId; j["sender"] = sender;
        j["type"] = type; j["content"] = content;
        if (!stateKey.empty()) j["state_key"] = stateKey;
        j["origin_server_ts"] = originServerTs;
        if (!redacts.empty()) j["redacts"] = redacts;
        return j;
    }
    static MatrixEvent fromJson(const json& j) {
        MatrixEvent e;
        e.eventId = j.value("event_id", ""); e.roomId = j.value("room_id", "");
        e.sender = j.value("sender", ""); e.type = j.value("type", "");
        e.content = j.value("content", json::object());
        e.stateKey = j.value("state_key", "");
        e.originServerTs = j.value("origin_server_ts", 0ULL);
        e.redacts = j.value("redacts", "");
        if (j.contains("unsigned")) {
            auto& u = j["unsigned"];
            e.unsignedData.age = u.value("age", 0ULL);
            e.unsignedData.redactedBecause = u.value("redacted_because", "");
        }
        return e;
    }
};

// ── Sync token management ──

class SyncToken {
public:
    SyncToken() = default;
    explicit SyncToken(const std::string& token) : m_token(token) {}

    const std::string& get() const { return m_token; }
    void set(const std::string& token) { m_token = token; m_updated = true; }
    bool isUpdated() const { return m_updated; }
    void markConsumed() { m_updated = false; }
    bool isEmpty() const { return m_token.empty(); }
    void clear() { m_token.clear(); m_updated = false; }

private:
    std::string m_token;
    bool m_updated = false;
};

// ── Transaction ID generator ──

class TransactionIdGenerator {
public:
    static std::string generate() {
        static std::atomic<uint64_t> counter{0};
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return "m" + std::to_string(now) + "." + std::to_string(counter.fetch_add(1));
    }
};

// ── Filter JSON builder ──

class FilterBuilder {
public:
    FilterBuilder& setLimit(int limit) { m_filter["room"]["timeline"]["limit"] = limit; return *this; }
    FilterBuilder& includePresence(bool inc) { m_filter["presence"]["types"] = inc ? json::array({"*"}) : json::array(); return *this; }
    FilterBuilder& includeAccountData(bool inc) { m_filter["account_data"]["types"] = inc ? json::array({"*"}) : json::array(); return *this; }
    FilterBuilder& addRoom(const std::string& roomId) { m_filter["room"]["rooms"].push_back(roomId); return *this; }
    FilterBuilder& excludeRoom(const std::string& roomId) { m_filter["room"]["not_rooms"].push_back(roomId); return *this; }
    FilterBuilder& includeLazyLoadMembers(bool inc) {
        m_filter["room"]["state"]["lazy_load_members"] = inc; return *this;
    }
    FilterBuilder& addEventType(const std::string& type) {
        m_filter["room"]["timeline"]["types"].push_back(type); return *this;
    }
    FilterBuilder& unreadThreadNotifications(bool v) {
        m_filter["room"]["timeline"]["unread_thread_notifications"] = v; return *this;
    }
    json build() const { return m_filter; }
private:
    json m_filter = {{"presence", {{"types", json::array({"*"})}}},
                     {"account_data", {{"types", json::array({"*"})}}},
                     {"room", {{"timeline", {{"limit", 50}}},
                               {"state", {{"lazy_load_members", true}}},
                               {"ephemeral", {{"types", json::array({"m.typing", "m.receipt"})}}}}}};
};

} // namespace matrix_sdk
