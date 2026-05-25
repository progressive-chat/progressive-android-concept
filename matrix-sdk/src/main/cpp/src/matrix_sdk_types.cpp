#include "matrix_sdk/matrix_sdk_types.hpp"
#include <algorithm>
#include <sstream>
#include <cctype>

namespace matrix_sdk {

// ── UserId ──
UserId UserId::parse(const std::string& id) {
    UserId uid;
    if (id.empty() || id[0] != '@') return uid;

    auto colonPos = id.find(':');
    if (colonPos == std::string::npos) return uid;

    uid.localpart = id.substr(1, colonPos - 1);
    uid.server_name = id.substr(colonPos + 1);

    // Validate localpart
    if (uid.localpart.empty()) { uid.localpart.clear(); uid.server_name.clear(); return uid; }
    // Validate server_name
    if (uid.server_name.empty()) { uid.localpart.clear(); return uid; }
    // Check server_name has at least one dot or is a valid hostname
    if (uid.server_name.find('.') == std::string::npos &&
        uid.server_name.find(':') == std::string::npos &&
        uid.server_name != "localhost") {
        // Could be a bare hostname — still valid
    }

    return uid;
}

// ── RoomId ──
RoomId RoomId::parse(const std::string& id) {
    RoomId rid;
    if (id.empty() || id[0] != '!') return rid;

    auto colonPos = id.find(':');
    if (colonPos == std::string::npos) return rid;

    rid.localpart = id.substr(1, colonPos - 1);
    rid.server_name = id.substr(colonPos + 1);
    return rid;
}

// ── EventId ──
EventId EventId::parse(const std::string& id) {
    EventId eid;
    if (id.empty() || id[0] != '$') return eid;

    auto colonPos = id.find(':');
    if (colonPos != std::string::npos) {
        eid.id = id.substr(1, colonPos - 1);
        eid.server_name = id.substr(colonPos + 1);
    } else {
        eid.id = id.substr(1);
    }
    return eid;
}

// ── PowerLevels ──
int PowerLevels::getUserLevel(const std::string& userId) const {
    auto it = users.find(userId);
    if (it != users.end()) return it->second;
    return usersDefault;
}

int PowerLevels::getEventLevel(const std::string& eventType) const {
    auto it = events.find(eventType);
    if (it != events.end()) return it->second;
    return eventsDefault;
}

// ── Sync Filter → JSON ──
json SyncFilter::toJson() const {
    json j;
    j["room"] = json::object();
    j["room"]["timeline"] = json::object();

    if (!notTypes.empty()) j["room"]["timeline"]["not_types"] = notTypes;
    if (!notSenders.empty()) j["room"]["timeline"]["not_senders"] = notSenders;
    if (!types.empty()) j["room"]["timeline"]["types"] = types;
    if (!senders.empty()) j["room"]["timeline"]["senders"] = senders;
    if (limit > 0) j["room"]["timeline"]["limit"] = limit;
    if (lazyLoadMembers) j["room"]["timeline"]["lazy_load_members"] = true;
    if (includeRedundantMembers) j["room"]["timeline"]["include_redundant_members"] = true;
    if (unreadThreadNotifications) j["room"]["timeline"]["unread_thread_notifications"] = true;

    // Account data
    j["account_data"] = json::object();
    if (!notTypes.empty()) j["account_data"]["not_types"] = notTypes;

    // Presence
    j["presence"] = json::object();
    if (!senders.empty()) j["presence"]["senders"] = senders;

    return j;
}

// ── AuthenticationData serialization ──
json AuthenticationData::toJson() const {
    json j;
    j["user_id"] = userId;
    j["homeserver_url"] = homeServer;
    if (!identityServer.empty()) j["identity_server_url"] = identityServer;
    j["access_token"] = accessToken;
    if (!refreshToken.empty()) j["refresh_token"] = refreshToken;
    j["device_id"] = deviceId;
    if (isOidc) j["is_oidc"] = true;
    return j;
}

AuthenticationData AuthenticationData::fromJson(const json& j) {
    AuthenticationData data;
    data.userId = j.value("user_id", "");
    data.homeServer = j.value("homeserver_url", "");
    data.identityServer = j.value("identity_server_url", "");
    data.accessToken = j.value("access_token", "");
    data.refreshToken = j.value("refresh_token", "");
    data.deviceId = j.value("device_id", "");
    data.isOidc = j.value("is_oidc", false);
    return data;
}

// ── Event Type Conversions ──
EventType eventTypeFromString(const std::string& s) {
    if (s == "m.room.message") return EventType::ROOM_MESSAGE;
    if (s == "m.room.encrypted") return EventType::ROOM_MESSAGE_ENCRYPTED;
    if (s == "m.room.member") return EventType::ROOM_MEMBER;
    if (s == "m.room.create") return EventType::ROOM_CREATE;
    if (s == "m.room.join_rules") return EventType::ROOM_JOIN_RULES;
    if (s == "m.room.guest_access") return EventType::ROOM_GUEST_ACCESS;
    if (s == "m.room.history_visibility") return EventType::ROOM_HISTORY_VISIBILITY;
    if (s == "m.room.name") return EventType::ROOM_NAME;
    if (s == "m.room.topic") return EventType::ROOM_TOPIC;
    if (s == "m.room.avatar") return EventType::ROOM_AVATAR;
    if (s == "m.room.canonical_alias") return EventType::ROOM_CANONICAL_ALIAS;
    if (s == "m.room.aliases") return EventType::ROOM_ALIASES;
    if (s == "m.room.power_levels") return EventType::ROOM_POWER_LEVELS;
    if (s == "m.room.tombstone") return EventType::ROOM_TOMBSTONE;
    if (s == "m.room.encryption") return EventType::ROOM_ENCRYPTION;
    if (s == "m.room.redaction") return EventType::ROOM_REDACTION;
    if (s == "m.room.third_party_invite") return EventType::ROOM_THIRD_PARTY_INVITE;
    if (s == "m.room.pinned_events") return EventType::ROOM_PINNED_EVENTS;
    if (s == "m.room.server_acl") return EventType::ROOM_SERVER_ACL;
    if (s == "m.typing") return EventType::TYPING;
    if (s == "m.presence") return EventType::PRESENCE;
    if (s == "m.receipt") return EventType::RECEIPT;
    if (s == "m.fully_read") return EventType::FULLY_READ;
    if (s == "m.tag") return EventType::TAG;
    if (s == "m.direct") return EventType::DIRECT;
    if (s == "m.ignored_user_list") return EventType::IGNORED_USER_LIST;
    if (s == "m.push_rules") return EventType::PUSH_RULES;
    if (s == "m.reaction") return EventType::REACTION;
    if (s == "m.call.invite") return EventType::CALL_INVITE;
    if (s == "m.call.candidates") return EventType::CALL_CANDIDATES;
    if (s == "m.call.answer") return EventType::CALL_ANSWER;
    if (s == "m.call.hangup") return EventType::CALL_HANGUP;
    if (s == "m.call.negotiate") return EventType::CALL_NEGOTIATE;
    if (s == "m.call.reject") return EventType::CALL_REJECT;
    if (s == "m.poll.start") return EventType::POLL_START;
    if (s == "m.poll.response") return EventType::POLL_RESPONSE;
    if (s == "m.poll.end") return EventType::POLL_END;
    if (s == "m.location") return EventType::LOCATION;
    if (s == "m.sticker") return EventType::STICKER;
    if (s == "m.widget") return EventType::WIDGET;
    if (s == "m.key.verification.start") return EventType::KEY_VERIFICATION_START;
    if (s == "m.key.verification.accept") return EventType::KEY_VERIFICATION_ACCEPT;
    if (s == "m.key.verification.key") return EventType::KEY_VERIFICATION_KEY;
    if (s == "m.key.verification.mac") return EventType::KEY_VERIFICATION_MAC;
    if (s == "m.key.verification.cancel") return EventType::KEY_VERIFICATION_CANCEL;
    if (s == "m.key.verification.done") return EventType::KEY_VERIFICATION_DONE;
    if (s == "m.room_key") return EventType::ROOM_KEY;
    if (s == "m.room_key_request") return EventType::ROOM_KEY_REQUEST;
    if (s == "m.forwarded_room_key") return EventType::FORWARDED_ROOM_KEY;
    if (s == "m.megolm_key") return EventType::MEGOLM_KEY;
    if (s == "m.olm_key") return EventType::OLMLM_KEY;
    if (s == "m.device_list") return EventType::DEVICE_LIST;
    if (s == "m.dummy") return EventType::DUMMY;
    return EventType::UNKNOWN;
}

std::string eventTypeToString(EventType t) {
    switch (t) {
        case EventType::ROOM_MESSAGE: return "m.room.message";
        case EventType::ROOM_MESSAGE_ENCRYPTED: return "m.room.encrypted";
        case EventType::ROOM_MEMBER: return "m.room.member";
        case EventType::ROOM_CREATE: return "m.room.create";
        case EventType::ROOM_JOIN_RULES: return "m.room.join_rules";
        case EventType::ROOM_GUEST_ACCESS: return "m.room.guest_access";
        case EventType::ROOM_HISTORY_VISIBILITY: return "m.room.history_visibility";
        case EventType::ROOM_NAME: return "m.room.name";
        case EventType::ROOM_TOPIC: return "m.room.topic";
        case EventType::ROOM_AVATAR: return "m.room.avatar";
        case EventType::ROOM_CANONICAL_ALIAS: return "m.room.canonical_alias";
        case EventType::ROOM_ALIASES: return "m.room.aliases";
        case EventType::ROOM_POWER_LEVELS: return "m.room.power_levels";
        case EventType::ROOM_TOMBSTONE: return "m.room.tombstone";
        case EventType::ROOM_ENCRYPTION: return "m.room.encryption";
        case EventType::ROOM_REDACTION: return "m.room.redaction";
        case EventType::ROOM_THIRD_PARTY_INVITE: return "m.room.third_party_invite";
        case EventType::ROOM_PINNED_EVENTS: return "m.room.pinned_events";
        case EventType::ROOM_SERVER_ACL: return "m.room.server_acl";
        case EventType::TYPING: return "m.typing";
        case EventType::PRESENCE: return "m.presence";
        case EventType::RECEIPT: return "m.receipt";
        case EventType::FULLY_READ: return "m.fully_read";
        case EventType::TAG: return "m.tag";
        case EventType::DIRECT: return "m.direct";
        case EventType::IGNORED_USER_LIST: return "m.ignored_user_list";
        case EventType::PUSH_RULES: return "m.push_rules";
        case EventType::REACTION: return "m.reaction";
        case EventType::CALL_INVITE: return "m.call.invite";
        case EventType::CALL_CANDIDATES: return "m.call.candidates";
        case EventType::CALL_ANSWER: return "m.call.answer";
        case EventType::CALL_HANGUP: return "m.call.hangup";
        case EventType::CALL_NEGOTIATE: return "m.call.negotiate";
        case EventType::CALL_REJECT: return "m.call.reject";
        case EventType::POLL_START: return "m.poll.start";
        case EventType::POLL_RESPONSE: return "m.poll.response";
        case EventType::POLL_END: return "m.poll.end";
        case EventType::LOCATION: return "m.location";
        case EventType::STICKER: return "m.sticker";
        case EventType::WIDGET: return "m.widget";
        case EventType::KEY_VERIFICATION_START: return "m.key.verification.start";
        case EventType::KEY_VERIFICATION_ACCEPT: return "m.key.verification.accept";
        case EventType::KEY_VERIFICATION_KEY: return "m.key.verification.key";
        case EventType::KEY_VERIFICATION_MAC: return "m.key.verification.mac";
        case EventType::KEY_VERIFICATION_CANCEL: return "m.key.verification.cancel";
        case EventType::KEY_VERIFICATION_DONE: return "m.key.verification.done";
        case EventType::ROOM_KEY: return "m.room_key";
        case EventType::ROOM_KEY_REQUEST: return "m.room_key_request";
        case EventType::FORWARDED_ROOM_KEY: return "m.forwarded_room_key";
        case EventType::MEGOLM_KEY: return "m.megolm_key";
        case EventType::OLMLM_KEY: return "m.olm_key";
        case EventType::DEVICE_LIST: return "m.device_list";
        case EventType::DUMMY: return "m.dummy";
        default: return "m.unknown";
    }
}

MessageType messageTypeFromString(const std::string& s) {
    if (s == "m.text") return MessageType::TEXT;
    if (s == "m.notice") return MessageType::NOTICE;
    if (s == "m.emote") return MessageType::EMOTE;
    if (s == "m.image") return MessageType::IMAGE;
    if (s == "m.file") return MessageType::FILE;
    if (s == "m.audio") return MessageType::AUDIO;
    if (s == "m.video") return MessageType::VIDEO;
    if (s == "m.location") return MessageType::LOCATION;
    return MessageType::NONE;
}

std::string messageTypeToString(MessageType t) {
    switch (t) {
        case MessageType::TEXT: return "m.text";
        case MessageType::NOTICE: return "m.notice";
        case MessageType::EMOTE: return "m.emote";
        case MessageType::IMAGE: return "m.image";
        case MessageType::FILE: return "m.file";
        case MessageType::AUDIO: return "m.audio";
        case MessageType::VIDEO: return "m.video";
        case MessageType::LOCATION: return "m.location";
        default: return "m.text";
    }
}

Membership membershipFromString(const std::string& s) {
    if (s == "invite") return Membership::INVITE;
    if (s == "join") return Membership::JOIN;
    if (s == "knock") return Membership::KNOCK;
    if (s == "leave") return Membership::LEAVE;
    if (s == "ban") return Membership::BAN;
    return Membership::NONE;
}

std::string membershipToString(Membership m) {
    switch (m) {
        case Membership::INVITE: return "invite";
        case Membership::JOIN: return "join";
        case Membership::KNOCK: return "knock";
        case Membership::LEAVE: return "leave";
        case Membership::BAN: return "ban";
        default: return "leave";
    }
}

} // namespace matrix_sdk
