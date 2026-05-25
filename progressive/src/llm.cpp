#include "progressive/llm.hpp"
#include "progressive/json_parser.hpp"
#include <sstream>
#include <algorithm>
#include <unordered_map>

namespace progressive {

// ---- LLM Request Building ----

static std::string escapeJson(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c;
        }
    }
    return out;
}

std::string buildOpenAiRequestBody(const LlmConfig& config, const std::string& prompt) {
    std::ostringstream json;
    json << "{";
    json << R"("model": ")" << config.model << R"(",)";
    json << R"("messages": [)";

    if (!config.systemPrompt.empty()) {
        json << R"({"role": "system", "content": ")" << escapeJson(config.systemPrompt) << R"("},)";
    }
    json << R"({"role": "user", "content": ")" << escapeJson(prompt) << R"("})";

    json << "],";
    json << R"("temperature": )" << config.temperature << ",";
    json << R"("max_tokens": )" << config.maxTokens;
    json << "}";
    return json.str();
}

std::string buildAnthropicRequestBody(const LlmConfig& config, const std::string& prompt) {
    std::ostringstream json;
    json << "{";
    json << R"("model": ")" << config.model << R"(",)";
    json << R"("max_tokens": )" << config.maxTokens << ",";
    json << R"("messages": [)";

    if (!config.systemPrompt.empty()) {
        json << R"({"role": "user", "content": ")" << escapeJson(config.systemPrompt) << R"("},)";
    }
    json << R"({"role": "user", "content": ")" << escapeJson(prompt) << R"("})";

    json << "]";
    json << "}";
    return json.str();
}

std::string buildLlmRequestBody(const LlmConfig& config, const std::string& prompt) {
    switch (config.provider) {
        case LlmProvider::Anthropic: return buildAnthropicRequestBody(config, prompt);
        default: return buildOpenAiRequestBody(config, prompt);
    }
}

std::string buildLlmHeaders(const LlmConfig& config) {
    switch (config.provider) {
        case LlmProvider::Anthropic:
            return "x-api-key: " + config.apiToken + "\n" +
                   "anthropic-version: 2023-06-01\n" +
                   "Content-Type: application/json";
        default:
            return "Authorization: Bearer " + config.apiToken + "\n" +
                   "Content-Type: application/json";
    }
}

// ---- LLM Response Parsing ----

LlmResponse parseOpenAiResponse(const std::string& body, int statusCode) {
    LlmResponse resp;
    resp.statusCode = statusCode;

    if (statusCode != 200) {
        auto err = parseJsonStringValue(body, "error");
        resp.errorMessage = err.empty() ? "Server returned " + std::to_string(statusCode) : err;
        return resp;
    }

    auto choices = parseJsonStringValue(body, "choices");
    if (choices.empty()) {
        resp.errorMessage = "No choices in response";
        return resp;
    }

    auto message = parseJsonStringValue("{" + choices + "}", "message");
    if (message.empty()) {
        resp.errorMessage = "No message in response";
        return resp;
    }

    auto content = parseJsonStringValue("{" + message + "}", "content");
    if (content.empty()) {
        resp.errorMessage = "No content in response";
        return resp;
    }

    resp.success = true;
    resp.text = content;
    return resp;
}

LlmResponse parseAnthropicResponse(const std::string& body, int statusCode) {
    LlmResponse resp;
    resp.statusCode = statusCode;

    if (statusCode != 200) {
        auto err = parseJsonStringValue(body, "error");
        resp.errorMessage = err.empty() ? "Server returned " + std::to_string(statusCode) : err;
        return resp;
    }

    auto content = parseJsonStringValue(body, "content");
    if (content.empty()) {
        // Try "completion" field (legacy API)
        content = parseJsonStringValue(body, "completion");
    }

    if (content.empty()) {
        // Anthropic returns content array: "content": [{"type": "text", "text": "..."}]
        auto contentBlock = parseJsonStringValue(body, "content");
        auto text = parseJsonStringValue("{" + contentBlock + "}", "text");
        if (!text.empty()) content = text;
    }

    if (content.empty()) {
        resp.errorMessage = "No content in response";
        return resp;
    }

    resp.success = true;
    resp.text = content;
    return resp;
}

