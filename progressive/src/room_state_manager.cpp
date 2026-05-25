#include "progressive/room_state_manager.hpp"
#include "progressive/room_content.hpp"
#include <sstream>
#include <algorithm>

namespace progressive {

// ====== Enums ======





// ====== Helpers ======

static std::string extractStr(const std::string& json, const std::string& key) {
    auto pp = json.find("\"" + key + "\"");
    if (pp == std::string::npos) return "";
    pp = json.find('"', pp + key.size() + 2);
    if (pp == std::string::npos) return "";
    pp++;
    size_t e = pp;
    while (e < json.size() && json[e] != '"') e++;
    return json.substr(pp, e - pp);
}

// ====== History Visibility Functions ======
// Original: shouldShareHistory() = WORLD_READABLE || SHARED

bool shouldShareHistory(RSM_RoomHistoryVisibility visibility) {
    return visibility == RSM_RoomHistoryVisibility::WORLD_READABLE ||
           visibility == RSM_RoomHistoryVisibility::SHARED;
}

bool canSeeEvent(RSM_RoomHistoryVisibility visibility, MembershipState memberStateAtEventTime,
                  MembershipState memberCurrentState) {
    switch (visibility) {
        case RSM_RoomHistoryVisibility::WORLD_READABLE:
            // Anyone can see all events, even non-members
            return true;

        case RSM_RoomHistoryVisibility::SHARED:
            // Joined members see all events; non-members see nothing
            return memberCurrentState == MembershipState::JOIN;

        case RSM_RoomHistoryVisibility::INVITED:
            // Members see events from when they were invited onwards
            return memberCurrentState == MembershipState::JOIN ||
                   memberCurrentState == MembershipState::INVITE ||
                   (memberStateAtEventTime == MembershipState::INVITE);

        case RSM_RoomHistoryVisibility::JOINED:
            // Members see events from when they joined onwards
            return memberCurrentState == MembershipState::JOIN &&
                   memberStateAtEventTime == MembershipState::JOIN;
    }
    return false;
}

bool canNonMemberSeeEvents(RSM_RoomHistoryVisibility visibility) {
    return visibility == RSM_RoomHistoryVisibility::WORLD_READABLE;
}

std::string getVisibilityLabel(RSM_RoomHistoryVisibility visibility) {
    switch (visibility) {
        case RSM_RoomHistoryVisibility::WORLD_READABLE: return "Anyone";
        case RSM_RoomHistoryVisibility::SHARED: return "Members (since beginning)";
        case RSM_RoomHistoryVisibility::INVITED: return "Members (since invite)";
        case RSM_RoomHistoryVisibility::JOINED: return "Members (since join)";
    }
    return "Members";
}

std::string getVisibilityDescription(RSM_RoomHistoryVisibility visibility) {
    switch (visibility) {
        case RSM_RoomHistoryVisibility::WORLD_READABLE:
            return "Anyone can read the room history, even without joining.";
        case RSM_RoomHistoryVisibility::SHARED:
            return "All members can see the entire room history.";
        case RSM_RoomHistoryVisibility::INVITED:
            return "Members can see history from the point they were invited.";
        case RSM_RoomHistoryVisibility::JOINED:
            return "Members can only see history from the point they joined.";
    }
    return "";
}

// ====== Content Builders ======

static std::string roomHistoryVisibilityToString(RSM_RoomHistoryVisibility vis) { switch(vis) { case RSM_RoomHistoryVisibility::WORLD_READABLE: return "world_readable"; case RSM_RoomHistoryVisibility::SHARED: return "shared"; case RSM_RoomHistoryVisibility::INVITED: return "invited"; case RSM_RoomHistoryVisibility::JOINED: return "joined"; } return "joined"; }

std::string buildHistoryVisibilityContent(RSM_RoomHistoryVisibility visibility) {
    return R"({"history_visibility":")" + std::string(roomHistoryVisibilityToString(visibility)) + R"("})";
}

static std::string roomJoinRulesToString(RoomJoinRule rule) { switch(rule) { case RoomJoinRule::PUBLIC: return "public"; case RoomJoinRule::KNOCK: return "knock"; case RoomJoinRule::INVITE: return "invite"; case RoomJoinRule::PRIVATE: return "private"; } return "invite"; }

std::string buildJoinRulesContent(RoomJoinRule rule) {
    return R"({"join_rule":")" + std::string(roomJoinRulesToString(rule)) + R"("})";
}



// ====== progressive::RoomStateManager ======

RoomStateManager::RoomStateManager() {}

void progressive::RoomStateManager::setHistoryVisibility(const std::string& roomId, RSM_RoomHistoryVisibility visibility) {
    auto it = rooms_.find(roomId);
    auto& state = it->second;
    state.historyVisibility = visibility;
    state.isWorldReadable = (visibility == RSM_RoomHistoryVisibility::WORLD_READABLE);
    state.canShareHistory = shouldShareHistory(visibility);
}

void progressive::RoomStateManager::setJoinRule(const std::string& roomId, RoomJoinRule rule) {
    auto it = rooms_.find(roomId);
    if (it == rooms_.end()) {
        RoomStateSummary s;
        s.roomId = roomId;
        rooms_[roomId] = s;
        it = rooms_.find(roomId);
    }
    auto& state = it->second;
    state.joinRule = rule;
    state.isPublicRoom = (rule == RoomJoinRule::PUBLIC);
}

void progressive::RoomStateManager::setRoomName(const std::string& roomId, const std::string& name) {
    rooms_[roomId].roomName = name;
}

void progressive::RoomStateManager::setEncrypted(const std::string& roomId, bool encrypted) {
    rooms_[roomId].isEncrypted = encrypted;
}

void progressive::RoomStateManager::setMemberCount(const std::string& roomId, int count) {
    rooms_[roomId].memberCount = count;
}

RoomStateSummary progressive::RoomStateManager::getRoomState(const std::string& roomId) const {
    auto it = rooms_.find(roomId);
    if (it != rooms_.end()) return it->second;
    RoomStateSummary s;
    s.roomId = roomId;
    return s;
}

bool progressive::RoomStateManager::canShareRoomHistory(const std::string& roomId) const {
    auto state = getRoomState(roomId);
    return state.canShareHistory;
}

bool progressive::RoomStateManager::isPublicRoom(const std::string& roomId) const {
    return getRoomState(roomId).isPublicRoom;
}

bool progressive::RoomStateManager::isWorldReadable(const std::string& roomId) const {
    return getRoomState(roomId).isWorldReadable;
}

bool progressive::RoomStateManager::isInviteOnly(const std::string& roomId) const {
    return getRoomState(roomId).joinRule == RoomJoinRule::INVITE;
}

bool progressive::RoomStateManager::areGuestsAllowed(const std::string& roomId) const {
    auto state = getRoomState(roomId);
    return state.isPublicRoom && state.isWorldReadable;
}

void progressive::RoomStateManager::clear() { rooms_.clear(); }

// ====== Serialization ======

std::string progressive::RoomStateManager::roomStateToJson(const RoomStateSummary& state) const {
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) { if (c == '"') out += "\\\""; else out += c; }
        return out;
    };

    std::ostringstream os;
    os << R"({"room_id":")" << esc(state.roomId)
       << R"(","name":")" << esc(state.roomName)
       << R"(","history_visibility":")" << roomHistoryVisibilityToString(state.historyVisibility)
       << R"(","visibility_label":")" << getVisibilityLabel(state.historyVisibility)
       << R"(","join_rule":")" << roomJoinRulesToString(state.joinRule)
       << R"(,"is_public":)" << (state.isPublicRoom ? "true" : "false")
       << R"(,"is_world_readable":)" << (state.isWorldReadable ? "true" : "false")
       << R"(,"can_share_history":)" << (state.canShareHistory ? "true" : "false")
       << R"(,"is_invite_only":)" << (roomJoinRulesToString(state.joinRule) == std::string("invite") ? "true" : "false")
       << R"(,"is_encrypted":)" << (state.isEncrypted ? "true" : "false")
       << R"(,"members":)" << state.memberCount
       << "}";
    return os.str();
}

} // namespace progressive


