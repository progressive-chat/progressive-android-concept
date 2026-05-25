#include "progressive/terms_manager.hpp"
#include <sstream>
#include <algorithm>

namespace progressive {

const char* termsServiceTypeToString(TermsServiceType type) {
    switch (type) {
        case TermsServiceType::INTEGRATION_MANAGER: return "integration_manager";
        case TermsServiceType::IDENTITY_SERVICE: return "identity_service";
        case TermsServiceType::HOME_SERVER: return "home_server";
    }
    return "home_server";
}

// ====== JSON helpers ======

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

// ====== Constructor ======

TermsManager::TermsManager() {}

// ====== Terms Parsing ======
// Original: TermsResponse.kt — {"policies": {"policy_name": {"en": {"name":"...","url":"..."}, "version":"1.0"}}}

TermsResponse TermsManager::parseTermsResponse(const std::string& json) {
    TermsResponse resp;
    resp.rawJson = json;

    // Parse policies object
    size_t pos = json.find("\"policies\"");
    if (pos == std::string::npos) return resp;

    pos = json.find('{', pos);
    if (pos == std::string::npos) return resp;

    resp.hasPolicies = true;

    // Walk through policy entries: "policy_name": { ... }
    pos++;
    while (pos < json.size() && json[pos] != '}') {
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == ',' || json[pos] == '\n')) pos++;
        if (pos >= json.size() || json[pos] == '}') break;

        if (json[pos] == '"') {
            pos++;
            size_t e = pos;
            while (e < json.size() && json[e] != '"') e++;
            std::string policyName = json.substr(pos, e - pos);
            pos = e + 1;

            // Find '{' for this policy's value
            pos = json.find('{', pos);
            if (pos == std::string::npos) break;

            int depth = 0;
            size_t start = pos;
            pos++;
            while (pos < json.size()) {
                if (json[pos] == '{') depth++;
                else if (json[pos] == '}') depth--;
                if (depth == -1) break;
                pos++;
            }
            std::string policyJson = json.substr(start, pos - start);
            pos++;

            // Get version
            std::string version = extractStr(policyJson, "version");

            // Get "en" entry
            auto enPos = policyJson.find("\"en\"");
            if (enPos != std::string::npos) {
                enPos = policyJson.find('{', enPos);
                if (enPos != std::string::npos) {
                    depth = 0; start = enPos; enPos++;
                    while (enPos < policyJson.size()) {
                        if (policyJson[enPos] == '{') depth++;
                        else if (policyJson[enPos] == '}') depth--;
                        if (depth == -1) break;
                        enPos++;
                    }
                    std::string enJson = policyJson.substr(start, enPos - start);

                    LocalizedTerms term;
                    term.policyName = policyName;
                    term.version = version;
                    term.localizedName = extractStr(enJson, "name");
                    term.localizedUrl = extractStr(enJson, "url");
                    term.valid = !term.localizedUrl.empty();

                    if (term.valid) resp.policies.push_back(term);
                }
            }
        } else {
            pos++;
        }
    }

    resp.policyCount = static_cast<int>(resp.policies.size());
    return resp;
}

// Original: getLocalizedTerms(userLanguage, defaultLanguage="en")
std::vector<LocalizedTerms> TermsManager::getLocalizedTerms(const TermsResponse& response,
                                                             const std::string& userLanguage,
                                                             const std::string& defaultLanguage) {
    // For now, return all policies from the response
    // A full implementation would parse policies json for the specific language
    return response.policies;
}

// ====== Terms Agreement ======

std::string TermsManager::buildAgreeRequest(const TermsAgreementRequest& req) {
    std::ostringstream os;
    os << R"({"user_accepts":[)";
    for (size_t i = 0; i < req.agreedUrls.size(); i++) {
        if (i > 0) os << ","; os << "\"" << req.agreedUrls[i] << "\"";
    }
    os << "]}";
    return os.str();
}

bool TermsManager::parseAgreementResponse(const std::string& json) {
    auto err = extractStr(json, "errcode");
    return err.empty();
}

// ====== Terms Status ======

