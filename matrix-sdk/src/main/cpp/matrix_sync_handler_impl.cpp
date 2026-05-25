#include "matrix_sync_handler.hpp"
#include "matrix_client.hpp"
#include "matrix_room.hpp"
#include "matrix_session.hpp"
#include "matrix_push_rules.hpp"
#include <android/log.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <set>
#include <map>
#include <sstream>
#include <chrono>

#define LOG_TAG "SyncHandler"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {
using json = nlohmann::json;

// ── Sync Response Parser ──

struct SyncParseContext {
    std::string nextBatch;
    std::map<std::string, json> joinedRooms;
    std::map<std::string, json> invitedRooms;
    std::map<std::string, json> leftRooms;
    json presence;
    json accountData;
    json toDevice;
    std::vector<std::string> deviceListChanges;
    std::vector<std::string> deviceListLeft;
    int oneTimeKeyCount = 0;
    int64_t parseStartMs = 0;
    int eventsProcessed = 0;
    int stateEventsProcessed = 0;
    int accountDataEvents = 0;
    std::vector<std::string> errors;
};

static SyncParseContext g_syncCtx;

// Parse a full /sync v2 response
bool parseSyncResponse(const std::string& rawJson, SyncParseContext& ctx) {
    ctx.parseStartMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    ctx.eventsProcessed = 0;
    ctx.stateEventsProcessed = 0;
    ctx.accountDataEvents = 0;
    ctx.errors.clear();

    try {
        json j = json::parse(rawJson);
        ctx.nextBatch = j.value("next_batch", "");

        // Parse rooms
        if (j.contains("rooms")) {
            auto& rooms = j["rooms"];
            if (rooms.contains("join")) {
                for (auto& [roomId, roomData] : rooms["join"].items()) {
                    ctx.joinedRooms[roomId] = roomData;
                    parseRoomData(roomId, roomData, "join", ctx);
                }
            }
            if (rooms.contains("invite")) {
                for (auto& [roomId, roomData] : rooms["invite"].items()) {
                    ctx.invitedRooms[roomId] = roomData;
                    parseRoomData(roomId, roomData, "invite", ctx);
                }
            }
            if (rooms.contains("leave")) {
                for (auto& [roomId, roomData] : rooms["leave"].items()) {
                    ctx.leftRooms[roomId] = roomData;
                    parseRoomData(roomId, roomData, "leave", ctx);
                }
            }
        }

        // Parse presence
        if (j.contains("presence") && j["presence"].contains("events")) {
            ctx.presence = j["presence"]["events"];
            ctx.eventsProcessed += static_cast<int>(ctx.presence.size());
        }

        // Parse account_data
        if (j.contains("account_data") && j["account_data"].contains("events")) {
            ctx.accountData = j["account_data"]["events"];
            ctx.accountDataEvents = static_cast<int>(ctx.accountData.size());
        }

        // Parse to_device
        if (j.contains("to_device") && j["to_device"].contains("events")) {
            ctx.toDevice = j["to_device"]["events"];
            ctx.eventsProcessed += static_cast<int>(ctx.toDevice.size());
        }

        // Parse device_lists
        if (j.contains("device_lists")) {
            auto& dl = j["device_lists"];
            if (dl.contains("changed")) {
                for (auto& uid : dl["changed"]) ctx.deviceListChanges.push_back(uid);
            }
            if (dl.contains("left")) {
                for (auto& uid : dl["left"]) ctx.deviceListLeft.push_back(uid);
            }
        }

        // Parse device_one_time_keys_count
        if (j.contains("device_one_time_keys_count")) {
            auto& otk = j["device_one_time_keys_count"];
            if (otk.contains("signed_curve25519")) {
                ctx.oneTimeKeyCount = otk["signed_curve25519"];
            }
        }

        return true;
    } catch (const std::exception& e) {
        LOGE("Sync parse error: %s", e.what());
        ctx.errors.push_back(std::string("Parse error: ") + e.what());
        return false;
    }
}

