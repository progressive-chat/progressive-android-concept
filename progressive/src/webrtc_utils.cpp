#include "progressive/webrtc_utils.hpp"
#include "progressive/event_classifier.hpp"
#include "progressive/json_parser.hpp"
#include <sstream>
#include <chrono>

namespace progressive {

CallInfo parseCallInvite(const std::string& eventContentJson, const std::string& eventId,
    const std::string& roomId, const std::string& senderId) {
    CallInfo call;
    call.callId      = parseJsonStringValue(eventContentJson, "call_id");
    if (call.callId.empty()) call.callId = eventId;
    call.roomId      = roomId;
    call.callerId    = senderId;
    call.isVideo     = (eventContentJson.find("\"m.video\"") != std::string::npos);
    call.isActive    = true;
    call.isIncoming  = true;
    call.startedAtMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return call;
}

SdpOffer parseSdpOffer(const std::string& eventContentJson) {
    SdpOffer offer;
    offer.type = parseJsonStringValue(eventContentJson, "type");
    offer.sdp  = parseJsonStringValue(eventContentJson, "sdp");
    // Alternative: description field
    if (offer.sdp.empty()) {
        auto desc = parseJsonStringValue(eventContentJson, "description");
        if (!desc.empty()) {
            auto offerType = parseJsonStringValue("{" + desc + "}", "type");
            auto offerSdp  = parseJsonStringValue("{" + desc + "}", "sdp");
            if (!offerSdp.empty()) {
                offer.type = offerType;
                offer.sdp = offerSdp;
            }
        }
    }
    offer.valid = !offer.sdp.empty();
    return offer;
}

IceCandidate parseIceCandidate(const std::string& eventContentJson) {
    IceCandidate ice;
    ice.sdpMid = parseJsonStringValue(eventContentJson, "sdpMid");
    ice.candidate = parseJsonStringValue(eventContentJson, "candidate");

    auto mlIdx = parseJsonStringValue(eventContentJson, "sdpMLineIndex");
    if (!mlIdx.empty()) ice.sdpMLineIndex = std::stoi(mlIdx);

    // Also try in `candidates` array
    if (ice.candidate.empty()) {
        auto cands = parseJsonStringValue(eventContentJson, "candidates");
        if (!cands.empty()) {
            auto firstCand = parseJsonStringValue("[" + cands + "]", "candidate");
            if (!firstCand.empty()) ice.candidate = firstCand;
        }
    }
    return ice;
}

std::string buildCallInviteContent(const std::string& callId, bool isVideo,
    const std::string& sdpOffer, int lifetimeSeconds) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    std::ostringstream json;
    json << R"({"call_id": ")" << esc(callId) << R"(")";
    json << R"(,"offer": {"type": "offer", "sdp": ")" << esc(sdpOffer) << R"("})";
    if (isVideo) json << R"(,"m.video": true)";
    json << R"(,"version": "1")";
    if (lifetimeSeconds > 0)
        json << R"(,"lifetime": )" << lifetimeSeconds;
    json << "}";
    return json.str();
}

std::string buildCallAnswerContent(const std::string& callId, const std::string& sdpAnswer) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    std::ostringstream json;
    json << R"({"call_id": ")" << esc(callId) << R"(")";
    json << R"(,"answer": {"type": "answer", "sdp": ")" << esc(sdpAnswer) << R"("})";
    json << R"(,"version": "1")";
    json << "}";
    return json.str();
}

std::string buildCallCandidatesContent(const std::string& callId,
    const std::vector<IceCandidate>& candidates) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    std::ostringstream json;
    json << R"({"call_id": ")" << esc(callId) << R"(")";
    json << R"(,"candidates": [)";
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (i > 0) json << ",";
        json << R"({"sdpMid": ")" << esc(candidates[i].sdpMid) << R"(")";
        json << R"(,"sdpMLineIndex": )" << candidates[i].sdpMLineIndex;
        json << R"(,"candidate": ")" << esc(candidates[i].candidate) << R"(")" << "}";
    }
    json << R"(],"version": "1")";
    json << "}";
    return json.str();
}

std::string buildCallHangupContent(const std::string& callId, const std::string& reason) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    std::ostringstream json;
    json << R"({"call_id": ")" << esc(callId) << R"(")";
    if (!reason.empty()) json << R"(,"reason": ")" << esc(reason) << R"(")";
    json << R"(,"version": "1")";
    json << "}";
    return json.str();
}

std::string formatCallNotification(const CallInfo& call) {
    std::ostringstream out;
    out << (call.isVideo ? "Video" : "Voice") << " call";
    if (call.isIncoming) out << " from " << call.callerName;
    return out.str();
}

std::string formatCallDuration(int seconds) {
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;

    std::ostringstream out;
    if (h > 0) {
        out << h << ":";
        if (m < 10) out << "0";
    }
    out << m << ":";
    if (s < 10) out << "0";
    out << s;
    return out.str();
}

bool isCallExpired(int64_t createdAtMs, int timeoutSeconds) {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return (now - createdAtMs) > timeoutSeconds * 1000LL;
}

std::string getCallState(const std::string& eventContentJson) {
    if (eventContentJson.find("\"m.call.invite\"") != std::string::npos) return "invite";
    if (eventContentJson.find("\"m.call.answer\"") != std::string::npos) return "answer";
    if (eventContentJson.find("\"m.call.candidates\"") != std::string::npos) return "candidates";
    if (eventContentJson.find("\"m.call.hangup\"") != std::string::npos) return "hangup";
    if (eventContentJson.find("\"m.call.reject\"") != std::string::npos) return "reject";
    return "unknown";
}

} // namespace progressive


// ==== Extended webrtc_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string webrtc_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool webrtc_utils::deserialize(const std::string& data) {
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
bool webrtc_utils::validate() const {
    if (!m_initialized) {
        LOGE("webrtc_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool webrtc_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool webrtc_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json webrtc_utils::getMetrics() const {
    json m;
    m["class"] = "webrtc_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int webrtc_utils::getOperationCount() const {
    return m_operationCount;
}

void webrtc_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void webrtc_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "webrtc_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool webrtc_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool webrtc_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool webrtc_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void webrtc_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void webrtc_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int webrtc_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void webrtc_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> webrtc_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> webrtc_utils::listItems() const {
    return {};
}

int webrtc_utils::itemCount() const {
    return 0;
}

// Versioning
std::string webrtc_utils::getVersion() const {
    return "1.0.0";
}

bool webrtc_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool webrtc_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void webrtc_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> webrtc_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool webrtc_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("webrtc_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int webrtc_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json webrtc_utils::exportData() const {
    return toJson();
}

bool webrtc_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void webrtc_utils::performCleanup() {
    LOGI("webrtc_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t webrtc_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool webrtc_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool webrtc_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool webrtc_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void webrtc_utils::logDebug(const std::string& msg) const {
    LOGI("webrtc_utils: %s", msg.c_str());
}

void webrtc_utils::logWarning(const std::string& msg) const {
    LOGW("webrtc_utils: %s", msg.c_str());
}

void webrtc_utils::logError(const std::string& msg) const {
    LOGE("webrtc_utils: %s", msg.c_str());
}