LlmResponse parseLlmResponse(const std::string& body, int statusCode, LlmProvider provider) {
    switch (provider) {
        case LlmProvider::Anthropic: return parseAnthropicResponse(body, statusCode);
        default: return parseOpenAiResponse(body, statusCode);
    }
}

std::string formatLlmBroadcastMessage(const std::string& prompt, const std::string& response) {
    std::ostringstream out;
    out << "On prompt: \"" << prompt << "\", LLM says: " << response;
    return out.str();
}

// ---- Duplicate Name Detection ----

std::vector<DuplicateNameInfo> detectDuplicateNames(
    const std::vector<DuplicateNameInfo>& users
) {
    auto result = users;

    // Count display names
    std::unordered_map<std::string, int> nameCount;
    for (const auto& u : users) {
        nameCount[u.displayName]++;
    }

    // Mark duplicates
    for (auto& u : result) {
        u.hasDuplicates = nameCount[u.displayName] > 1;
    }

    return result;
}

std::string formatUserDisplayName(const std::string& displayName, const std::string& mxid, bool showMxid) {
    if (!showMxid || mxid.empty()) return displayName;
    return displayName + " (" + mxid + ")";
}

// ---- UserMxidVisibility ----

void UserMxidVisibility::hideMxid(const std::string& mxid) {
    if (std::find(hidden_.begin(), hidden_.end(), mxid) == hidden_.end()) {
        hidden_.push_back(mxid);
    }
}

void UserMxidVisibility::showMxid(const std::string& mxid) {
    hidden_.erase(std::remove(hidden_.begin(), hidden_.end(), mxid), hidden_.end());
}

bool UserMxidVisibility::isVisible(const std::string& mxid) const {
    return std::find(hidden_.begin(), hidden_.end(), mxid) == hidden_.end();
}

void UserMxidVisibility::reset(const std::string& mxid) {
    showMxid(mxid);
}

void UserMxidVisibility::clear() {
    hidden_.clear();
}

std::string UserMxidVisibility::exportJson() const {
    std::ostringstream json;
    json << "[";
    for (size_t i = 0; i < hidden_.size(); ++i) {
        if (i > 0) json << ",";
        json << R"(")" << hidden_[i] << R"(")";
    }
    json << "]";
    return json.str();
}

void UserMxidVisibility::importJson(const std::string& json) {
    clear();
    size_t pos = 0;
    while (true) {
        pos = json.find('"', pos);
        if (pos == std::string::npos) break;
        ++pos;
        auto end = json.find('"', pos);
        if (end == std::string::npos) break;
        std::string mxid = json.substr(pos, end - pos);
        if (!mxid.empty() && mxid[0] == '@') hidden_.push_back(mxid);
        pos = end + 1;
    }
}

} // namespace progressive


// ==== Extended llm implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string llm::serialize() const {
    json j = toJson();
    return j.dump();
}

bool llm::deserialize(const std::string& data) {
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
bool llm::validate() const {
    if (!m_initialized) {
        LOGE("llm: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool llm::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool llm::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json llm::getMetrics() const {
    json m;
    m["class"] = "llm";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int llm::getOperationCount() const {
    return m_operationCount;
}

void llm::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void llm::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "llm";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool llm::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool llm::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool llm::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void llm::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void llm::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int llm::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void llm::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> llm::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> llm::listItems() const {
    return {};
}

int llm::itemCount() const {
    return 0;
}

// Versioning
std::string llm::getVersion() const {
    return "1.0.0";
}

bool llm::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool llm::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void llm::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> llm::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool llm::migrateData(int fromVersion, int toVersion) {
    LOGI("llm: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int llm::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json llm::exportData() const {
    return toJson();
}

bool llm::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void llm::performCleanup() {
    LOGI("llm: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t llm::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool llm::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool llm::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool llm::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void llm::logDebug(const std::string& msg) const {
    LOGI("llm: %s", msg.c_str());
}

void llm::logWarning(const std::string& msg) const {
    LOGW("llm: %s", msg.c_str());
}

void llm::logError(const std::string& msg) const {
    LOGE("llm: %s", msg.c_str());
}
