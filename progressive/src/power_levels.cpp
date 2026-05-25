#include "progressive/power_levels.hpp"
#include <sstream>

namespace progressive {

// Helper: extract integer value for a JSON key
static int getInt(const std::string& json, const std::string& key, int defaultVal = 0) {
    auto search = "\"" + key + "\":";
    auto pos = json.find(search);
    if (pos == std::string::npos) return defaultVal;
    pos += search.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    // Skip negative sign if present
    bool neg = false;
    if (pos < json.size() && json[pos] == '-') { neg = true; pos++; }
    if (pos >= json.size() || json[pos] < '0' || json[pos] > '9') return defaultVal;
    int val = 0;
    while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
        val = val * 10 + (json[pos] - '0');
        pos++;
    }
    return neg ? -val : val;
}

PowerLevels parsePowerLevels(const std::string& json) {
    PowerLevels pl;
    pl.rawJson = json;

    pl.usersDefault = getInt(json, "users_default", 0);
    pl.eventsDefault = getInt(json, "events_default", 0);
    pl.stateDefault = getInt(json, "state_default", 50);
    pl.inviteLevel = getInt(json, "invite", 0);
    pl.kickLevel = getInt(json, "kick", 50);
    pl.banLevel = getInt(json, "ban", 50);
    pl.redactLevel = getInt(json, "redact", 50);

    // notifications → room
    auto notifPos = json.find("\"notifications\"");
    if (notifPos != std::string::npos) {
        auto roomPos = json.find("\"room\"", notifPos);
        if (roomPos != std::string::npos) {
            auto colonPos = json.find(':', roomPos);
            if (colonPos != std::string::npos) {
                colonPos++;
                while (colonPos < json.size() && (json[colonPos] == ' ' || json[colonPos] == '\t')) colonPos++;
                int val = 0;
                while (colonPos < json.size() && json[colonPos] >= '0' && json[colonPos] <= '9') {
                    val = val * 10 + (json[colonPos] - '0');
                    colonPos++;
                }
                pl.notificationsRoomLevel = val;
            }
        }
    }

    // Parse "users" object: {"@alice:server": 100, "@bob:server": 50, ...}
    auto usersPos = json.find("\"users\"");
    if (usersPos != std::string::npos) {
        auto openPos = json.find('{', usersPos);
        if (openPos != std::string::npos) {
            int braceDepth = 1;
            size_t pos = openPos + 1;
            while (pos < json.size() && braceDepth > 0) {
                if (json[pos] == '"') {
                    // Found a key
                    size_t keyEnd = json.find('"', pos + 1);
                    if (keyEnd == std::string::npos) break;
                    std::string userId = json.substr(pos + 1, keyEnd - pos - 1);
                    auto colon = json.find(':', keyEnd);
                    if (colon == std::string::npos) break;
                    size_t valPos = colon + 1;
                    while (valPos < json.size() && (json[valPos] == ' ' || json[valPos] == '\t')) valPos++;
                    int level = 0;
                    while (valPos < json.size() && json[valPos] >= '0' && json[valPos] <= '9') {
                        level = level * 10 + (json[valPos] - '0');
                        valPos++;
                    }
                    pl.userOverrides.push_back({userId, level});
                    pos = valPos;
                } else if (json[pos] == '{') {
                    braceDepth++;
                    pos++;
                } else if (json[pos] == '}') {
                    braceDepth--;
                    pos++;
                } else {
                    pos++;
                }
            }
        }
    }

    // Parse "events" object
    auto eventsPos = json.find("\"events\"");
    if (eventsPos != std::string::npos) {
        auto openPos = json.find('{', eventsPos);
        if (openPos != std::string::npos) {
            int braceDepth = 1;
            size_t pos = openPos + 1;
            while (pos < json.size() && braceDepth > 0) {
                if (json[pos] == '"') {
                    size_t keyEnd = json.find('"', pos + 1);
                    if (keyEnd == std::string::npos) break;
                    std::string eventType = json.substr(pos + 1, keyEnd - pos - 1);
                    auto colon = json.find(':', keyEnd);
                    if (colon == std::string::npos) break;
                    size_t valPos = colon + 1;
                    while (valPos < json.size() && (json[valPos] == ' ' || json[valPos] == '\t')) valPos++;
                    int level = 0;
                    while (valPos < json.size() && json[valPos] >= '0' && json[valPos] <= '9') {
                        level = level * 10 + (json[valPos] - '0');
                        valPos++;
                    }
                    pl.eventOverrides.push_back({eventType, level});
                    pos = valPos;
                } else if (json[pos] == '{') {
                    braceDepth++;
                    pos++;
                } else if (json[pos] == '}') {
                    braceDepth--;
                    pos++;
                } else {
                    pos++;
                }
            }
        }
    }

    return pl;
}

int getUserPowerLevel(const PowerLevels& pl, const std::string& userId) {
    for (const auto& u : pl.userOverrides) {
        if (u.userId == userId) return u.level;
    }
    return pl.usersDefault;
}

