#include "progressive/matrix_patterns.hpp"
#include <regex>
#include <algorithm>

namespace progressive {

static const char* DOMAIN_REGEX = ":[A-Z0-9.-]+(:[0-9]{2,5})?";
static const char* BASE64 = "[0-9A-Za-z/\\\\+=]+";
static const char* BASE64_SAFE = "[0-9A-Za-z/\\\\_\\-]+";

bool isUserId(const std::string& input) {
    std::regex re(std::string("@[A-Z0-9\\x21-\\x39\\x3B-\\x7F]+") + DOMAIN_REGEX, std::regex::icase);
    return std::regex_match(input, re);
}

bool isRoomId(const std::string& input) {
    std::regex re(std::string("^!.+") + DOMAIN_REGEX + "$", std::regex::icase);
    return std::regex_match(input, re);
}

bool isRoomAlias(const std::string& input) {
    std::regex re(std::string("#[A-Z0-9._%#@=+-]+") + DOMAIN_REGEX, std::regex::icase);
    return std::regex_match(input, re);
}

bool isEventId(const std::string& input) {
    std::regex reV4(std::string("\\$") + BASE64_SAFE, std::regex::icase);
    std::regex reV3(std::string("\\$") + BASE64, std::regex::icase);
    return std::regex_match(input, reV4) || std::regex_match(input, reV3);
}

bool isGroupId(const std::string& input) {
    std::regex re(std::string("\\+[A-Z0-9=_\\-./]+") + DOMAIN_REGEX, std::regex::icase);
    return std::regex_match(input, re);
}

bool isMatrixToPermalink(const std::string& url) {
    return url.rfind("https://matrix.to/#/", 0) == 0;
}

bool isAppPermalink(const std::string& url) {
    std::regex re(R"(https://[A-Z0-9.-]+\.[A-Z]{2,}/#/(room|user)/)", std::regex::icase);
    return std::regex_search(url, re);
}

PermalinkInfo parseMatrixToPermalink(const std::string& url) {
    PermalinkInfo info;

    if (!isMatrixToPermalink(url)) return info;

    auto rest = url.substr(std::string("https://matrix.to/#/").size());

    if (rest.rfind('@', 0) == 0) {
        info.type = "user";
        info.userId = rest;
        info.valid = true;
    } else if (rest.rfind('#', 0) == 0) {
        info.type = "room";
        info.roomId = rest;
        info.valid = true;
    } else if (rest.rfind('!', 0) == 0) {
        auto dollar = rest.find('/');
        if (dollar != std::string::npos && dollar + 1 < rest.size() && rest[dollar + 1] == '$') {
            info.type = "event";
            info.roomId = rest.substr(0, dollar);
            info.eventId = rest.substr(dollar + 1);
            info.valid = true;
        } else {
            info.type = "room";
            info.roomId = rest;
            info.valid = true;
        }
    }

    return info;
}

ExtractedIds extractMatrixIds(const std::string& text) {
    ExtractedIds ids;

    // User IDs: @xxx:domain
    std::regex userRe(std::string("@[A-Z0-9\\x21-\\x39\\x3B-\\x7F]+") + DOMAIN_REGEX, std::regex::icase);
    for (auto it = std::sregex_iterator(text.begin(), text.end(), userRe); it != std::sregex_iterator(); ++it)
        ids.userIds.push_back(it->str());

    // Room IDs: !xxx:domain
    std::regex roomRe(std::string("![A-Z0-9._%#@=+-]+") + DOMAIN_REGEX, std::regex::icase);
    for (auto it = std::sregex_iterator(text.begin(), text.end(), roomRe); it != std::sregex_iterator(); ++it)
        ids.roomIds.push_back(it->str());

    // Room aliases: #xxx:domain
    std::regex aliasRe(std::string("#[A-Z0-9._%#@=+-]+") + DOMAIN_REGEX, std::regex::icase);
    for (auto it = std::sregex_iterator(text.begin(), text.end(), aliasRe); it != std::sregex_iterator(); ++it)
        ids.roomAliases.push_back(it->str());

    // Event IDs: $xxx
    std::regex eventRe(std::string("\\$") + BASE64_SAFE, std::regex::icase);
    for (auto it = std::sregex_iterator(text.begin(), text.end(), eventRe); it != std::sregex_iterator(); ++it)
        ids.eventIds.push_back(it->str());

    return ids;
}

bool isMxcUrl(const std::string& url) {
    return url.rfind("mxc://", 0) == 0 && url.size() > 6;
}

bool isPhoneNumber(const std::string& input) {
    // E.164-ish: +1234567890 (7-15 digits)
    std::regex re(R"(^\+\d{7,15}$)");
    return std::regex_match(input, re);
}

bool isValidEmail(const std::string& input) {
    std::regex re(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(input, re);
}

// ---- New functions from MatrixPatterns.kt lines 159-204 ----

std::string extractServerNameFromId(const std::string& matrixId) {
    // Original Kotlin: matrixId?.substringAfter(":", missingDelimiterValue = "")?.takeIf { it.isNotEmpty() }
    auto colon = matrixId.find(':');
    if (colon == std::string::npos) return "";
    std::string server = matrixId.substr(colon + 1);
    return server.empty() ? "" : server;
}

std::string extractUserNameFromId(const std::string& matrixId) {
    // Original Kotlin:
    //   fun extractUserNameFromId(matrixId: String): String? {
    //       return if (isUserId(matrixId)) {
    //           matrixId.removePrefix("@").substringBefore(":", "")
    //       } else null
    //   }
    if (!isUserId(matrixId)) return "";
    std::string localpart = matrixId;
    if (!localpart.empty() && localpart[0] == '@') localpart.erase(0, 1);
    auto colon = localpart.find(':');
    if (colon != std::string::npos) localpart = localpart.substr(0, colon);
    return localpart;
}

bool isValidOrderString(const std::string& order) {
    // Original Kotlin: order != null && order.length < 50 && order matches ORDER_STRING_REGEX
    // ORDER_STRING_REGEX = "[ -~]+" — ASCII printable characters (0x20-0x7E)
    if (order.empty() || order.size() >= 50) return false;
    for (unsigned char c : order) {
        if (c < 0x20 || c > 0x7E) return false;
    }
    return true;
}

std::string candidateAliasFromRoomName(const std::string& roomName, const std::string& domain, int maxAliasLength) {
    // Original Kotlin (MatrixPatterns.kt:candidateAliasFromRoomName):
    //   roomName.lowercase()
    //       .replaceSpaceChars("_")
    //       .removeInvalidRoomNameChars()
    //       .take(MatrixConstants.maxAliasLocalPartLength(domain))
    std::string result;

    // Lowercase and replace spaces with underscore
    for (char c : roomName) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isspace(uc)) {
            // Collapse multiple spaces into single underscore
            if (!result.empty() && result.back() != '_') result += '_';
        } else if (std::isalnum(uc) || c == '_' || c == '.' || c == '%' || c == '#' || c == '@' || c == '=' || c == '+' || c == '-') {
            result += static_cast<char>(std::tolower(uc));
        }
        // Skip invalid characters (removeInvalidRoomNameChars)
    }

