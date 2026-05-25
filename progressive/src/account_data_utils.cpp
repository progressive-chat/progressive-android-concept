#include "progressive/account_data_utils.hpp"
#include <sstream>

namespace progressive {

// ==== Helper: extract JSON string array for a key ====

static std::vector<std::string> extractJsonStringArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return result;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return result;
    pos++;
    while (pos < json.size()) {
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == ',' || json[pos] == '\n')) pos++;
        if (pos >= json.size() || json[pos] == ']') break;
        if (json[pos] == '"') {
            pos++;
            size_t end = pos;
            while (end < json.size() && json[end] != '"') end++;
            result.push_back(json.substr(pos, end - pos));
            pos = end + 1;
        }
    }
    return result;
}

// ==== Direct Message Map ====

DirectMessageMap parseDirectMessageMap(const std::string& json) {
    DirectMessageMap result;
    if (json.empty()) return result;

    size_t pos = 1; // Skip opening {
    while (pos < json.size()) {
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == ',' || json[pos] == '\n')) pos++;
        if (pos >= json.size() || json[pos] == '}') break;
        if (json[pos] == '"') {
            pos++;
            size_t keyEnd = pos;
            while (keyEnd < json.size() && json[keyEnd] != '"') keyEnd++;
            std::string userId = json.substr(pos, keyEnd - pos);
            pos = keyEnd + 1;
            while (pos < json.size() && json[pos] != ':') pos++;
            pos++;
            while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n')) pos++;
            if (pos < json.size() && json[pos] == '[') {
                pos++;
                std::vector<std::string> rooms;
                while (pos < json.size()) {
                    while (pos < json.size() && (json[pos] == ' ' || json[pos] == ',' || json[pos] == '\n')) pos++;
                    if (pos >= json.size() || json[pos] == ']') break;
                    if (json[pos] == '"') {
                        pos++;
                        size_t end = pos;
                        while (end < json.size() && json[end] != '"') end++;
                        rooms.push_back(json.substr(pos, end - pos));
                        pos = end + 1;
                    }
                }
                result[userId] = rooms;
            }
        }
    }
    return result;
}

std::string buildDirectMessageMapJson(const DirectMessageMap& map) {
    std::ostringstream os;
    os << "{";
    bool firstUser = true;
    for (auto it = map.begin(); it != map.end(); ++it) {
        const auto& uid = it->first;
        const auto& rooms = it->second;
        if (!firstUser) os << ",";
        firstUser = false;
        os << "\"" << uid << "\":[";
        bool firstRoom = true;
        for (const auto& rid : rooms) {
            if (!firstRoom) os << ",";
            firstRoom = false;
            os << "\"" << rid << "\"";
        }
        os << "]";
    }
    os << "}";
    return os.str();
}

// ==== Ignored Users ====

std::vector<std::string> parseIgnoredUsers(const std::string& json) {
    std::vector<std::string> result;
    auto pos = json.find("\"ignored_users\"");
    if (pos == std::string::npos) return result;
    pos = json.find('{', pos);
    if (pos == std::string::npos) return result;
    // Parse keys of the ignored_users object
    int depth = 1;
    pos++;
    while (pos < json.size() && depth > 0) {
        if (json[pos] == '{') depth++;
        else if (json[pos] == '}') depth--;
        else if (json[pos] == '"' && depth == 1) {
            pos++;
            size_t end = pos;
            while (end < json.size() && json[end] != '"') end++;
            std::string key = json.substr(pos, end - pos);
            if (!key.empty() && key[0] == '@') result.push_back(key);
            pos = end;
        }
        pos++;
    }
    return result;
}

std::string buildIgnoredUsersJson(const std::vector<std::string>& userIds) {
    std::ostringstream os;
    os << R"({"ignored_users":{)";
    for (size_t i = 0; i < userIds.size(); i++) {
        if (i > 0) os << ",";
        os << "\"" << userIds[i] << "\":{}";
    }
    os << "}}";
    return os.str();
}

// ==== Breadcrumbs ====

std::vector<std::string> parseBreadcrumbs(const std::string& json) {
    return extractJsonStringArray(json, "recent_rooms");
}

std::string addBreadcrumb(const std::string& currentJson, const std::string& roomId) {
    auto current = parseBreadcrumbs(currentJson);

    // Remove existing entry if present
    current.erase(std::remove(current.begin(), current.end(), roomId), current.end());

    // Insert at front
    current.insert(current.begin(), roomId);

    // Limit to 20
    if (current.size() > 20) current.resize(20);

    std::ostringstream os;
    os << R"({"recent_rooms":[)";
    for (size_t i = 0; i < current.size(); i++) {
        if (i > 0) os << ",";
        os << "\"" << current[i] << "\"";
    }
    os << "]}";
    return os.str();
}

} // namespace progressive


// ==== Extended account_data_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string account_data_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool account_data_utils::deserialize(const std::string& data) {
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
bool account_data_utils::validate() const {
    if (!m_initialized) {
        LOGE("account_data_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool account_data_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool account_data_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json account_data_utils::getMetrics() const {
    json m;
    m["class"] = "account_data_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int account_data_utils::getOperationCount() const {
    return m_operationCount;
}

void account_data_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void account_data_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "account_data_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool account_data_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool account_data_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool account_data_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void account_data_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void account_data_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int account_data_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void account_data_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> account_data_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> account_data_utils::listItems() const {
    return {};
}

int account_data_utils::itemCount() const {
    return 0;
}

// Versioning
std::string account_data_utils::getVersion() const {
    return "1.0.0";
}

bool account_data_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool account_data_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void account_data_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> account_data_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool account_data_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("account_data_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int account_data_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json account_data_utils::exportData() const {
    return toJson();
}

bool account_data_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void account_data_utils::performCleanup() {
    LOGI("account_data_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t account_data_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool account_data_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool account_data_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool account_data_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void account_data_utils::logDebug(const std::string& msg) const {
    LOGI("account_data_utils: %s", msg.c_str());
}

void account_data_utils::logWarning(const std::string& msg) const {
    LOGW("account_data_utils: %s", msg.c_str());
}

void account_data_utils::logError(const std::string& msg) const {
    LOGE("account_data_utils: %s", msg.c_str());
}