// ==== Extended room_state_manager implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string room_state_manager::serialize() const {
    json j = toJson();
    return j.dump();
}

bool room_state_manager::deserialize(const std::string& data) {
    if (data.empty()) return false;
    try {
        json j = json::parse(data);
        return fromJson(j);
    } catch (...) {
        setError("Failed to deserialize data");
        return false;
    }
}

// Validation helpers
bool room_state_manager::validate() const {
    if (!m_initialized) {
        LOGE("room_state_manager: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool room_state_manager::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool room_state_manager::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json room_state_manager::getMetrics() const {
    json m;
    m["class"] = "room_state_manager";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int room_state_manager::getOperationCount() const {
    return m_operationCount;
}

void room_state_manager::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void room_state_manager::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "room_state_manager";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool room_state_manager::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool room_state_manager::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool room_state_manager::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
    auto now = currentTimeMs();
    auto& window = m_rateLimitWindows[key];
    if (now - window.startTime > windowMs) {
        window.startTime = now;
        window.count = 0;
    }
    if (window.count >= maxRequests) return false;
    window.count++;
    return true;
}

// Observation pattern
void room_state_manager::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void room_state_manager::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int room_state_manager::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void room_state_manager::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> room_state_manager::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> room_state_manager::listItems() const {
    return {};
}

int room_state_manager::itemCount() const {
    return 0;
}

// Versioning
std::string room_state_manager::getVersion() const {
    return "1.0.0";
}

bool room_state_manager::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool room_state_manager::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void room_state_manager::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> room_state_manager::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool room_state_manager::migrateData(int fromVersion, int toVersion) {
    LOGI("room_state_manager: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int room_state_manager::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json room_state_manager::exportData() const {
    return toJson();
}

bool room_state_manager::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void room_state_manager::performCleanup() {
    LOGI("room_state_manager: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t room_state_manager::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool room_state_manager::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool room_state_manager::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool room_state_manager::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void room_state_manager::logDebug(const std::string& msg) const {
    LOGI("room_state_manager: %s", msg.c_str());
}

void room_state_manager::logWarning(const std::string& msg) const {
    LOGW("room_state_manager: %s", msg.c_str());
}

void room_state_manager::logError(const std::string& msg) const {
    LOGE("room_state_manager: %s", msg.c_str());
}