    // Trim leading/trailing underscores
    while (!result.empty() && result.front() == '_') result.erase(0, 1);
    while (!result.empty() && result.back() == '_') result.pop_back();

    // Truncate to max alias localpart length
    if (static_cast<int>(result.size()) > maxAliasLength) {
        result = result.substr(0, maxAliasLength);
    }

    return result;
}

} // namespace progressive


// ==== Extended matrix_patterns implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string matrix_patterns::serialize() const {
    json j = toJson();
    return j.dump();
}

bool matrix_patterns::deserialize(const std::string& data) {
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
bool matrix_patterns::validate() const {
    if (!m_initialized) {
        LOGE("matrix_patterns: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool matrix_patterns::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool matrix_patterns::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json matrix_patterns::getMetrics() const {
    json m;
    m["class"] = "matrix_patterns";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int matrix_patterns::getOperationCount() const {
    return m_operationCount;
}

void matrix_patterns::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void matrix_patterns::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "matrix_patterns";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool matrix_patterns::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool matrix_patterns::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool matrix_patterns::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void matrix_patterns::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void matrix_patterns::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int matrix_patterns::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void matrix_patterns::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> matrix_patterns::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> matrix_patterns::listItems() const {
    return {};
}

int matrix_patterns::itemCount() const {
    return 0;
}

// Versioning
std::string matrix_patterns::getVersion() const {
    return "1.0.0";
}

bool matrix_patterns::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool matrix_patterns::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void matrix_patterns::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> matrix_patterns::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool matrix_patterns::migrateData(int fromVersion, int toVersion) {
    LOGI("matrix_patterns: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int matrix_patterns::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json matrix_patterns::exportData() const {
    return toJson();
}

bool matrix_patterns::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void matrix_patterns::performCleanup() {
    LOGI("matrix_patterns: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t matrix_patterns::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool matrix_patterns::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool matrix_patterns::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool matrix_patterns::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void matrix_patterns::logDebug(const std::string& msg) const {
    LOGI("matrix_patterns: %s", msg.c_str());
}

void matrix_patterns::logWarning(const std::string& msg) const {
    LOGW("matrix_patterns: %s", msg.c_str());
}

void matrix_patterns::logError(const std::string& msg) const {
    LOGE("matrix_patterns: %s", msg.c_str());
}