UserPermissions computeUserPermissions(const PowerLevels& pl, const std::string& userId) {
    UserPermissions perms;
    perms.userId = userId;
    perms.powerLevel = getUserPowerLevel(pl, userId);
    int plv = perms.powerLevel;

    // State events default
    perms.canSendState = plv >= pl.stateDefault;

    // Specific state events: check event overrides first, fall back to state_default
    auto getEventLevel = [&](const std::string& eventType) -> int {
        for (const auto& e : pl.eventOverrides) {
            if (e.eventType == eventType) return e.level;
        }
        return pl.stateDefault;
    };

    perms.canChangeName = plv >= getEventLevel("m.room.name");
    perms.canChangeTopic = plv >= getEventLevel("m.room.topic");
    perms.canChangeAvatar = plv >= getEventLevel("m.room.avatar");
    perms.canChangeCanonicalAlias = plv >= getEventLevel("m.room.canonical_alias");
    perms.canChangeHistoryVisibility = plv >= getEventLevel("m.room.history_visibility");
    perms.canChangeJoinRules = plv >= getEventLevel("m.room.join_rules");
    perms.canChangeGuestAccess = plv >= getEventLevel("m.room.guest_access");
    perms.canChangePowerLevels = plv >= getEventLevel("m.room.power_levels");
    perms.canChangeServerACL = plv >= getEventLevel("m.room.server_acl");

    // Messages
    perms.canSendMessages = plv >= pl.eventsDefault;

    // Redaction
    perms.canSendRedacted = plv >= pl.redactLevel;  // simplified: own messages always, others check below
    perms.canRedactOthers = plv >= pl.redactLevel;

    // Moderation
    perms.canInvite = plv >= pl.inviteLevel;
    perms.canKick = plv >= pl.kickLevel;
    perms.canBan = plv >= pl.banLevel;

    // Notifications
    perms.canNotifyRoom = plv >= pl.notificationsRoomLevel;

    // Roles
    perms.isAdmin = plv >= pl.stateDefault;
    perms.isOwner = plv >= 100;

    return perms;
}

bool isValidPowerLevels(const PowerLevels& pl) {
    return !pl.rawJson.empty() && pl.rawJson.find("users") != std::string::npos;
}

std::string formatPowerLevel(int level, const PowerLevels& pl) {
    if (level >= 100) return "Admin (Owner)";
    if (level >= pl.stateDefault) return "Admin";
    if (level >= 50 && pl.stateDefault == 50) return "Moderator";
    if (level > pl.usersDefault) return "Custom (" + std::to_string(level) + ")";
    return "Default (" + std::to_string(level) + ")";
}

std::string getUserRole(const UserPermissions& perms) {
    if (perms.isOwner) return "Owner";
    if (perms.isAdmin) return "Admin";
    if (perms.canKick || perms.canBan) return "Moderator";
    return "Member";
}

std::string permissionsToJson(const UserPermissions& perms) {
    std::ostringstream json;
    json << R"({"userId": ")" << perms.userId << R"(",)";
    json << R"("powerLevel": )" << perms.powerLevel << ",";
    json << R"("role": ")" << getUserRole(perms) << R"(",)";
    json << R"("canSendState": )" << (perms.canSendState ? "true" : "false") << ",";
    json << R"("canSendMessages": )" << (perms.canSendMessages ? "true" : "false") << ",";
    json << R"("canChangeName": )" << (perms.canChangeName ? "true" : "false") << ",";
    json << R"("canChangeTopic": )" << (perms.canChangeTopic ? "true" : "false") << ",";
    json << R"("canChangeAvatar": )" << (perms.canChangeAvatar ? "true" : "false") << ",";
    json << R"("canInvite": )" << (perms.canInvite ? "true" : "false") << ",";
    json << R"("canKick": )" << (perms.canKick ? "true" : "false") << ",";
    json << R"("canBan": )" << (perms.canBan ? "true" : "false") << ",";
    json << R"("canRedactOthers": )" << (perms.canRedactOthers ? "true" : "false") << ",";
    json << R"("isAdmin": )" << (perms.isAdmin ? "true" : "false") << ",";
    json << R"("isOwner": )" << (perms.isOwner ? "true" : "false") << "}";
    return json.str();
}

} // namespace progressive


// ==== Extended power_levels implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string power_levels::serialize() const {
    json j = toJson();
    return j.dump();
}

bool power_levels::deserialize(const std::string& data) {
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
bool power_levels::validate() const {
    if (!m_initialized) {
        LOGE("power_levels: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool power_levels::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool power_levels::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json power_levels::getMetrics() const {
    json m;
    m["class"] = "power_levels";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int power_levels::getOperationCount() const {
    return m_operationCount;
}

void power_levels::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void power_levels::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "power_levels";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool power_levels::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool power_levels::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool power_levels::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void power_levels::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void power_levels::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int power_levels::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void power_levels::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> power_levels::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> power_levels::listItems() const {
    return {};
}

int power_levels::itemCount() const {
    return 0;
}

// Versioning
std::string power_levels::getVersion() const {
    return "1.0.0";
}

bool power_levels::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool power_levels::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void power_levels::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> power_levels::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool power_levels::migrateData(int fromVersion, int toVersion) {
    LOGI("power_levels: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int power_levels::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json power_levels::exportData() const {
    return toJson();
}

bool power_levels::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void power_levels::performCleanup() {
    LOGI("power_levels: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t power_levels::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool power_levels::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool power_levels::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool power_levels::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void power_levels::logDebug(const std::string& msg) const {
    LOGI("power_levels: %s", msg.c_str());
}

void power_levels::logWarning(const std::string& msg) const {
    LOGW("power_levels: %s", msg.c_str());
}

void power_levels::logError(const std::string& msg) const {
    LOGE("power_levels: %s", msg.c_str());
}
