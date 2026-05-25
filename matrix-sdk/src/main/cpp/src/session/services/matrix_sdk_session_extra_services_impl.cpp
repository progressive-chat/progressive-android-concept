#include "matrix_sdk/session/services/matrix_sdk_session_services.hpp"
#include "matrix_sdk/session/matrix_sdk_session.hpp"
#include "matrix_sdk/matrix_sdk_types.hpp"
#include "matrix_sdk/room/services/matrix_sdk_room_services.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <vector>
#include <atomic>

namespace matrix_sdk {

// ============================================================================
// PushersServiceImpl
// ============================================================================
class PushersServiceImpl : public PushersService {
public:
    explicit PushersServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void addPusher(const std::string& pushKey, const std::string& appId,
                   const std::string& appDisplayName, const std::string& deviceDisplayName,
                   const std::string& lang, const std::string& profileTag,
                   const std::string& appUrl, bool append,
                   SimpleCallback callback) override {
        if (pushKey.empty()) {
            callback(false, "Push key is required");
            return;
        }
        if (appId.empty()) {
            callback(false, "App ID is required");
            return;
        }
        if (appDisplayName.empty()) {
            callback(false, "App display name is required");
            return;
        }
        if (deviceDisplayName.empty()) {
            callback(false, "Device display name is required");
            return;
        }
        if (lang.empty()) {
            callback(false, "Language is required");
            return;
        }

        // Validate language code format (e.g., "en", "en-US")
        static const std::regex langRegex(R"(^[a-z]{2}(-[A-Z]{2})?$)");
        if (!std::regex_match(lang, langRegex)) {
            callback(false, "Invalid language code format. Use e.g. 'en' or 'en-US'");
            return;
        }

        json pusherData;
        pusherData["pushkey"] = pushKey;
        pusherData["app_id"] = appId;
        pusherData["app_display_name"] = appDisplayName;
        pusherData["device_display_name"] = deviceDisplayName;
        pusherData["lang"] = lang;
        if (!profileTag.empty()) {
            pusherData["profile_tag"] = profileTag;
        }
        if (!appUrl.empty()) {
            pusherData["data"] = {{"url", appUrl}};
        }
        pusherData["kind"] = "http";
        pusherData["append"] = append;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::string key = pushKey + ":" + appId;
            pushers_[key] = pusherData.dump();
        }

        // POST /_matrix/client/v3/pushers/set
        callback(true, "");
    }

    void removePusher(const std::string& pushKey, const std::string& appId,
                      SimpleCallback callback) override {
        if (pushKey.empty() && appId.empty()) {
            callback(false, "Either push key or app ID is required");
            return;
        }

        json body;
        if (!pushKey.empty()) body["pushkey"] = pushKey;
        if (!appId.empty()) body["app_id"] = appId;
        body["kind"] = "http";

        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::string key = pushKey + ":" + appId;
            auto it = pushers_.find(key);
            if (it != pushers_.end()) {
                pushers_.erase(it);
            }
        }

        // POST /_matrix/client/v3/pushers/set (with kind=null to delete)
        callback(true, "");
    }

    void getPushers(std::function<void(bool, const std::string&, const std::vector<std::string>&)> callback) override {
        // GET /_matrix/client/v3/pushers
        std::vector<std::string> result;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            result.reserve(pushers_.size());
            for (const auto& [key, data] : pushers_) {
                result.push_back(data);
            }
        }
        callback(true, "", result);
    }

    // Internal helpers
    void loadPushersFromStore(const std::vector<std::string>& pusherJsons) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& pj : pusherJsons) {
            try {
                auto j = json::parse(pj);
                std::string key = j.value("pushkey", "") + ":" + j.value("app_id", "");
                pushers_[key] = pj;
            } catch (...) {}
        }
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::string> pushers_; // key: pushKey:appId -> JSON
};

// ============================================================================
// PushRuleServiceImpl
// ============================================================================
class PushRuleServiceImpl : public PushRuleService {
public:
    explicit PushRuleServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {
        initializeDefaultRules();
    }

    void fetchPushRules(SimpleCallback callback) override {
        // GET /_matrix/client/v3/pushrules/
        // In a real impl, parse the response and populate local state

        {
            std::lock_guard<std::mutex> lock(mutex_);
            isFetched_ = true;
            lastFetched_ = currentTimeMs();
        }

        callback(true, "");
    }

