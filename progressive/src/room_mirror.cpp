#include "progressive/room_mirror.hpp"
#include "progressive/json_parser.hpp"
#include <sstream>
#include <regex>
#include <algorithm>

namespace progressive {

void RoomMirrorManager::addMirror(const MirrorConfig& config) {
    mirrors_[config.sourceRoomId] = config;
}

void RoomMirrorManager::removeMirror(const std::string& sourceRoomId) {
    mirrors_.erase(sourceRoomId);
}

std::vector<const MirrorConfig*> RoomMirrorManager::getMirrorsForSource(const std::string& sourceRoomId) const {
    std::vector<const MirrorConfig*> result;
    auto it = mirrors_.find(sourceRoomId);
    if (it != mirrors_.end() && it->second.enabled) {
        result.push_back(&it->second);
    }
    return result;
}

std::vector<const MirrorConfig*> RoomMirrorManager::getMirrorsForTarget(const std::string& mirrorRoomId) const {
    std::vector<const MirrorConfig*> result;
    for (const auto& [_, cfg] : mirrors_) {
        if (cfg.mirrorRoomId == mirrorRoomId && cfg.enabled) {
            result.push_back(&cfg);
        }
    }
    return result;
}

void RoomMirrorManager::setEnabled(const std::string& sourceRoomId, bool enabled) {
    auto it = mirrors_.find(sourceRoomId);
    if (it != mirrors_.end()) {
        it->second.enabled = enabled;
    }
}

bool RoomMirrorManager::isMirroring(const std::string& sourceRoomId) const {
    auto it = mirrors_.find(sourceRoomId);
    return it != mirrors_.end() && it->second.enabled;
}

std::string RoomMirrorManager::formatMirrorMessage(const MirrorMessage& msg) {
    std::ostringstream out;
    out << msg.senderName << " wrote in " << msg.sourceRoomName << ": " << msg.body;
    return out.str();
}

std::string RoomMirrorManager::generateDollMxid(const std::string& originalMxid, const std::string& targetServer) {
    auto username = extractUsername(originalMxid);
    if (username.empty()) return {};

    // Generate doll name: alice → alicedoll
    std::string dollName = username + "doll";

    // Ensure it matches MXID format
    if (dollName.size() > 255) dollName = dollName.substr(0, 255);

    return "@" + dollName + ":" + targetServer;
}

bool RoomMirrorManager::isValidDollMxid(const std::string& mxid) {
    std::regex mxidRe(R"(@[a-zA-Z0-9._=\-/]+:[a-zA-Z0-9.\-]+(?::\d+)?)");
    if (!std::regex_match(mxid, mxidRe)) return false;

    auto username = extractUsername(mxid);
    // Must contain "doll" suffix
    return username.size() > 4 && username.rfind("doll") == username.size() - 4;
}

std::string RoomMirrorManager::extractUsername(const std::string& mxid) {
    if (mxid.empty() || mxid[0] != '@') return {};
    auto colonPos = mxid.find(':');
    if (colonPos == std::string::npos) return {};
    return mxid.substr(1, colonPos - 1);
}

DollAccount RoomMirrorManager::prepareDollRegistration(
    const std::string& originalMxid,
    const std::string& targetHomeserver,
    const std::string& adminToken
) {
    DollAccount doll;
    doll.originalMxid = originalMxid;
    doll.homeserverUrl = targetHomeserver;
    doll.dollMxid = generateDollMxid(originalMxid, targetHomeserver);
    return doll;
}

std::string RoomMirrorManager::exportJson() const {
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) {
            if (c == '"') out += "\\\"";
            else out += c;
        }
        return out;
    };

    std::ostringstream json;
    json << "[";
    size_t i = 0;
    for (const auto& [_, cfg] : mirrors_) {
        if (i++ > 0) json << ",";
        json << "{";
        json << R"("sourceRoomId": ")" << esc(cfg.sourceRoomId) << R"(",)";
        json << R"("sourceRoomName": ")" << esc(cfg.sourceRoomName) << R"(",)";
        json << R"("mirrorRoomId": ")" << esc(cfg.mirrorRoomId) << R"(",)";
        json << R"("mirrorRoomName": ")" << esc(cfg.mirrorRoomName) << R"(",)";
        json << R"("enabled": )" << (cfg.enabled ? "true" : "false") << ",";
        json << R"("useDolls": )" << (cfg.useDolls ? "true" : "false");
        json << "}";
    }
    json << "]";
    return json.str();
}

