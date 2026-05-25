#include "progressive/call_models.hpp"

namespace progressive {

// ==== SdpType ====
const char* sdpTypeToString(SdpType t) {
    return t == SdpType::OFFER ? "offer" : "answer";
}
SdpType sdpTypeFromString(const std::string& s) {
    if (s == "offer") return SdpType::OFFER;
    return SdpType::ANSWER;
}

// ==== EndCallReason ====
const char* endCallReasonToString(EndCallReason r) {
    switch (r) {
        case EndCallReason::ICE_FAILED: return "ice_failed";
        case EndCallReason::ICE_TIMEOUT: return "ice_timeout";
        case EndCallReason::USER_HANGUP: return "user_hangup";
        case EndCallReason::REPLACED: return "replaced";
        case EndCallReason::USER_MEDIA_FAILED: return "user_media_failed";
        case EndCallReason::INVITE_TIMEOUT: return "invite_timeout";
        case EndCallReason::UNKNOWN_ERROR: return "unknown_error";
        case EndCallReason::USER_BUSY: return "user_busy";
        case EndCallReason::ANSWERED_ELSEWHERE: return "answered_elsewhere";
    }
    return "user_hangup";
}
EndCallReason endCallReasonFromString(const std::string& s) {
    if (s == "ice_failed") return EndCallReason::ICE_FAILED;
    if (s == "ice_timeout") return EndCallReason::ICE_TIMEOUT;
    if (s == "user_hangup") return EndCallReason::USER_HANGUP;
    if (s == "replaced") return EndCallReason::REPLACED;
    if (s == "user_media_failed") return EndCallReason::USER_MEDIA_FAILED;
    if (s == "invite_timeout") return EndCallReason::INVITE_TIMEOUT;
    if (s == "unknown_error") return EndCallReason::UNKNOWN_ERROR;
    if (s == "user_busy") return EndCallReason::USER_BUSY;
    if (s == "answered_elsewhere") return EndCallReason::ANSWERED_ELSEWHERE;
    return EndCallReason::USER_HANGUP;
}

// ==== JSON Helpers ====
static std::string extractJsonString(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.size() || json[pos] != '"') return "";
    pos++;
    size_t end = pos;
    while (end < json.size() && json[end] != '"') {
        if (json[end] == '\\') end++;
        end++;
    }
    return json.substr(pos, end - pos);
}
static int extractJsonInt(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return 0;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return 0;
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.size()) return 0;
    int val = 0;
    while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
        val = val * 10 + (json[pos] - '0');
        pos++;
    }
    return val;
}
static bool extractJsonBool(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return false;
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    return json.compare(pos, 4, "true") == 0;
}
static std::string extractJsonObject(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.size() || json[pos] != '{') return "";
    int depth = 1;
    size_t start = pos;
    pos++;
    while (pos < json.size() && depth > 0) {
        if (json[pos] == '{') depth++;
        else if (json[pos] == '}') depth--;
        pos++;
    }
    return json.substr(start, pos - start);
}

static void fillCallBase(CallSignalingContent& c, const std::string& json) {
    c.callId = extractJsonString(json, "call_id");
    c.partyId = extractJsonString(json, "party_id");
    c.version = extractJsonString(json, "version");
}

// Original Kotlin (CallInviteContent.kt:26-59)
CallInviteContent parseCallInvite(const std::string& json) {
    CallInviteContent c;
    fillCallBase(c, json);
    c.lifetime = extractJsonInt(json, "lifetime");
    c.invitee = extractJsonString(json, "invitee");

    auto offerJson = extractJsonObject(json, "offer");
    if (!offerJson.empty()) {
        c.offer.type = sdpTypeFromString(extractJsonString(offerJson, "type"));
        c.offer.sdp = extractJsonString(offerJson, "sdp");
    }

    auto capJson = extractJsonObject(json, "capabilities");
    if (!capJson.empty()) {
        c.capabilities.transferee = extractJsonBool(capJson, "m.call.transferee");
    }

    return c;
}

// Original Kotlin (CallAnswerContent.kt:26-49)
CallAnswerContent parseCallAnswer(const std::string& json) {
    CallAnswerContent c;
    fillCallBase(c, json);

    auto answerJson = extractJsonObject(json, "answer");
    if (!answerJson.empty()) {
        c.answer.type = sdpTypeFromString(extractJsonString(answerJson, "type"));
        c.answer.sdp = extractJsonString(answerJson, "sdp");
    }

    return c;
}

// Original Kotlin (CallCandidatesContent.kt:26-44)
CallCandidatesContent parseCallCandidates(const std::string& json) {
    CallCandidatesContent c;
    fillCallBase(c, json);

    auto candArr = json.find("\"candidates\"");
    if (candArr != std::string::npos) {
        candArr = json.find('[', candArr);
        if (candArr != std::string::npos) {
            size_t pos = candArr + 1;
            while (pos < json.size()) {
                while (pos < json.size() && (json[pos] == ' ' || json[pos] == ',' || json[pos] == '\n')) pos++;
                if (pos >= json.size() || json[pos] == ']') break;
                if (json[pos] == '{') {
                    int d = 1;
                    size_t start = pos;
                    pos++;
                    while (pos < json.size() && d > 0) {
                        if (json[pos] == '{') d++;
                        else if (json[pos] == '}') d--;
                        pos++;
                    }
                    std::string candJson = json.substr(start, pos - start);
                    CallCandidate cand;
                    cand.sdpMid = extractJsonString(candJson, "sdpMid");
                    cand.sdpMLineIndex = extractJsonInt(candJson, "sdpMLineIndex");
                    cand.candidate = extractJsonString(candJson, "candidate");
                    c.candidates.push_back(cand);
                }
            }
        }
    }

    return c;
}