    void setPushRuleEnabled(const std::string& ruleId, bool enabled,
                            SimpleCallback callback) override {
        if (ruleId.empty()) {
            callback(false, "Rule ID is required");
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = ruleStates_.find(ruleId);
        if (it != ruleStates_.end()) {
            it->second.enabled = enabled;
            it->second.modified = true;
        } else {
            callback(false, "Rule not found: " + ruleId);
            return;
        }

        // PUT /_matrix/client/v3/pushrules/{scope}/{kind}/{ruleId}/enabled
        callback(true, "");
    }

    void setPushRuleActions(const std::string& ruleId,
                            const std::vector<std::string>& actions,
                            SimpleCallback callback) override {
        if (ruleId.empty()) {
            callback(false, "Rule ID is required");
            return;
        }
        if (actions.empty()) {
            callback(false, "At least one action is required");
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = ruleStates_.find(ruleId);
        if (it != ruleStates_.end()) {
            it->second.actions = actions;
            it->second.modified = true;
        } else {
            callback(false, "Rule not found: " + ruleId);
            return;
        }

        // PUT /_matrix/client/v3/pushrules/{scope}/{kind}/{ruleId}/actions
        callback(true, "");
    }

    std::string getPushRuleForRoom(const std::string& roomId) const override {
        if (roomId.empty()) return "";

        std::lock_guard<std::mutex> lock(mutex_);
        // Check room-specific rules first
        auto it = roomRules_.find(roomId);
        if (it != roomRules_.end()) {
            return it->second;
        }

        // Apply default rule evaluation
        return evaluateRuleForRoom(roomId);
    }

    bool isPushRuleEnabled(const std::string& ruleId) const override {
        if (ruleId.empty()) return false;

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = ruleStates_.find(ruleId);
        if (it != ruleStates_.end()) {
            return it->second.enabled;
        }
        return false;
    }

    std::vector<std::string> getAllPushRules() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> result;
        result.reserve(ruleStates_.size());
        for (const auto& [id, state] : ruleStates_) {
            result.push_back(id);
        }
        return result;
    }

    // Internal helpers
    void setRoomRule(const std::string& roomId, const std::string& ruleId) {
        std::lock_guard<std::mutex> lock(mutex_);
        roomRules_[roomId] = ruleId;
    }

    void removeRoomRule(const std::string& roomId) {
        std::lock_guard<std::mutex> lock(mutex_);
        roomRules_.erase(roomId);
    }

    std::string getNotificationStateForRoom(const std::string& roomId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = roomRules_.find(roomId);
        if (it != roomRules_.end()) {
            auto rit = ruleStates_.find(it->second);
            if (rit != ruleStates_.end()) {
                if (!rit->second.enabled) return "mute";
                // Check actions for sound tweaks
                for (const auto& action : rit->second.actions) {
                    if (action == "dont_notify") return "mute";
                }
                return "notify";
            }
        }
        return "notify"; // Default: notify
    }

private:
    struct PushRuleState {
        std::string ruleId;
        bool enabled = true;
        std::vector<std::string> actions;
        bool modified = false;
        PushRuleKind kind = PushRuleKind::OVERRIDE;
        int priority = 0;
    };

    void initializeDefaultRules() {
        // Default Matrix push rules (simplified subset)
        // Override rules (highest priority)
        addDefaultRule(".m.rule.master", PushRuleKind::OVERRIDE, 5, true,
            {"notify", {"set_tweak", "sound", "default"}});
        addDefaultRule(".m.rule.suppress_notices", PushRuleKind::OVERRIDE, 4, true,
            {"dont_notify"});
        addDefaultRule(".m.rule.invite_for_me", PushRuleKind::OVERRIDE, 3, true,
            {"notify", {"set_tweak", "highlight", "true"}});
        addDefaultRule(".m.rule.member_event", PushRuleKind::OVERRIDE, 2, true,
            {"dont_notify"});
        addDefaultRule(".m.rule.contains_display_name", PushRuleKind::OVERRIDE, 1, true,
            {"notify", {"set_tweak", "highlight", "true"}, {"set_tweak", "sound", "default"}});
        addDefaultRule(".m.rule.room_one_to_one", PushRuleKind::OVERRIDE, 0, true,
            {"notify", {"set_tweak", "sound", "default"}});

        // Content rules
        addDefaultRule(".m.rule.contains_user_name", PushRuleKind::CONTENT, 0, true,
            {"notify", {"set_tweak", "highlight", "true"}, {"set_tweak", "sound", "default"}});

        // Underride rules (lowest priority)
        addDefaultRule(".m.rule.call", PushRuleKind::UNDER, 0, true,
            {"notify", {"set_tweak", "sound", "ring"}});
        addDefaultRule(".m.rule.room", PushRuleKind::UNDER, 0, true,
            {"notify", {"set_tweak", "sound", "default"}});

        isFetched_ = true;
    }

    void addDefaultRule(const std::string& ruleId, PushRuleKind kind, int priority,
                        bool enabled, const std::vector<std::string>& actions) {
        PushRuleState state;
        state.ruleId = ruleId;
        state.kind = kind;
        state.priority = priority;
        state.enabled = enabled;
        state.actions = actions;
        ruleStates_[ruleId] = state;
    }

    std::string evaluateRuleForRoom(const std::string& roomId) const {
        // Simplified rule evaluation: check room rules, then defaults
        // Return the first matching rule ID
        for (const auto& [id, state] : ruleStates_) {
            if (state.enabled && state.kind == PushRuleKind::ROOM) {
                // In real impl, check if this room rule matches the roomId pattern
                return id;
            }
        }
        return ".m.rule.room_one_to_one"; // Default fallback
    }

    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, PushRuleState> ruleStates_;
    std::unordered_map<std::string, std::string> roomRules_; // roomId -> ruleId
    bool isFetched_ = false;
    int64_t lastFetched_ = 0;
};

// ============================================================================
// RoomDirectoryServiceImpl
// ============================================================================
class RoomDirectoryServiceImpl : public RoomDirectoryService {
public:
    explicit RoomDirectoryServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void getPublicRooms(const std::string& server, int limit, const std::string& since,
                        const std::string& filter, bool includeAllNetworks,
                        const std::string& thirdPartyInstanceId,
                        PublicRoomsCallback callback) override {
        if (limit <= 0 || limit > 500) {
            callback(false, "Limit must be between 1 and 500", {}, "", 0);
            return;
        }

        // POST /_matrix/client/v3/publicRooms (with optional server filter)
        // or GET /_matrix/client/v3/publicRooms?server={server}&limit={limit}&since={since}
        std::string queryServer = server.empty() ? session_->getHomeServerUrl() : server;

        // In a real implementation, this makes an HTTP request and parses the response
        // that includes: chunk[], next_batch, total_room_count_estimate

        std::vector<PublicRoom> rooms;

        // Search in local cache if filter is provided
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [roomId, room] : roomCache_) {
                if (!filter.empty()) {
                    std::string filterLower = filter;
                    std::string nameLower = room.name;
                    std::string topicLower = room.topic;
                    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
                    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                    std::transform(topicLower.begin(), topicLower.end(), topicLower.begin(), ::tolower);

                    if (nameLower.find(filterLower) == std::string::npos &&
                        topicLower.find(filterLower) == std::string::npos &&
                        room.roomId.find(filterLower) == std::string::npos) {
                        continue;
                    }
                }
                rooms.push_back(room);
                if (static_cast<int>(rooms.size()) >= limit) break;
            }
        }

        int totalCount = static_cast<int>(rooms.size());
        std::string nextBatch = rooms.size() < limit ? "" : "next_page_token";

        callback(true, "", rooms, nextBatch, totalCount);
    }

    void getRoomPreview(const std::string& roomId, const std::vector<std::string>& viaServers,
                        std::function<void(bool, const std::string&, const RoomPreview&)> callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required", RoomPreview{});
            return;
        }

        // GET /_matrix/client/v3/rooms/{roomId}/preview?via={viaServers}
        // Returns limited info about a room even if you haven't joined

        RoomPreview preview;
        preview.roomId = roomId;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = previewCache_.find(roomId);
            if (it != previewCache_.end()) {
                callback(true, "", it->second);
                return;
            }
        }

        // Simulate preview with defaults
        preview.roomName = "Unknown Room";
        preview.canonicalAlias = "";
        preview.roomType = "";
        preview.numJoinedMembers = 0;
        preview.worldReadable = false;
        preview.guestCanJoin = false;
        preview.isDirect = false;

        callback(true, "", preview);
    }

    void getThirdPartyProtocol(const std::string& protocol,
                               std::function<void(bool, const std::string&, const ThirdPartyProtocol&)> callback) override {
        if (protocol.empty()) {
            callback(false, "Protocol name is required", ThirdPartyProtocol{});
            return;
        }

        // GET /_matrix/client/v3/thirdparty/protocol/{protocol}
        ThirdPartyProtocol tp;
        tp.name = protocol;
        callback(true, "", tp);
    }

    void getThirdPartyUser(const std::string& protocol, const std::string& fields,
                           std::function<void(bool, const std::string&, const ThirdPartyUser&)> callback) override {
        if (protocol.empty()) {
            callback(false, "Protocol is required", ThirdPartyUser{});
            return;
        }

        // GET /_matrix/client/v3/thirdparty/user/{protocol}
        ThirdPartyUser user;
        callback(true, "", user);
    }

    void getThirdPartyLocation(const std::string& protocol, const std::string& fields,
                               std::function<void(bool, const std::string&, const ThirdPartyLocation&)> callback) override {
        if (protocol.empty()) {
            callback(false, "Protocol is required", ThirdPartyLocation{});
            return;
        }

        // GET /_matrix/client/v3/thirdparty/location/{protocol}
        ThirdPartyLocation location;
        callback(true, "", location);
    }

    // Internal helpers
    void cachePublicRoom(const PublicRoom& room) {
        std::lock_guard<std::mutex> lock(mutex_);
        roomCache_[room.roomId] = room;
    }

    void cacheRoomPreview(const RoomPreview& preview) {
        std::lock_guard<std::mutex> lock(mutex_);
        previewCache_[preview.roomId] = preview;
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, PublicRoom> roomCache_;
    std::unordered_map<std::string, RoomPreview> previewCache_;
};