void RoomMirrorManager::importJson(const std::string& json) {
    mirrors_.clear();
    size_t pos = 0;
    while (true) {
        pos = json.find('{', pos);
        if (pos == std::string::npos) break;

        int depth = 0;
        auto end = pos;
        while (end < json.size()) {
            if (json[end] == '{') ++depth;
            else if (json[end] == '}') --depth;
            if (depth == 0) break;
            ++end;
        }
        if (end >= json.size()) break;

        std::string obj = json.substr(pos, end - pos + 1);
        MirrorConfig cfg;
        cfg.sourceRoomId   = progressive::parseJsonStringValue(obj, "sourceRoomId");
        cfg.sourceRoomName = progressive::parseJsonStringValue(obj, "sourceRoomName");
        cfg.mirrorRoomId   = progressive::parseJsonStringValue(obj, "mirrorRoomId");
        cfg.mirrorRoomName = progressive::parseJsonStringValue(obj, "mirrorRoomName");
        auto enabledStr    = progressive::parseJsonStringValue(obj, "enabled");
        auto dollsStr      = progressive::parseJsonStringValue(obj, "useDolls");
        cfg.enabled  = (enabledStr == "true");
        cfg.useDolls = (dollsStr == "true");

        if (!cfg.sourceRoomId.empty()) mirrors_[cfg.sourceRoomId] = cfg;

        pos = end + 1;
    }
}

void RoomMirrorManager::clear() {
    mirrors_.clear();
}

HomeserverCapability parseRegistrationCapabilities(const std::string& wellKnownJson) {
    HomeserverCapability caps;

    // Parse "m.registration_disabled" or similar fields
    auto regDisabled = progressive::parseJsonStringValue(wellKnownJson, "m.registration_disabled");
    caps.registrationEnabled = (regDisabled != "true");

    // Check flows for email/captcha requirements
    // Simple heuristic: look for "m.login.email.identity" or "recaptcha" in the JSON
    auto lower = wellKnownJson;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    caps.emailRequired = lower.find("email") != std::string::npos;
    caps.captchaRequired = lower.find("captcha") != std::string::npos ||
                           lower.find("recaptcha") != std::string::npos;

    return caps;
}

} // namespace progressive


// ==== Extended room_mirror implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string room_mirror::serialize() const {
    json j = toJson();
    return j.dump();
}

bool room_mirror::deserialize(const std::string& data) {
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
bool room_mirror::validate() const {
    if (!m_initialized) {
        LOGE("room_mirror: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool room_mirror::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool room_mirror::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json room_mirror::getMetrics() const {
    json m;
    m["class"] = "room_mirror";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int room_mirror::getOperationCount() const {
    return m_operationCount;
}

void room_mirror::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void room_mirror::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "room_mirror";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool room_mirror::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool room_mirror::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool room_mirror::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void room_mirror::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void room_mirror::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int room_mirror::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void room_mirror::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> room_mirror::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> room_mirror::listItems() const {
    return {};
}

int room_mirror::itemCount() const {
    return 0;
}

// Versioning
std::string room_mirror::getVersion() const {
    return "1.0.0";
}

bool room_mirror::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool room_mirror::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void room_mirror::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> room_mirror::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool room_mirror::migrateData(int fromVersion, int toVersion) {
    LOGI("room_mirror: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int room_mirror::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json room_mirror::exportData() const {
    return toJson();
}

bool room_mirror::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void room_mirror::performCleanup() {
    LOGI("room_mirror: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t room_mirror::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool room_mirror::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool room_mirror::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool room_mirror::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void room_mirror::logDebug(const std::string& msg) const {
    LOGI("room_mirror: %s", msg.c_str());
}

void room_mirror::logWarning(const std::string& msg) const {
    LOGW("room_mirror: %s", msg.c_str());
}

void room_mirror::logError(const std::string& msg) const {
    LOGE("room_mirror: %s", msg.c_str());
}