// Parse room data from sync response
void parseRoomData(const std::string& roomId, const json& roomData, const std::string& membership, SyncParseContext& ctx) {
    LOGI("Parsing room %s (%s)", roomId.c_str(), membership.c_str());

    // Parse timeline events
    if (roomData.contains("timeline")) {
        auto& timeline = roomData["timeline"];
        bool limited = timeline.value("limited", false);
        std::string prevBatch = timeline.value("prev_batch", "");

        if (timeline.contains("events")) {
            for (auto& event : timeline["events"]) {
                std::string type = event.value("type", "");
                std::string eventId = event.value("event_id", "");
                std::string sender = event.value("sender", "");
                int64_t ts = event.value("origin_server_ts", 0LL);

                if (type == "m.room.message" || type == "m.sticker") {
                    ctx.eventsProcessed++;
                    processTimelineEvent(roomId, event);
                } else if (event.contains("state_key")) {
                    ctx.stateEventsProcessed++;
                    processStateEvent(roomId, event);
                }
            }
        }
    }

    // Parse state events
    if (roomData.contains("state")) {
        auto& state = roomData["state"];
        if (state.contains("events")) {
            for (auto& event : state["events"]) {
                ctx.stateEventsProcessed++;
                processStateEvent(roomId, event);
            }
        }
    }

    // Parse ephemeral events (typing, receipts)
    if (roomData.contains("ephemeral")) {
        auto& ephemeral = roomData["ephemeral"];
        if (ephemeral.contains("events")) {
            for (auto& event : ephemeral["events"]) {
                std::string type = event.value("type", "");
                if (type == "m.typing") {
                    processTypingEvent(roomId, event);
                } else if (type == "m.receipt") {
                    processReceiptEvent(roomId, event);
                }
            }
        }
    }

    // Parse unread notifications
    if (roomData.contains("unread_notifications")) {
        auto& unread = roomData["unread_notifications"];
        int notificationCount = unread.value("notification_count", 0);
        int highlightCount = unread.value("highlight_count", 0);
        updateRoomUnreadCounts(roomId, notificationCount, highlightCount);
    }

    // Parse summary
    if (roomData.contains("summary")) {
        auto& summary = roomData["summary"];
        int joinedMembers = summary.value("m.joined_member_count", 0);
        int invitedMembers = summary.value("m.invited_member_count", 0);
        updateRoomSummary(roomId, joinedMembers, invitedMembers);
    }
}

// Process timeline event
void processTimelineEvent(const std::string& roomId, const json& event) {
    std::string eventId = event.value("event_id", "");
    std::string sender = event.value("sender", "");
    std::string type = event.value("type", "");
    auto content = event.value("content", json::object());

    // Deduplication check
    static std::set<std::string> processedEvents;
    if (processedEvents.find(eventId) != processedEvents.end()) {
        LOGW("Duplicate event: %s", eventId.c_str());
        return;
    }
    processedEvents.insert(eventId);

    // Trim processed set periodically
    if (processedEvents.size() > 100000) {
        auto it = processedEvents.begin();
        for (int i = 0; i < 50000 && it != processedEvents.end(); i++, ++it);
        processedEvents.erase(processedEvents.begin(), it);
    }

    // Handle different message types
    std::string msgType = content.value("msgtype", "");
    std::string body = content.value("body", "");

    if (msgType == "m.text" || msgType == "m.notice" || msgType == "m.emote") {
        LOGI("Message from %s in %s: %s", sender.c_str(), roomId.c_str(), body.substr(0, 50).c_str());
    } else if (msgType == "m.image") {
        std::string url = content.value("url", "");
        LOGI("Image from %s in %s: %s", sender.c_str(), roomId.c_str(), url.c_str());
    } else if (msgType == "m.file") {
        std::string filename = content.value("filename", body);
        LOGI("File from %s in %s: %s", sender.c_str(), roomId.c_str(), filename.c_str());
    }

    // Check for edit/relation
    if (content.contains("m.relates_to")) {
        auto& relates = content["m.relates_to"];
        std::string relType = relates.value("rel_type", "");
        std::string relEventId = relates.value("event_id", "");
        if (relType == "m.replace") {
            LOGI("Edit to %s from %s", relEventId.c_str(), sender.c_str());
        } else if (relType == "m.annotation") {
            std::string key = relates.value("key", "");
            LOGI("Reaction %s to %s from %s", key.c_str(), relEventId.c_str(), sender.c_str());
        }
    }
}