// ============================================================================
// RoomServiceImpl
// ============================================================================
class RoomServiceImpl : public RoomService {
public:
    explicit RoomServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    std::shared_ptr<Room> getRoom(const std::string& roomId) override {
        if (roomId.empty()) return nullptr;

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = rooms_.find(roomId);
        if (it != rooms_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void getRoomLive(const std::string& roomId, RoomCallback callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required", nullptr);
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = rooms_.find(roomId);
        if (it != rooms_.end()) {
            callback(true, "", it->second);
        } else {
            callback(false, "Room not found: " + roomId, nullptr);
        }
    }

    std::vector<std::string> getRoomIds() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> result;
        result.reserve(rooms_.size());
        for (const auto& [id, room] : rooms_) {
            result.push_back(id);
        }
        return result;
    }

    void createRoom(const CreateRoomParams& params,
                    std::function<void(bool, const std::string&, const std::string&)> callback) override {
        if (params.name.empty()) {
            callback(false, "Room name is required", "");
            return;
        }

        json body;
        body["name"] = params.name;
        body["topic"] = params.topic;
        body["is_direct"] = params.isDirect;

        if (!params.roomAliasName.empty()) {
            body["room_alias_name"] = params.roomAliasName;
        }
        if (!params.roomVersion.empty()) {
            body["room_version"] = params.roomVersion;
        }

        // Preset
        switch (params.preset) {
            case RoomPreset::PRIVATE_CHAT: body["preset"] = "private_chat"; break;
            case RoomPreset::TRUSTED_PRIVATE_CHAT: body["preset"] = "trusted_private_chat"; break;
            case RoomPreset::PUBLIC_CHAT: body["preset"] = "public_chat"; break;
        }

        // Visibility
        body["visibility"] = (params.visibility == RoomVisibility::PUBLIC) ? "public" : "private";

        // Invite
        if (!params.invite.empty()) {
            body["invite"] = params.invite;
        }
        if (!params.invite3pid.empty()) {
            body["invite_3pid"] = params.invite3pid;
        }

        // Power levels
        if (params.powerLevels.has_value()) {
            json pl;
            pl["users_default"] = params.powerLevels->usersDefault;
            pl["events_default"] = params.powerLevels->eventsDefault;
            pl["state_default"] = params.powerLevels->stateDefault;
            pl["ban"] = params.powerLevels->ban;
            pl["kick"] = params.powerLevels->kick;
            pl["redact"] = params.powerLevels->redact;
            pl["invite"] = params.powerLevels->invite;
            json users;
            for (const auto& [uid, level] : params.powerLevels->users) {
                users[uid] = level;
            }
            pl["users"] = users;
            json events;
            for (const auto& [etype, level] : params.powerLevels->events) {
                events[etype] = level;
            }
            pl["events"] = events;
            body["power_level_content_override"] = pl;
        }

        // POST /_matrix/client/v3/createRoom
        std::string roomId = generateRoomId();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pendingCreates_[roomId] = params;
        }

        callback(true, roomId, "");
    }

    void joinRoom(const std::string& roomId, const std::vector<std::string>& viaServers,
                  SimpleCallback callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required");
            return;
        }

        // POST /_matrix/client/v3/join/{roomId} (with optional via servers)
        json body;
        if (!viaServers.empty()) {
            body["via"] = viaServers;
        }

        // In a real impl, the server returns the room state after joining
        callback(true, "");
    }

    void leaveRoom(const std::string& roomId, SimpleCallback callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required");
            return;
        }

        // Check room exists (or just declare intent)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (rooms_.find(roomId) != rooms_.end()) {
                pendingLeaves_.insert(roomId);
            }
        }

        // POST /_matrix/client/v3/rooms/{roomId}/leave
        callback(true, "");
    }

    void forgetRoom(const std::string& roomId, SimpleCallback callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            rooms_.erase(roomId);
        }

        // POST /_matrix/client/v3/rooms/{roomId}/forget
        callback(true, "");
    }

    void joinRoomByAlias(const std::string& alias, SimpleCallback callback) override {
        if (alias.empty()) {
            callback(false, "Room alias is required");
            return;
        }

        // Validate alias format (#something:server)
        if (alias[0] != '#' || alias.find(':') == std::string::npos) {
            callback(false, "Invalid room alias format. Expected #alias:server");
            return;
        }

        // POST /_matrix/client/v3/join/{roomAlias}
        // The server resolves the alias and returns the room state

        callback(true, "");
    }

    void knockRoom(const std::string& roomId, const std::vector<std::string>& viaServers,
                   const std::string& reason, SimpleCallback callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required");
            return;
        }

        json body;
        if (!reason.empty()) {
            body["reason"] = reason;
        }
        if (!viaServers.empty()) {
            body["via"] = viaServers;
        }

        // POST /_matrix/client/v3/knock/{roomId}
        callback(true, "");
    }

    void getExistingDirectRoomWithUser(const std::string& userId,
                                       std::function<void(bool, const std::string&, const std::string&)> callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required", "");
            return;
        }

        // Check local direct rooms map
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = directRooms_.find(userId);
            if (it != directRooms_.end()) {
                // Check the first DM room still exists
                for (const auto& roomId : it->second) {
                    if (rooms_.find(roomId) != rooms_.end()) {
                        callback(true, roomId, "");
                        return;
                    }
                }
            }
        }

        callback(false, "No existing direct room found", "");
    }

    void createDirectRoom(const std::string& userId,
                          std::function<void(bool, const std::string&, const std::string&)> callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required", "");
            return;
        }

        CreateRoomParams params;
        params.name = "DM with " + userId;
        params.preset = RoomPreset::TRUSTED_PRIVATE_CHAT;
        params.isDirect = true;
        params.invite.push_back(userId);

        createRoom(params, [this, userId, callback](bool success, const std::string& error, const std::string& roomId) {
            if (success && !roomId.empty()) {
                std::lock_guard<std::mutex> lock(mutex_);
                directRooms_[userId].push_back(roomId);
                callback(true, roomId, "");
            } else {
                callback(false, error, "");
            }
        });
    }

    // Internal helpers
    void addRoom(const std::string& roomId, std::shared_ptr<Room> room) {
        std::lock_guard<std::mutex> lock(mutex_);
        rooms_[roomId] = room;
        pendingCreates_.erase(roomId);
        pendingLeaves_.erase(roomId);
    }

    void removeRoom(const std::string& roomId) {
        std::lock_guard<std::mutex> lock(mutex_);
        rooms_.erase(roomId);
    }

    void setDirectRooms(const std::map<std::string, std::vector<std::string>>& directRooms) {
        std::lock_guard<std::mutex> lock(mutex_);
        directRooms_ = directRooms;
    }

    int getRoomCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return static_cast<int>(rooms_.size());
    }

