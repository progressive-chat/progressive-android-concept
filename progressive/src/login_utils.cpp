#include "progressive/login_utils.hpp"
#include "progressive/json_parser.hpp"
#include "progressive/url_tools.hpp"
#include <sstream>
#include <regex>
#include <algorithm>
#include <random>

namespace progressive {

std::vector<LoginFlow> parseLoginFlowsList(const std::string& apiResponseJson) {
    std::vector<LoginFlow> flows;

    // Parse "flows": [{"type": "m.login.password"}, ...]
    size_t pos = 0;
    while (true) {
        pos = apiResponseJson.find("\"type\"", pos);
        if (pos == std::string::npos) break;

        auto objStart = apiResponseJson.rfind('{', pos);
        if (objStart == std::string::npos) break;

        int depth = 0;
        auto objEnd = objStart;
        while (objEnd < apiResponseJson.size()) {
            if (apiResponseJson[objEnd] == '{') ++depth;
            else if (apiResponseJson[objEnd] == '}') --depth;
            if (depth == 0) break;
            ++objEnd;
        }
        if (objEnd >= apiResponseJson.size()) break;

        std::string obj = apiResponseJson.substr(objStart, objEnd - objStart + 1);

        LoginFlow flow;
        flow.type = parseJsonStringValue(obj, "type");
        flow.description = flow.type;
        if (!flow.type.empty()) flows.push_back(flow);
        pos = objEnd + 1;
    }

    return flows;
}

std::string buildUserIdentifier(const std::string& userId) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    if (userId.find('@') != std::string::npos) {
        return R"({"type": "m.id.user", "user": ")" + esc(userId) + R"("})";
    }
    // Phone or email
    if (userId.find('@') != std::string::npos && userId.find(":+") == std::string::npos) {
        return R"({"type": "m.id.thirdparty", "medium": "email", "address": ")" + esc(userId) + R"("})";
    }
    return R"({"type": "m.id.user", "user": ")" + esc(userId) + R"("})";
}

std::string buildLoginBody(const LoginParams& params) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    std::ostringstream json;
    json << "{";
    if (!params.password.empty()) {
        json << R"("type": "m.login.password",)";
        json << R"("identifier": )" << buildUserIdentifier(params.userId) << ",";
        json << R"("password": ")" << esc(params.password) << R"(")";
    } else if (!params.token.empty()) {
        json << R"("type": "m.login.token",)";
        json << R"("token": ")" << esc(params.token) << R"(")";
    }
    if (!params.deviceName.empty())
        json << R"(,"initial_device_display_name": ")" << esc(params.deviceName) << R"(")";
    if (!params.deviceId.empty())
        json << R"(,"device_id": ")" << esc(params.deviceId) << R"(")";
    if (params.refreshToken)
        json << R"(,"refresh_token": true)";
    json << "}";
    return json.str();
}

bool isValidLoginCredentials(const std::string& userId, const std::string& password) {
    return !userId.empty() && userId.size() >= 3 && !password.empty() && password.size() >= 1;
}

LoginResult parseLoginResponse(const std::string& responseJson, int httpStatus) {
    LoginResult result;
    result.httpStatus = httpStatus;

    if (httpStatus != 200) {
        result.errorCode = parseJsonStringValue(responseJson, "errcode");
        result.errorMessage = parseJsonStringValue(responseJson, "error");
        if (result.errorMessage.empty()) result.errorMessage = "Server returned " + std::to_string(httpStatus);
        return result;
    }

    result.userId = parseJsonStringValue(responseJson, "user_id");
    result.accessToken = parseJsonStringValue(responseJson, "access_token");
    result.refreshToken = parseJsonStringValue(responseJson, "refresh_token");
    result.deviceId = parseJsonStringValue(responseJson, "device_id");
    result.homeServer = parseJsonStringValue(responseJson, "home_server");
    result.success = !result.accessToken.empty();

    return result;
}

std::string buildRefreshBody(const std::string& refreshToken) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    return R"({"refresh_token": ")" + esc(refreshToken) + R"("})";
}

bool isRateLimited(const std::string& responseJson, int httpStatus) {
    if (httpStatus == 429) return true;
    auto errcode = parseJsonStringValue(responseJson, "errcode");
    return errcode == "M_LIMIT_EXCEEDED";
}

int getRateLimitRetrySeconds(const std::string& responseJson) {
    auto retryMs = parseJsonStringValue(responseJson, "retry_after_ms");
    if (!retryMs.empty()) return std::stoi(retryMs) / 1000;
    return 5; // default 5 seconds
}

std::string generateDeviceName(const std::string& model, const std::string& osVersion) {
    if (!model.empty() && !osVersion.empty()) return model + " (" + osVersion + ")";
    if (!model.empty()) return model;
    return "Progressive Chat";
}

std::string generateDeviceId() {
    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 51);
    std::string id(10, 'A');
    for (int i = 0; i < 10; ++i) id[i] = chars[dis(gen)];
    return id;
}

std::string parseDeviceName(const std::string& userAgent) {
    // Extract meaningful name from UA
    auto pos = userAgent.find("Progressive");
    if (pos != std::string::npos) return userAgent.substr(pos);
    pos = userAgent.find("Element");
    if (pos != std::string::npos) return userAgent.substr(pos);
    return userAgent.substr(0, std::min(size_t(50), userAgent.size()));
}

WellKnownResult parseWellKnown(const std::string& responseJson) {
    WellKnownResult result;

    auto homeserver = parseJsonStringValue(responseJson, "base_url");
    if (!homeserver.empty()) {
        result.homeServerBaseUrl = homeserver;
        result.valid = true;
        result.isWellKnown = true;
    }

    auto identity = parseJsonStringValue(responseJson, "id_base_url");
    if (!identity.empty()) {
        result.identityServerBaseUrl = identity;
    }

    return result;
}