// Process state event
void processStateEvent(const std::string& roomId, const json& event) {
    std::string type = event.value("type", "");
    std::string stateKey = event.value("state_key", "");
    auto content = event.value("content", json::object());
    std::string sender = event.value("sender", "");

    if (type == "m.room.name") {
        std::string name = content.value("name", "");
        LOGI("Room %s name: %s", roomId.c_str(), name.c_str());
    } else if (type == "m.room.topic") {
        std::string topic = content.value("topic", "");
        LOGI("Room %s topic: %s", roomId.c_str(), topic.c_str());
    } else if (type == "m.room.member") {
        std::string membership = content.value("membership", "");
        std::string displayName = content.value("displayname", "");
        LOGI("Member %s in %s: %s (%s)", stateKey.c_str(), roomId.c_str(), membership.c_str(), displayName.c_str());
        
        // Track member count changes
        if (membership == "join" || membership == "invite" || membership == "leave" || membership == "ban") {
            recordMembershipChange(roomId, stateKey, membership);
        }
    } else if (type == "m.room.encryption") {
        std::string algorithm = content.value("algorithm", "");
        int rotationMsgs = content.value("rotation_period_msgs", 0);
        int rotationMs = content.value("rotation_period_ms", 0);
        LOGI("Room %s encryption: %s (rotate every %d msgs or %d ms)", roomId.c_str(), algorithm.c_str(), rotationMsgs, rotationMs);
    } else if (type == "m.room.power_levels") {
        LOGI("Room %s power levels updated by %s", roomId.c_str(), sender.c_str());
    } else if (type == "m.room.join_rules") {
        std::string joinRule = content.value("join_rule", "");
        LOGI("Room %s join rule: %s", roomId.c_str(), joinRule.c_str());
    } else if (type == "m.room.guest_access") {
        std::string guestAccess = content.value("guest_access", "");
        LOGI("Room %s guest access: %s", roomId.c_str(), guestAccess.c_str());
    } else if (type == "m.room.tombstone") {
        std::string replacementRoom = content.value("replacement_room", "");
        std::string reason = content.value("body", "");
        LOGI("Room %s tombstoned → %s: %s", roomId.c_str(), replacementRoom.c_str(), reason.c_str());
    } else if (type == "m.room.create") {
        std::string creator = content.value("creator", "");
        std::string version = content.value("room_version", "1");
        LOGI("Room %s created by %s (v%s)", roomId.c_str(), creator.c_str(), version.c_str());
    } else if (type == "m.room.canonical_alias") {
        std::string alias = content.value("alias", "");
        LOGI("Room %s canonical alias: %s", roomId.c_str(), alias.c_str());
    } else if (type == "m.room.history_visibility") {
        std::string vis = content.value("history_visibility", "");
        LOGI("Room %s history visibility: %s", roomId.c_str(), vis.c_str());
    } else if (type == "m.room.server_acl") {
        LOGI("Room %s server ACL updated", roomId.c_str());
    }
}

// Process typing notification
void processTypingEvent(const std::string& roomId, const json& event) {
    auto content = event.value("content", json::object());
    if (content.contains("user_ids")) {
        auto& userIds = content["user_ids"];
        if (userIds.size() > 0) {
            LOGI("Typing in %s: %zu users", roomId.c_str(), userIds.size());
        }
    }
}