private:
    static std::string generateRoomId() {
        static std::atomic<uint64_t> counter{0};
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "!room_" + std::to_string(now) + "_" + std::to_string(++counter);
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<Room>> rooms_;           // roomId -> Room
    std::unordered_map<std::string, CreateRoomParams> pendingCreates_;       // roomId -> params
    std::unordered_set<std::string> pendingLeaves_;
    std::map<std::string, std::vector<std::string>> directRooms_;            // userId -> roomIds
};

// ============================================================================
// SearchServiceImpl
// ============================================================================
class SearchServiceImpl : public SearchService {
public:
    explicit SearchServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void search(const std::string& query, int limit, const std::string& nextBatch,
                const std::string& order, const std::vector<std::string>& roomIds,
                SearchCallback callback) override {
        if (query.empty()) {
            callback(false, "Search query is required", {}, 0);
            return;
        }
        if (limit <= 0 || limit > 100) {
            callback(false, "Limit must be between 1 and 100", {}, 0);
            return;
        }

        json body;
        body["search_categories"] = {
            {"room_events", {
                {"search_term", query},
                {"order_by", order.empty() ? "recent" : order},
                {"event_context", {
                    {"before_limit", 5},
                    {"after_limit", 5},
                    {"include_profile", true}
                }}
            }}
        };

        if (!roomIds.empty()) {
            body["search_categories"]["room_events"]["filter"] = {
                {"rooms", roomIds}
            };
        }

        if (!nextBatch.empty()) {
            body["next_batch"] = nextBatch;
        }

        // POST /_matrix/client/v3/search
        // In a real implementation, this queries the homeserver search endpoint

        std::vector<SearchResult> results;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            // Search through cached events
            for (const auto& [eventId, cached] : eventCache_) {
                if (results.size() >= static_cast<size_t>(limit)) break;

                // Simple case-insensitive substring search
                std::string eventLower = cached.body;
                std::string queryLower = query;
                std::transform(eventLower.begin(), eventLower.end(), eventLower.begin(), ::tolower);
                std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);

                if (eventLower.find(queryLower) != std::string::npos) {
                    if (!roomIds.empty()) {
                        if (std::find(roomIds.begin(), roomIds.end(), cached.roomId) == roomIds.end()) {
                            continue;
                        }
                    }
                    results.push_back(cached);
                }
            }

            // Sort results
            if (order == "recent") {
                std::sort(results.begin(), results.end(),
                    [](const SearchResult& a, const SearchResult& b) {
                        return a.originServerTs > b.originServerTs;
                    });
            } else if (order == "rank") {
                std::sort(results.begin(), results.end(),
                    [](const SearchResult& a, const SearchResult& b) {
                        return a.rank > b.rank;
                    });
            }
        }

        callback(true, "", results, static_cast<int>(results.size()));
    }

    void searchRooms(const std::string& query, int limit, SearchCallback callback) override {
        // Search only room names/topics
        std::vector<std::string> allRoomIds;

        // Get all known room IDs (in a real impl, from the room directory service)
        search(query, limit, "", "recent", allRoomIds, callback);
    }

    void searchMessages(const std::string& query, int limit, SearchCallback callback) override {
        // Search only message events
        search(query, limit, "", "recent", {}, callback);
    }

    void searchUsers(const std::string& query, int limit,
                     std::function<void(bool, const std::string&, const std::vector<UserProfile>&)> callback) override {
        if (query.empty()) {
            callback(false, "Search query is required", {});
            return;
        }
        if (limit <= 0 || limit > 100) {
            callback(false, "Limit must be between 1 and 100", {});
            return;
        }

        std::vector<UserProfile> results;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [userId, profile] : userCache_) {
                if (results.size() >= static_cast<size_t>(limit)) break;

                std::string nameLower = profile.displayName;
                std::string queryLower = query;
                std::string userIdLower = userId;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);
                std::transform(userIdLower.begin(), userIdLower.end(), userIdLower.begin(), ::tolower);

                if (nameLower.find(queryLower) != std::string::npos ||
                    userIdLower.find(queryLower) != std::string::npos) {
                    results.push_back(profile);
                }
            }
        }

        callback(true, "", results);
    }

    // Internal helpers
    void indexEvent(const std::string& eventId, const std::string& roomId,
                    const std::string& senderId, const std::string& senderName,
                    const std::string& body, int64_t ts) {
        std::lock_guard<std::mutex> lock(mutex_);
        SearchResult sr;
        sr.eventId = eventId;
        sr.roomId = roomId;
        sr.senderId = senderId;
        sr.senderName = senderName;
        sr.body = body;
        sr.originServerTs = ts;
        sr.rank = 0.0;
        eventCache_[eventId] = sr;
    }

    void cacheUserProfile(const UserProfile& profile) {
        std::lock_guard<std::mutex> lock(mutex_);
        userCache_[profile.userId] = profile;
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, SearchResult> eventCache_;
    std::unordered_map<std::string, UserProfile> userCache_;
};

// ============================================================================
// SharedSecretStorageServiceImpl
// ============================================================================
class SharedSecretStorageServiceImpl : public SharedSecretStorageService {
public:
    explicit SharedSecretStorageServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)), defaultKeyId_("") {}

    void setSecret(const std::string& name, const std::string& value,
                   SimpleCallback callback) override {
        if (name.empty()) {
            callback(false, "Secret name is required");
            return;
        }
        if (value.empty()) {
            callback(false, "Secret value is required");
            return;
        }

        // In a real impl: PUT /_matrix/client/v3/user/{userId}/account_data/{secretType}
        // where the account data event content is encrypted with the default key

        {
            std::lock_guard<std::mutex> lock(mutex_);
            secrets_[name] = value;
        }

        // Store as account data with type m.secret_storage.key.{keyId}
        callback(true, "");
    }

    void getSecret(const std::string& name,
                   std::function<void(bool, const std::string&, const std::string&)> callback) override {
        if (name.empty()) {
            callback(false, "Secret name is required", "");
            return;
        }

        // In a real impl: GET /_matrix/client/v3/user/{userId}/account_data/{type}
        // and decrypt with the appropriate key

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = secrets_.find(name);
            if (it != secrets_.end()) {
                callback(true, it->second, "");
                return;
            }
        }

        callback(false, "Secret not found: " + name, "");
    }

    void deleteSecret(const std::string& name, SimpleCallback callback) override {
        if (name.empty()) {
            callback(false, "Secret name is required");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = secrets_.find(name);
            if (it == secrets_.end()) {
                callback(false, "Secret not found: " + name);
                return;
            }
            secrets_.erase(it);
        }

        callback(true, "");
    }

    void getDefaultKey(std::function<void(bool, const std::string&, const std::string&)> callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (defaultKeyId_.empty()) {
            callback(false, "No default key set", "");
        } else {
            callback(true, defaultKeyId_, "");
        }
    }

    void setDefaultKey(const std::string& keyId, SimpleCallback callback) override {
        if (keyId.empty()) {
            callback(false, "Key ID is required");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            defaultKeyId_ = keyId;
        }

        // Also store in account data: PUT .../account_data/m.secret_storage.default_key
        callback(true, "");
    }

    // Internal helpers
    void loadSecrets(const std::map<std::string, std::string>& stored) {
        std::lock_guard<std::mutex> lock(mutex_);
        secrets_ = stored;
    }

    std::map<std::string, std::string> dumpSecrets() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return secrets_;
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::map<std::string, std::string> secrets_; // name -> value
    std::string defaultKeyId_;
};