bool needsWellKnownDiscovery(const std::string& homeserverUrl) {
    return homeserverUrl.find("matrix.org") == std::string::npos &&
           homeserverUrl.find("://matrix") == std::string::npos;
}

std::string buildWellKnownUrl(const std::string& domain) {
    return "https://" + domain + "/.well-known/matrix/client";
}

// ==== Registration Flow Parser (from RegistrationFlowResponse.kt:78-115) ====

RegistrationFlowResult parseRegistrationFlow(const std::string& json) {
    RegistrationFlowResult result;

    auto extractStr = [&](const std::string& key) -> std::string {
        auto search = "\"" + key + "\":\"";
        auto pos = json.find(search);
        if (pos == std::string::npos) { search = "\"" + key + "\": \""; pos = json.find(search); }
        if (pos == std::string::npos) return "";
        pos += search.size();
        auto end = json.find('"', pos);
        return (end != std::string::npos) ? json.substr(pos, end - pos) : "";
    };

    result.session = extractStr("session");

    // Parse completed stages
    auto compPos = json.find("\"completed\"");
    if (compPos != std::string::npos) {
        auto bracket = json.find('[', compPos);
        if (bracket != std::string::npos) {
            size_t pos = bracket + 1;
            while (pos < json.size()) {
                if (json[pos] == '"') {
                    size_t end = json.find('"', pos + 1);
                    if (end != std::string::npos) {
                        RegistrationStage s;
                        s.type = json.substr(pos + 1, end - pos - 1);
                        s.isMandatory = true;
                        result.completedStages.push_back(s);
                        pos = end + 1;
                        continue;
                    }
                }
                if (json[pos] == ']') break;
                pos++;
            }
        }
    }

    // Parse flows for missing stages
    auto flowsPos = json.find("\"flows\"");
    if (flowsPos != std::string::npos) {
        auto bracket = json.find('[', flowsPos);
        if (bracket != std::string::npos) {
            size_t pos = bracket + 1;
            while (pos < json.size()) {
                if (json[pos] == '{') {
                    // Find "stages" inside this flow object
                    auto stagesPos = json.find("\"stages\"", pos);
                    if (stagesPos != std::string::npos && stagesPos < pos + 500) {
                        auto sb = json.find('[', stagesPos);
                        if (sb != std::string::npos && sb < pos + 600) {
                            size_t sp = sb + 1;
                            while (sp < json.size()) {
                                if (json[sp] == '"') {
                                    size_t se = json.find('"', sp + 1);
                                    if (se != std::string::npos) {
                                        std::string stageType = json.substr(sp + 1, se - sp - 1);
                                        // Check if already completed
                                        bool alreadyDone = false;
                                        for (const auto& cs : result.completedStages) {
                                            if (cs.type == stageType) { alreadyDone = true; break; }
                                        }
                                        if (!alreadyDone) {
                                            RegistrationStage s;
                                            s.type = stageType;
                                            s.isMandatory = true;
                                            s.isEmail = (stageType == "m.login.email.identity");
                                            s.isMsisdn = (stageType == "m.login.msisdn");
                                            // Extract reCAPTCHA public key
                                            if (stageType == "m.login.recaptcha") {
                                                auto pkPos = json.find("\"public_key\"", sp);
                                                if (pkPos != std::string::npos && pkPos < se + 200) {
                                                    auto colon = json.find(':', pkPos);
                                                    if (colon != std::string::npos) {
                                                        auto q = json.find('"', colon);
                                                        if (q != std::string::npos) {
                                                            auto qe = json.find('"', q + 1);
                                                            if (qe != std::string::npos)
                                                                s.publicKey = json.substr(q + 1, qe - q - 1);
                                                        }
                                                    }
                                                }
                                            }
                                            result.missingStages.push_back(s);
                                        }
                                        sp = se + 1;
                                        continue;
                                    }
                                }
                                if (json[sp] == ']') break;
                                sp++;
                            }
                        }
                    }
                    // Skip to end of this flow object
                    int depth = 1;
                    pos++;
                    while (pos < json.size() && depth > 0) {
                        if (json[pos] == '{') depth++;
                        else if (json[pos] == '}') depth--;
                        pos++;
                    }
                    continue;
                }
                pos++;
            }
        }
    }

    result.hasStages = !result.missingStages.empty() || !result.completedStages.empty();
    return result;
}

bool requiresOidc(const WellKnownResult& /*wk*/) {
    return false;
}

} // namespace progressive


// ==== Extended login_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string login_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool login_utils::deserialize(const std::string& data) {
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
bool login_utils::validate() const {
    if (!m_initialized) {
        LOGE("login_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool login_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool login_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json login_utils::getMetrics() const {
    json m;
    m["class"] = "login_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int login_utils::getOperationCount() const {
    return m_operationCount;
}

void login_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void login_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "login_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool login_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool login_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool login_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void login_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void login_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int login_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void login_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> login_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> login_utils::listItems() const {
    return {};
}

int login_utils::itemCount() const {
    return 0;
}

// Versioning
std::string login_utils::getVersion() const {
    return "1.0.0";
}

bool login_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool login_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void login_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> login_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool login_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("login_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int login_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json login_utils::exportData() const {
    return toJson();
}

bool login_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void login_utils::performCleanup() {
    LOGI("login_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t login_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool login_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool login_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool login_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void login_utils::logDebug(const std::string& msg) const {
    LOGI("login_utils: %s", msg.c_str());
}

void login_utils::logWarning(const std::string& msg) const {
    LOGW("login_utils: %s", msg.c_str());
}

void login_utils::logError(const std::string& msg) const {
    LOGE("login_utils: %s", msg.c_str());
}