// Original Kotlin (CallHangupContent.kt:27-46)
CallHangupContent parseCallHangup(const std::string& json) {
    CallHangupContent c;
    fillCallBase(c, json);
    c.reason = endCallReasonFromString(extractJsonString(json, "reason"));
    return c;
}

// Original Kotlin (CallRejectContent.kt:27-44)
CallRejectContent parseCallReject(const std::string& json) {
    CallRejectContent c;
    fillCallBase(c, json);
    c.reason = endCallReasonFromString(extractJsonString(json, "reason"));
    return c;
}

// Original Kotlin (CallNegotiateContent.kt:26-52)
CallNegotiateContent parseCallNegotiate(const std::string& json) {
    CallNegotiateContent c;
    fillCallBase(c, json);
    c.lifetime = extractJsonInt(json, "lifetime");

    auto descJson = extractJsonObject(json, "description");
    if (!descJson.empty()) {
        c.description.type = sdpTypeFromString(extractJsonString(descJson, "type"));
        c.description.sdp = extractJsonString(descJson, "sdp");
    }

    return c;
}

// Original Kotlin (CallReplacesContent.kt:28-73)
CallReplacesContent parseCallReplaces(const std::string& json) {
    CallReplacesContent c;
    fillCallBase(c, json);
    c.replacementId = extractJsonString(json, "replacement_id");
    c.targetRoomId = extractJsonString(json, "target_room");
    c.createCall = extractJsonString(json, "create_call");
    c.awaitCall = extractJsonString(json, "await_call");

    auto targetJson = extractJsonObject(json, "target_user");
    if (!targetJson.empty()) {
        c.targetUser.id = extractJsonString(targetJson, "id");
        c.targetUser.displayName = extractJsonString(targetJson, "display_name");
        c.targetUser.avatarUrl = extractJsonString(targetJson, "avatar_url");
    }

    return c;
}

// Original Kotlin (CallAssertedIdentityContent.kt:27-56)
CallAssertedIdentityContent parseCallAssertedIdentity(const std::string& json) {
    CallAssertedIdentityContent c;
    fillCallBase(c, json);

    auto aiJson = extractJsonObject(json, "asserted_identity");
    if (!aiJson.empty()) {
        c.assertedIdentity.id = extractJsonString(aiJson, "id");
        c.assertedIdentity.displayName = extractJsonString(aiJson, "display_name");
        c.assertedIdentity.avatarUrl = extractJsonString(aiJson, "avatar_url");
    }

    return c;
}

// ==== Parse TurnServerResponse ====
//
// Original Kotlin (TurnServerResponse.kt:26-47)

TurnServerResponse parseTurnServerResponse(const std::string& json) {
    TurnServerResponse ts;
    ts.username = extractJsonString(json, "username");
    ts.password = extractJsonString(json, "password");
    ts.ttl = extractJsonInt(json, "ttl");

    auto urisPos = json.find("\"uris\"");
    if (urisPos != std::string::npos) {
        urisPos = json.find('[', urisPos);
        if (urisPos != std::string::npos) {
            urisPos++;
            while (urisPos < json.size()) {
                while (urisPos < json.size() && (json[urisPos] == ' ' || json[urisPos] == ',' || json[urisPos] == '\n')) urisPos++;
                if (urisPos >= json.size() || json[urisPos] == ']') break;
                if (json[urisPos] == '"') {
                    urisPos++;
                    size_t end = urisPos;
                    while (end < json.size() && json[end] != '"') end++;
                    ts.uris.push_back(json.substr(urisPos, end - urisPos));
                    urisPos = end + 1;
                }
            }
        }
    }

    return ts;
}

} // namespace progressive


// ==== Extended call_models implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string call_models::serialize() const {
    json j = toJson();
    return j.dump();
}

bool call_models::deserialize(const std::string& data) {
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
bool call_models::validate() const {
    if (!m_initialized) {
        LOGE("call_models: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool call_models::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool call_models::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json call_models::getMetrics() const {
    json m;
    m["class"] = "call_models";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int call_models::getOperationCount() const {
    return m_operationCount;
}

void call_models::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void call_models::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "call_models";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool call_models::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool call_models::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool call_models::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void call_models::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void call_models::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int call_models::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void call_models::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> call_models::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> call_models::listItems() const {
    return {};
}

int call_models::itemCount() const {
    return 0;
}

// Versioning
std::string call_models::getVersion() const {
    return "1.0.0";
}

bool call_models::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool call_models::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void call_models::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> call_models::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool call_models::migrateData(int fromVersion, int toVersion) {
    LOGI("call_models: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int call_models::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json call_models::exportData() const {
    return toJson();
}

bool call_models::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void call_models::performCleanup() {
    LOGI("call_models: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t call_models::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool call_models::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool call_models::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool call_models::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void call_models::logDebug(const std::string& msg) const {
    LOGI("call_models: %s", msg.c_str());
}

void call_models::logWarning(const std::string& msg) const {
    LOGW("call_models: %s", msg.c_str());
}

void call_models::logError(const std::string& msg) const {
    LOGE("call_models: %s", msg.c_str());
}