// ============================================================================
// SignOutServiceImpl
// ============================================================================
class SignOutServiceImpl : public SignOutService {
public:
    explicit SignOutServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void signOut(bool signOutAllDevices, SimpleCallback callback) override {
        // POST /_matrix/client/v3/logout
        // If signOutAllDevices: POST /_matrix/client/v3/logout/all

        json body;
        if (signOutAllDevices) {
            body; // All devices logout typically has no body
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            isSigningOut_ = true;
        }

        // In a real implementation:
        // 1. Call the logout endpoint
        // 2. Clear local access token
        // 3. Clean up all local data
        // 4. Notify listeners

        callback(true, "");
    }

    void softLogout(SimpleCallback callback) override {
        // POST /_matrix/client/v3/logout
        // The server returns a soft_logout: true in the error
        // which means the session can be refreshed instead of fully destroyed

        {
            std::lock_guard<std::mutex> lock(mutex_);
            isSoftLogout_ = true;
            softLogoutRequested_ = currentTimeMs();
        }

        callback(true, "");
    }

    void clearLocalData(SimpleCallback callback) override {
        // Clear all local storage, caches, and databases
        // This does NOT call the server — purely local cleanup

        {
            std::lock_guard<std::mutex> lock(mutex_);
            isSigningOut_ = true;
            localDataCleared_ = true;
            localDataClearedAt_ = currentTimeMs();
        }

        callback(true, "");
    }

    // Internal helpers
    bool isSigningOut() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return isSigningOut_;
    }

    bool isSoftLogoutRequested() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return isSoftLogout_;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        isSigningOut_ = false;
        isSoftLogout_ = false;
    }

private:
    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    bool isSigningOut_ = false;
    bool isSoftLogout_ = false;
    bool localDataCleared_ = false;
    int64_t softLogoutRequested_ = 0;
    int64_t localDataClearedAt_ = 0;
};

// ============================================================================
// SpaceServiceImpl
// ============================================================================
class SpaceServiceImpl : public SpaceService {
public:
    explicit SpaceServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void getSpaceChildren(const std::string& spaceId, int maxRoomsPerSpace,
                          SpaceChildrenCallback callback) override {
        if (spaceId.empty()) {
            callback(false, "Space ID is required", {});
            return;
        }
        if (maxRoomsPerSpace <= 0) {
            maxRoomsPerSpace = 50; // Sensible default
        }

        // GET /_matrix/client/v3/rooms/{spaceId}/hierarchy
        // Query params: ?max_rooms_per_space={maxRoomsPerSpace}
        // This returns a paginated list of child rooms/spaces

        std::vector<SpaceChildInfo> children;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = spaceChildren_.find(spaceId);
            if (it != spaceChildren_.end()) {
                children = it->second;
            }
        }

        // In a real impl, this would query the server
        // and return paginated results with a next_batch token
        callback(true, "", children);
    }

    void getSpaceSummary(const std::string& spaceId,
                         std::function<void(bool, const std::string&, const std::string&)> callback) override {
        if (spaceId.empty()) {
            callback(false, "Space ID is required", "");
            return;
        }

        // GET /_matrix/client/v3/rooms/{spaceId}/hierarchy (summary only)
        // Returns a JSON summary of the space hierarchy

        json summary;
        summary["space_id"] = spaceId;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = spaceChildren_.find(spaceId);
            summary["children_count"] = it != spaceChildren_.end() ?
                static_cast<int>(it->second.size()) : 0;
        }

        callback(true, summary.dump(), "");
    }

    std::shared_ptr<Room> getSpace(const std::string& spaceId) override {
        if (spaceId.empty()) return nullptr;

        // Spaces are just rooms with type "m.space"
        // Delegate to the room service
        // In a real impl: roomService->getRoom(spaceId) with space type check

        return nullptr; // Would be filled by actual room service integration
    }

    std::vector<std::string> getRootSpaces() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> result;
        for (const auto& [spaceId, parent] : spaceParents_) {
            if (parent.empty()) {
                result.push_back(spaceId);
            }
        }
        return result;
    }

    void addRoomToSpace(const std::string& spaceId, const std::string& roomId,
                        bool suggested, const std::string& order,
                        const std::string& via, SimpleCallback callback) override {
        if (spaceId.empty()) {
            callback(false, "Space ID is required");
            return;
        }
        if (roomId.empty()) {
            callback(false, "Room ID is required");
            return;
        }
        if (spaceId == roomId) {
            callback(false, "Cannot add a space to itself");
            return;
        }

        // PUT /_matrix/client/v3/rooms/{spaceId}/state/m.space.child/{roomId}
        json content;
        content["suggested"] = suggested;
        if (!order.empty()) content["order"] = order;
        if (!via.empty()) content["via"] = {via};

        {
            std::lock_guard<std::mutex> lock(mutex_);
            SpaceChildInfo child;
            child.roomId = roomId;
            child.isSuggested = suggested;
            child.order = order;
            child.isKnown = true;

            auto& children = spaceChildren_[spaceId];
            bool found = false;
            for (auto& c : children) {
                if (c.roomId == roomId) {
                    c = child;
                    found = true;
                    break;
                }
            }
            if (!found) {
                children.push_back(child);
            }
            spaceParents_[roomId] = spaceId;
        }

        callback(true, "");
    }

    void removeRoomFromSpace(const std::string& spaceId, const std::string& roomId,
                             SimpleCallback callback) override {
        if (spaceId.empty() || roomId.empty()) {
            callback(false, "Space ID and Room ID are required");
            return;
        }

        // PUT /_matrix/client/v3/rooms/{spaceId}/state/m.space.child/{roomId}
        // with empty content body to remove

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = spaceChildren_.find(spaceId);
            if (it != spaceChildren_.end()) {
                auto& children = it->second;
                children.erase(
                    std::remove_if(children.begin(), children.end(),
                        [&roomId](const SpaceChildInfo& c) { return c.roomId == roomId; }),
                    children.end());
            }
            spaceParents_.erase(roomId);
        }

        callback(true, "");
    }

    void setSpaceOrder(const std::string& spaceId, const std::string& order,
                       SimpleCallback callback) override {
        if (spaceId.empty()) {
            callback(false, "Space ID is required");
            return;
        }

        // PUT /_matrix/client/v3/rooms/{spaceId}/state/m.space.order
        json content;
        content["order"] = order;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            spaceOrders_[spaceId] = order;
        }

        callback(true, "");
    }

    // Internal helpers
    void loadSpaceHierarchy(const std::string& spaceId,
                            const std::vector<SpaceChildInfo>& children) {
        std::lock_guard<std::mutex> lock(mutex_);
        spaceChildren_[spaceId] = children;
        for (const auto& child : children) {
            spaceParents_[child.roomId] = spaceId;
        }
    }

    bool isSpace(const std::string& roomId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return spaceChildren_.find(roomId) != spaceChildren_.end() ||
               spaceOrders_.find(roomId) != spaceOrders_.end();
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<SpaceChildInfo>> spaceChildren_;  // spaceId -> children
    std::unordered_map<std::string, std::string> spaceParents_;                   // roomId -> parentSpaceId
    std::unordered_map<std::string, std::string> spaceOrders_;                    // spaceId -> order
};