// Process read receipt
void processReceiptEvent(const std::string& roomId, const json& event) {
    auto content = event.value("content", json::object());
    int totalReceipts = 0;
    for (auto& [eventId, receipts] : content.items()) {
        if (receipts.contains("m.read")) {
            totalReceipts += static_cast<int>(receipts["m.read"].size());
        }
    }
    if (totalReceipts > 0) {
        LOGI("Receipts in %s: %d reads", roomId.c_str(), totalReceipts);
    }
}

// Update room unread counts
void updateRoomUnreadCounts(const std::string& roomId, int notificationCount, int highlightCount) {
    static std::map<std::string, std::pair<int,int>> roomUnread;
    roomUnread[roomId] = {notificationCount, highlightCount};
    if (highlightCount > 0) {
        LOGI("Room %s: %d unread, %d highlights", roomId.c_str(), notificationCount, highlightCount);
    }
}

// Update room summary
void updateRoomSummary(const std::string& roomId, int joinedMembers, int invitedMembers) {
    static std::map<std::string, std::pair<int,int>> roomSummary;
    roomSummary[roomId] = {joinedMembers, invitedMembers};
}

// Record membership change for notification purposes
void recordMembershipChange(const std::string& roomId, const std::string& userId, const std::string& membership) {
    static std::map<std::string, std::map<std::string, std::string>> memberTracking;
    auto& room = memberTracking[roomId];
    std::string oldMembership = room[userId];
    room[userId] = membership;
    if (oldMembership != membership && !oldMembership.empty()) {
        LOGI("Membership change: %s in %s: %s → %s", userId.c_str(), roomId.c_str(), oldMembership.c_str(), membership.c_str());
    }
}

// Account data processing
bool processAccountDataSync(const json& accountData) {
    if (!accountData.is_array()) return false;
    for (auto& event : accountData) {
        std::string type = event.value("type", "");
        auto content = event.value("content", json::object());
        if (type == "m.direct") {
            LOGI("Processing m.direct account data");
            for (auto& [userId, rooms] : content.items()) {
                LOGI("  DM with %s: %zu rooms", userId.c_str(), rooms.size());
            }
        } else if (type == "m.push_rules") {
            LOGI("Processing push rules account data");
        } else if (type == "m.ignored_user_list") {
            LOGI("Processing ignored user list");
        } else if (type == "m.widgets") {
            LOGI("Processing widgets account data");
        }
    }
    return true;
}

// Device list processing
void processDeviceLists(const std::vector<std::string>& changed, const std::vector<std::string>& left) {
    if (!changed.empty()) {
        LOGI("Device list changed for %zu users", changed.size());
    }
    if (!left.empty()) {
        LOGI("Device list left for %zu users", left.size());
    }
}

// One-time key count monitoring
void checkOneTimeKeyCount(int count, int threshold) {
    if (count < threshold) {
        LOGW("One-time key count low: %d (threshold: %d) — uploading new keys", count, threshold);
    }
}

// Sync error recovery

// Sync filter builder
std::string buildSyncFilter(int timelineLimit, bool includePresence, bool includeTyping) {
    json filter;
    filter["room"]["timeline"]["limit"] = timelineLimit;
    filter["room"]["state"]["lazy_load_members"] = true;
    
    if (includePresence) {
        filter["presence"]["types"] = json::array({"m.presence"});
    }
    if (includeTyping) {
        filter["room"]["ephemeral"]["types"] = json::array({"m.typing", "m.receipt"});
    }
    
    filter["account_data"]["types"] = json::array({"*"});
    return filter.dump();
}

// Sync backoff calculator
int calculateBackoffMs(int consecutiveErrors, int baseDelayMs) {
    // Exponential backoff with jitter
    int delay = baseDelayMs * (1 << std::min(consecutiveErrors, 10));
    int jitter = delay / 4;
    return delay + (rand() % (jitter * 2)) - jitter;
}

// Sync statistics

static SyncStats g_syncStats;

SyncStats getSyncStats() { return g_syncStats; }

void resetSyncStats() {
    g_syncStats = SyncStats{};
}

} // namespace matrix_sdk