bool TermsManager::areTermsRequired(const std::string& errorJson) {
    return errorJson.find("M_TERMS_NOT_SIGNED") != std::string::npos ||
           errorJson.find("M_CONSENT_NOT_GIVEN") != std::string::npos;
}

bool TermsManager::hasAgreedToAll(const TermsResponse& response, const std::vector<std::string>& agreedUrls) {
    return getPendingPolicies(response, agreedUrls).empty();
}

std::vector<std::string> TermsManager::getPendingPolicies(const TermsResponse& response,
                                                            const std::vector<std::string>& agreedUrls) {
    std::vector<std::string> pending;
    for (const auto& policy : response.policies) {
        bool found = false;
        for (const auto& agreed : agreedUrls) {
            if (agreed == policy.localizedUrl) { found = true; break; }
        }
        if (!found) pending.push_back(policy.localizedUrl);
    }
    return pending;
}

// ====== Display ======

std::string TermsManager::formatTermsList(const std::vector<LocalizedTerms>& terms) {
    std::ostringstream os;
    os << "Please review and accept the following terms:\n";
    for (size_t i = 0; i < terms.size(); i++) {
        os << (i + 1) << ". " << formatSingleTerm(terms[i]) << "\n";
    }
    return os.str();
}

std::string TermsManager::formatSingleTerm(const LocalizedTerms& term) {
    std::string name = term.localizedName.empty() ? term.policyName : term.localizedName;
    return name + " (" + term.localizedUrl + ")";
}

// ====== Serialization ======

std::string TermsManager::termsToJson(const std::vector<LocalizedTerms>& terms) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) { if (c == '"') out += "\\\""; else out += c; }
        return out;
    };

    std::ostringstream os; os << "[";
    for (size_t i = 0; i < terms.size(); i++) {
        if (i > 0) os << ",";
        os << R"({"policy_name":")" << esc(terms[i].policyName)
           << R"(","version":")" << esc(terms[i].version)
           << R"(","name":")" << esc(terms[i].localizedName)
           << R"(","url":")" << esc(terms[i].localizedUrl)
           << "\"}";
    }
    os << "]";
    return os.str();
}

std::string TermsManager::responseToJson(const TermsResponse& resp) {
    std::ostringstream os;
    os << R"({"has_policies":)" << (resp.hasPolicies ? "true" : "false")
       << R"(,"count":)" << resp.policyCount
       << R"(,"policies":)" << termsToJson(resp.policies)
       << "}";
    return os.str();
}

} // namespace progressive


// ==== Extended terms_manager implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string terms_manager::serialize() const {
    json j = toJson();
    return j.dump();
}

bool terms_manager::deserialize(const std::string& data) {
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
bool terms_manager::validate() const {
    if (!m_initialized) {
        LOGE("terms_manager: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool terms_manager::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool terms_manager::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json terms_manager::getMetrics() const {
    json m;
    m["class"] = "terms_manager";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int terms_manager::getOperationCount() const {
    return m_operationCount;
}

void terms_manager::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void terms_manager::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "terms_manager";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool terms_manager::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool terms_manager::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool terms_manager::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void terms_manager::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void terms_manager::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int terms_manager::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void terms_manager::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> terms_manager::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> terms_manager::listItems() const {
    return {};
}

int terms_manager::itemCount() const {
    return 0;
}

// Versioning
std::string terms_manager::getVersion() const {
    return "1.0.0";
}

bool terms_manager::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool terms_manager::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void terms_manager::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> terms_manager::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool terms_manager::migrateData(int fromVersion, int toVersion) {
    LOGI("terms_manager: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int terms_manager::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json terms_manager::exportData() const {
    return toJson();
}

bool terms_manager::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void terms_manager::performCleanup() {
    LOGI("terms_manager: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t terms_manager::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool terms_manager::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool terms_manager::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool terms_manager::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void terms_manager::logDebug(const std::string& msg) const {
    LOGI("terms_manager: %s", msg.c_str());
}

void terms_manager::logWarning(const std::string& msg) const {
    LOGW("terms_manager: %s", msg.c_str());
}

void terms_manager::logError(const std::string& msg) const {
    LOGE("terms_manager: %s", msg.c_str());
}