// ============================================================================
// TermsServiceImpl
// ============================================================================
class TermsServiceImpl : public TermsService {
public:
    explicit TermsServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void getTerms(const std::string& baseUrl, const std::string& language,
                  std::function<void(bool, const std::string&, const std::string&)> callback) override {
        // GET /_matrix/client/v3/terms?language={language}
        // If baseUrl is provided, use the identity server's terms endpoint:
        // GET {baseUrl}/_matrix/identity/v2/terms

        std::string targetUrl;
        if (!baseUrl.empty()) {
            targetUrl = baseUrl + "/_matrix/identity/v2/terms";
        } else {
            targetUrl = session_->getHomeServerUrl() + "/_matrix/client/v3/terms";
        }

        if (!language.empty()) {
            targetUrl += "?language=" + language;
        }

        // In a real implementation, this fetches and returns the terms JSON
        json terms;
        terms["policies"] = json::object();

        callback(true, terms.dump(), "");
    }

    void agreeToTerms(const std::string& baseUrl, const std::vector<std::string>& agreedUrls,
                      SimpleCallback callback) override {
        if (agreedUrls.empty()) {
            callback(false, "At least one agreed URL is required");
            return;
        }

        std::string targetUrl;
        if (!baseUrl.empty()) {
            targetUrl = baseUrl + "/_matrix/identity/v2/terms";
        } else {
            targetUrl = session_->getHomeServerUrl() + "/_matrix/client/v3/terms";
        }

        json body;
        body["user_accepts"] = agreedUrls;

        // POST /_matrix/client/v3/terms with JSON body
        // POST {baseUrl}/_matrix/identity/v2/terms

        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& url : agreedUrls) {
                agreedUrls_.insert(url);
            }
        }

        callback(true, "");
    }

    bool hasUnagreedTerms() const override {
        // Check if there are policies we haven't agreed to
        // In a real impl, compare local agreed URLs against server-provided policies
        std::lock_guard<std::mutex> lock(mutex_);
        return !pendingTerms_.empty();
    }

    // Internal helpers
    void setPendingTerms(const std::vector<std::string>& urls) {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingTerms_ = urls;
    }

    void clearPendingTerms() {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingTerms_.clear();
    }

    std::vector<std::string> getPendingTerms() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pendingTerms_;
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_set<std::string> agreedUrls_;
    std::vector<std::string> pendingTerms_;
};

// ============================================================================
// ThirdPartyServiceImpl
// ============================================================================
class ThirdPartyServiceImpl : public ThirdPartyService {
public:
    explicit ThirdPartyServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void getThirdPartyProtocol(const std::string& protocol,
                               std::function<void(bool, const std::string&, const ThirdPartyProtocol&)> callback) override {
        if (protocol.empty()) {
            callback(false, "Protocol name is required", ThirdPartyProtocol{});
            return;
        }

        // GET /_matrix/client/v3/thirdparty/protocol/{protocol}
        ThirdPartyProtocol tp;
        tp.name = protocol;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = protocols_.find(protocol);
            if (it != protocols_.end()) {
                callback(true, "", it->second);
                return;
            }
        }

        // Simulate default response
        tp.fields = {"username", "nickname"};
        tp.userFields = {"username", "nickname"};
        tp.locationFields = {"channel", "network"};
        tp.icon = "";
        tp.instances = {};

        {
            std::lock_guard<std::mutex> lock(mutex_);
            protocols_[protocol] = tp;
        }

        callback(true, "", tp);
    }

    void getThirdPartyUser(const std::string& protocol, const std::string& fields,
                           std::function<void(bool, const std::string&, const ThirdPartyUser&)> callback) override {
        if (protocol.empty()) {
            callback(false, "Protocol is required", ThirdPartyUser{});
            return;
        }

        // GET /_matrix/client/v3/thirdparty/user/{protocol}?fields={fields}
        ThirdPartyUser user;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = users_.find(protocol);
            if (it != users_.end()) {
                callback(true, "", it->second);
                return;
            }
        }

        callback(true, "", user);
    }

    void getThirdPartyLocation(const std::string& protocol, const std::string& fields,
                               std::function<void(bool, const std::string&, const ThirdPartyLocation&)> callback) override {
        if (protocol.empty()) {
            callback(false, "Protocol is required", ThirdPartyLocation{});
            return;
        }

        // GET /_matrix/client/v3/thirdparty/location/{protocol}?fields={fields}
        ThirdPartyLocation location;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = locations_.find(protocol);
            if (it != locations_.end()) {
                callback(true, "", it->second);
                return;
            }
        }

        callback(true, "", location);
    }

    // Internal helpers
    void cacheProtocol(const ThirdPartyProtocol& protocol) {
        std::lock_guard<std::mutex> lock(mutex_);
        protocols_[protocol.name] = protocol;
    }

    void cacheUser(const std::string& protocol, const ThirdPartyUser& user) {
        std::lock_guard<std::mutex> lock(mutex_);
        users_[protocol] = user;
    }

    void cacheLocation(const std::string& protocol, const ThirdPartyLocation& location) {
        std::lock_guard<std::mutex> lock(mutex_);
        locations_[protocol] = location;
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, ThirdPartyProtocol> protocols_;
    std::unordered_map<std::string, ThirdPartyUser> users_;
    std::unordered_map<std::string, ThirdPartyLocation> locations_;
};

// ============================================================================
// TypingUsersTrackerImpl
// ============================================================================
class TypingUsersTrackerImpl : public TypingUsersTracker {
public:
    explicit TypingUsersTrackerImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void trackTyping(const std::string& roomId, SimpleCallback callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required");
            return;
        }

        // PUT /_matrix/client/v3/rooms/{roomId}/typing/{userId}
        json body;
        body["typing"] = true;
        body["timeout"] = 30000; // 30 seconds default typing timeout

        // In a real implementation, this sends a typing notification
        // and starts a local timer to auto-clear after timeout

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto& state = typingStates_[roomId];
            state[session_->getUserId()] = currentTimeMs() + 30000;
        }

        callback(true, "");
    }

    std::vector<std::string> getTypingUsers(const std::string& roomId) const override {
        if (roomId.empty()) return {};

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = typingStates_.find(roomId);
        if (it == typingStates_.end()) return {};

        std::vector<std::string> users;
        int64_t now = currentTimeMs();
        for (const auto& [userId, expiresAt] : it->second) {
            if (expiresAt > now) {
                users.push_back(userId);
            }
        }
        return users;
    }

    bool isUserTyping(const std::string& roomId, const std::string& userId) const override {
        if (roomId.empty() || userId.empty()) return false;

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = typingStates_.find(roomId);
        if (it == typingStates_.end()) return false;

        auto uit = it->second.find(userId);
        if (uit == it->second.end()) return false;

        return uit->second > currentTimeMs();
    }

    // Internal helpers - called from sync/events
    void updateTypingUsers(const std::string& roomId,
                           const std::vector<std::string>& userIds,
                           int timeoutMs) {
        if (roomId.empty()) return;

        int64_t expiresAt = currentTimeMs() + timeoutMs;
        std::lock_guard<std::mutex> lock(mutex_);
        auto& state = typingStates_[roomId];

        if (userIds.empty()) {
            // Clear all typing for this room
            state.clear();
            return;
        }

        // Remove users no longer typing
        std::unordered_set<std::string> currentSet(userIds.begin(), userIds.end());
        for (auto it = state.begin(); it != state.end(); ) {
            if (currentSet.find(it->first) == currentSet.end()) {
                it = state.erase(it);
            } else {
                ++it;
            }
        }

        // Add/update typing users
        for (const auto& userId : userIds) {
            state[userId] = expiresAt;
        }
    }

    void clearExpired() {
        int64_t now = currentTimeMs();
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [roomId, users] : typingStates_) {
            for (auto it = users.begin(); it != users.end(); ) {
                if (it->second <= now) {
                    it = users.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

private:
    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unordered_map<std::string, int64_t>> typingStates_;
    // roomId -> (userId -> expiresAt timestamp)
};

// ============================================================================
// UserServiceImpl
// ============================================================================
class UserServiceImpl : public UserService {
public:
    explicit UserServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void getUser(const std::string& userId, UserCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required", UserProfile{});
            return;
        }

        // Check cache first
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = userCache_.find(userId);
            if (it != userCache_.end()) {
                callback(true, "", it->second);
                return;
            }
        }

        // GET /_matrix/client/v3/profile/{userId}
        // GET /_matrix/client/v3/profile/{userId}/displayname
        // GET /_matrix/client/v3/profile/{userId}/avatar_url

        // Simulate user profile fetch
        UserProfile profile;
        profile.userId = userId;
        profile.lastUpdated = currentTimeMs();

        // Parse display name from userId
        if (userId.size() > 1 && userId[0] == '@') {
            auto colonPos = userId.find(':');
            if (colonPos != std::string::npos) {
                profile.displayName = userId.substr(1, colonPos - 1);
            } else {
                profile.displayName = userId;
            }
        } else {
            profile.displayName = userId;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            userCache_[userId] = profile;
        }

        callback(true, "", profile);
    }

    std::optional<UserProfile> getCachedUser(const std::string& userId) const override {
        if (userId.empty()) return std::nullopt;

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = userCache_.find(userId);
        if (it != userCache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void fetchUsers(const std::vector<std::string>& userIds,
                    std::function<void(bool, const std::string&, const std::map<std::string, UserProfile>&)> callback) override {
        if (userIds.empty()) {
            callback(false, "User IDs list is required", {});
            return;
        }

        std::map<std::string, UserProfile> results;
        std::vector<std::string> toFetch;

        // Check cache first
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& userId : userIds) {
                auto it = userCache_.find(userId);
                if (it != userCache_.end()) {
                    results[userId] = it->second;
                } else {
                    toFetch.push_back(userId);
                }
            }
        }

        // Fetch missing profiles (simulated)
        for (const auto& userId : toFetch) {
            UserProfile profile;
            profile.userId = userId;
            profile.lastUpdated = currentTimeMs();
            if (userId.size() > 1 && userId[0] == '@') {
                auto colonPos = userId.find(':');
                if (colonPos != std::string::npos) {
                    profile.displayName = userId.substr(1, colonPos - 1);
                } else {
                    profile.displayName = userId;
                }
            }

            {
                std::lock_guard<std::mutex> lock(mutex_);
                userCache_[userId] = profile;
                results[userId] = profile;
            }
        }

        callback(true, "", results);
    }

    void searchUsers(const std::string& query, int limit,
                     std::function<void(bool, const std::string&, const std::vector<UserProfile>&)> callback) override {
        if (query.empty()) {
            callback(false, "Search query is required", {});
            return;
        }
        if (limit <= 0 || limit > 100) {
            callback(false, "Limit must be between 1 and 100", {});
            return;
        }

        std::vector<UserProfile> results;

        // POST /_matrix/client/v3/user_directory/search
        // Body: {"search_term": query, "limit": limit}

        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::string queryLower = query;
            std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);

            for (const auto& [userId, profile] : userCache_) {
                if (results.size() >= static_cast<size_t>(limit)) break;

                std::string nameLower = profile.displayName;
                std::string idLower = userId;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                std::transform(idLower.begin(), idLower.end(), idLower.begin(), ::tolower);

                if (nameLower.find(queryLower) != std::string::npos ||
                    idLower.find(queryLower) != std::string::npos) {
                    results.push_back(profile);
                }
            }
        }

        callback(true, "", results);
    }

    void setDisplayName(const std::string& displayName, SimpleCallback callback) override {
        if (displayName.empty()) {
            callback(false, "Display name cannot be empty");
            return;
        }
        if (displayName.length() > 256) {
            callback(false, "Display name must be at most 256 characters");
            return;
        }

        // PUT /_matrix/client/v3/profile/{userId}/displayname
        std::string ownUserId = session_->getUserId();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = userCache_.find(ownUserId);
            if (it != userCache_.end()) {
                it->second.displayName = displayName;
            } else {
                UserProfile profile;
                profile.userId = ownUserId;
                profile.displayName = displayName;
                userCache_[ownUserId] = profile;
            }
        }

        callback(true, "");
    }

    void setAvatarUrl(const std::string& mxcUrl, SimpleCallback callback) override {
        if (mxcUrl.empty()) {
            callback(false, "Avatar MXC URL is required");
            return;
        }

        static const std::regex mxcRegex(R"(^mxc://[^/]+/[^/]+$)");
        if (!std::regex_match(mxcUrl, mxcRegex)) {
            callback(false, "Invalid MXC URL format");
            return;
        }

        // PUT /_matrix/client/v3/profile/{userId}/avatar_url
        std::string ownUserId = session_->getUserId();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = userCache_.find(ownUserId);
            if (it != userCache_.end()) {
                it->second.avatarUrl = mxcUrl;
            } else {
                UserProfile profile;
                profile.userId = ownUserId;
                profile.avatarUrl = mxcUrl;
                userCache_[ownUserId] = profile;
            }
        }

        callback(true, "");
    }

    void ignoreUser(const std::string& userId, SimpleCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required");
            return;
        }
        if (userId == session_->getUserId()) {
            callback(false, "Cannot ignore yourself");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            ignoredUsers_.insert(userId);
        }

        // PUT /_matrix/client/v3/user/{userId}/account_data/m.ignored_user_list
        callback(true, "");
    }

    void unignoreUser(const std::string& userId, SimpleCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = ignoredUsers_.find(userId);
            if (it == ignoredUsers_.end()) {
                callback(false, "User is not ignored: " + userId);
                return;
            }
            ignoredUsers_.erase(it);
        }

        // PUT /_matrix/client/v3/user/{userId}/account_data/m.ignored_user_list
        callback(true, "");
    }

    std::vector<std::string> getIgnoredUsers() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::vector<std::string>(ignoredUsers_.begin(), ignoredUsers_.end());
    }

    // Internal helpers
    void updateUserFromEvent(const std::string& userId, const UserProfile& profile) {
        std::lock_guard<std::mutex> lock(mutex_);
        userCache_[userId] = profile;
        userCache_[userId].lastUpdated = currentTimeMs();
    }

    void clearCache() {
        std::lock_guard<std::mutex> lock(mutex_);
        userCache_.clear();
    }

    bool isUserIgnored(const std::string& userId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return ignoredUsers_.find(userId) != ignoredUsers_.end();
    }

private:
    static int64_t currentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, UserProfile> userCache_;
    std::unordered_set<std::string> ignoredUsers_;
};

// ============================================================================
// WidgetServiceImpl
// ============================================================================
class WidgetServiceImpl : public WidgetService {
public:
    explicit WidgetServiceImpl(std::shared_ptr<Session> session)
        : session_(std::move(session)) {}

    void getWidgets(const std::string& roomId,
                    std::function<void(bool, const std::string&, const std::vector<Widget>&)> callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required", {});
            return;
        }

        // GET /_matrix/client/v3/rooms/{roomId}/state/im.vector.modular.widgets
        // or GET /_matrix/client/v3/user/{userId}/account_data/im.vector.modular.widgets?room_id={roomId}

        std::vector<Widget> widgets;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = roomWidgets_.find(roomId);
            if (it != roomWidgets_.end()) {
                widgets = it->second;
            }
        }

        callback(true, "", widgets);
    }

    void createWidget(const std::string& roomId, const Widget& widget,
                      SimpleCallback callback) override {
        if (roomId.empty()) {
            callback(false, "Room ID is required");
            return;
        }
        if (widget.widgetId.empty()) {
            callback(false, "Widget ID is required");
            return;
        }
        if (widget.url.empty()) {
            callback(false, "Widget URL is required");
            return;
        }

        // Validate widget URL
        static const std::regex urlRegex(R"(^https?://)");
        if (!std::regex_search(widget.url, urlRegex)) {
            callback(false, "Widget URL must be HTTP or HTTPS");
            return;
        }

        // Validate widget type
        if (widget.widgetType.empty()) {
            callback(false, "Widget type is required (e.g., 'm.custom', 'm.jitsi')");
            return;
        }

        json content;
        content["id"] = widget.widgetId;
        content["type"] = widget.widgetType;
        content["url"] = widget.url;
        content["name"] = widget.name;
        content["creatorUserId"] = widget.creatorUserId.empty() ?
            session_->getUserId() : widget.creatorUserId;
        content["waitForIframeLoad"] = widget.waitForIframeLoad;

        // PUT /_matrix/client/v3/rooms/{roomId}/state/im.vector.modular.widgets/{widgetId}
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto& widgets = roomWidgets_[roomId];
            // Check for duplicates
            for (auto& w : widgets) {
                if (w.widgetId == widget.widgetId) {
                    w = widget;
                    callback(true, "");
                    return;
                }
            }
            widgets.push_back(widget);
        }

        callback(true, "");
    }

    void deleteWidget(const std::string& roomId, const std::string& widgetId,
                      SimpleCallback callback) override {
        if (roomId.empty() || widgetId.empty()) {
            callback(false, "Room ID and Widget ID are required");
            return;
        }

        // PUT /_matrix/client/v3/rooms/{roomId}/state/im.vector.modular.widgets/{widgetId}
        // with empty content to delete

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = roomWidgets_.find(roomId);
            if (it != roomWidgets_.end()) {
                auto& widgets = it->second;
                auto before = widgets.size();
                widgets.erase(
                    std::remove_if(widgets.begin(), widgets.end(),
                        [&widgetId](const Widget& w) { return w.widgetId == widgetId; }),
                    widgets.end());
                if (widgets.size() == before) {
                    callback(false, "Widget not found: " + widgetId);
                    return;
                }
            } else {
                callback(false, "No widgets found for room: " + roomId);
                return;
            }
        }

        callback(true, "");
    }

    void getWidget(const std::string& roomId, const std::string& widgetId,
                   std::function<void(bool, const std::string&, const Widget&)> callback) override {
        if (roomId.empty() || widgetId.empty()) {
            callback(false, "Room ID and Widget ID are required", Widget{});
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = roomWidgets_.find(roomId);
            if (it != roomWidgets_.end()) {
                for (const auto& w : it->second) {
                    if (w.widgetId == widgetId) {
                        callback(true, "", w);
                        return;
                    }
                }
            }
        }

        callback(false, "Widget not found: " + widgetId, Widget{});
    }

    // Internal helpers
    void loadWidgets(const std::string& roomId, const std::vector<Widget>& widgets) {
        std::lock_guard<std::mutex> lock(mutex_);
        roomWidgets_[roomId] = widgets;
    }

    std::vector<std::string> getWidgetTypes(const std::string& roomId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> types;
        auto it = roomWidgets_.find(roomId);
        if (it != roomWidgets_.end()) {
            for (const auto& w : it->second) {
                types.push_back(w.widgetType);
            }
        }
        return types;
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<Widget>> roomWidgets_; // roomId -> widgets
};

// ============================================================================
// Service Factory Functions
// ============================================================================

std::shared_ptr<PushersService> createPushersService(std::shared_ptr<Session> session) {
    return std::make_shared<PushersServiceImpl>(session);
}

std::shared_ptr<PushRuleService> createPushRuleService(std::shared_ptr<Session> session) {
    return std::make_shared<PushRuleServiceImpl>(session);
}

std::shared_ptr<RoomDirectoryService> createRoomDirectoryService(std::shared_ptr<Session> session) {
    return std::make_shared<RoomDirectoryServiceImpl>(session);
}

std::shared_ptr<RoomService> createRoomService(std::shared_ptr<Session> session) {
    return std::make_shared<RoomServiceImpl>(session);
}

std::shared_ptr<SearchService> createSearchService(std::shared_ptr<Session> session) {
    return std::make_shared<SearchServiceImpl>(session);
}

std::shared_ptr<SharedSecretStorageService> createSharedSecretStorageService(
    std::shared_ptr<Session> session) {
    return std::make_shared<SharedSecretStorageServiceImpl>(session);
}

std::shared_ptr<SignOutService> createSignOutService(std::shared_ptr<Session> session) {
    return std::make_shared<SignOutServiceImpl>(session);
}

std::shared_ptr<SpaceService> createSpaceService(std::shared_ptr<Session> session) {
    return std::make_shared<SpaceServiceImpl>(session);
}

std::shared_ptr<TermsService> createTermsService(std::shared_ptr<Session> session) {
    return std::make_shared<TermsServiceImpl>(session);
}

std::shared_ptr<ThirdPartyService> createThirdPartyService(std::shared_ptr<Session> session) {
    return std::make_shared<ThirdPartyServiceImpl>(session);
}

std::shared_ptr<TypingUsersTracker> createTypingUsersTracker(std::shared_ptr<Session> session) {
    return std::make_shared<TypingUsersTrackerImpl>(session);
}

std::shared_ptr<UserService> createUserService(std::shared_ptr<Session> session) {
    return std::make_shared<UserServiceImpl>(session);
}

std::shared_ptr<WidgetService> createWidgetService(std::shared_ptr<Session> session) {
    return std::make_shared<WidgetServiceImpl>(session);
}

} // namespace matrix_sdk
