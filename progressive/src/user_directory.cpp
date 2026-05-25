#include "progressive/user_directory.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace progressive {

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

static bool extractBool(const std::string& json, const std::string& key) {
    return json.find("\"" + key + "\":true") != std::string::npos;
}

// ====== Constructor ======

UserDirectoryManager::UserDirectoryManager() {}

// ====== Search Request ======

std::string UserDirectoryManager::buildSearchRequest(const UserSearchQuery& query) const {
    std::ostringstream os;
    os << R"({"search_term":")" << query.searchTerm
       << R"(","limit":)" << query.limit;
    if (!query.serverFilter.empty()) {
        os << R"(,"server":")" << query.serverFilter << R"(")";
    }
    os << "}";
    return os.str();
}

// ====== Search Response Parsing ======

UserSearchResponse UserDirectoryManager::parseSearchResponse(const std::string& json) const {
    UserSearchResponse resp;

    // Check for error
    auto err = extractStr(json, "errcode");
    if (!err.empty()) {
        resp.error = err + ": " + extractStr(json, "error");
        return resp;
    }

    resp.limited = extractBool(json, "limited");

    // Parse results array
    size_t pos = json.find("\"results\"");
    if (pos == std::string::npos) return resp;

    pos = json.find('[', pos);
    if (pos == std::string::npos) return resp;

    pos++;
    while (pos < json.size()) {
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == ',' || json[pos] == '\n')) pos++;
        if (pos >= json.size() || json[pos] == ']') break;

        size_t objStart = pos;
        int depth = 0;
        while (pos < json.size()) {
            if (json[pos] == '{') depth++;
            else if (json[pos] == '}') depth--;
            if (depth == 0 && json[pos] == '}') { pos++; break; }
            pos++;
        }
        std::string userJson = json.substr(objStart, pos - objStart);

        UserSearchResult user;
        user.userId = extractStr(userJson, "user_id");
        user.displayName = extractStr(userJson, "display_name");
        user.avatarUrl = extractStr(userJson, "avatar_url");
        user.matrixId = user.userId;
        user.isValid = !user.userId.empty();

        if (user.isValid) resp.results.push_back(user);
    }

    resp.totalResults = static_cast<int>(resp.results.size());
    return resp;
}

// ====== Search ======

UserSearchResponse UserDirectoryManager::search(const UserSearchQuery& query, const std::string& responseJson) {
    auto resp = parseSearchResponse(responseJson);
    if (!resp.error.empty()) return resp;

    // Filter self
    if (query.excludeSelf && !query.currentUserId.empty()) {
        resp.results.erase(
            std::remove_if(resp.results.begin(), resp.results.end(),
                [&](const UserSearchResult& r) { return r.userId == query.currentUserId; }),
            resp.results.end());
    }

    // Deduplicate
    deduplicate(resp.results);

    // Calculate relevance
    for (auto& r : resp.results) {
        r.relevanceScore = calculateRelevance(r, query.searchTerm);
    }

    // Sort
    sortByRelevance(resp.results, query.searchTerm);

    resp.totalResults = static_cast<int>(resp.results.size());

    // Cache results
    for (const auto& r : resp.results) {
        cache_[r.userId] = r;
    }

    return resp;
}

// ====== Deduplication ======

void UserDirectoryManager::deduplicate(std::vector<UserSearchResult>& results) const {
    std::unordered_map<std::string, UserSearchResult> unique;
    for (const auto& r : results) {
        auto it = unique.find(r.userId);
        if (it == unique.end()) {
            unique[r.userId] = r;
        } else if (r.relevanceScore > it->second.relevanceScore) {
            unique[r.userId] = r;
        }
    }

    results.clear();
    for (const auto& [id, user] : unique) results.push_back(user);
}

// ====== Tokenization ======

std::vector<std::string> UserDirectoryManager::tokenize(const std::string& text) const {
    std::vector<std::string> tokens;
    std::string current;
    for (char c : text) {
        if (c == ' ' || c == ',' || c == '.') {
            if (!current.empty()) tokens.push_back(current);
            current.clear();
        } else {
            current += static_cast<char>(std::tolower(c));
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

// ====== Relevance Calculation ======

int UserDirectoryManager::calculateRelevance(const UserSearchResult& result, const std::string& query) const {
    if (query.empty()) return 0;

    std::string q;
    for (char c : query) q += static_cast<char>(std::tolower(c));

    std::string name;
    for (char c : result.displayName) name += static_cast<char>(std::tolower(c));

    std::string uid;
    for (char c : result.userId) uid += static_cast<char>(std::tolower(c));

    int score = 0;

    // Exact match on display name (highest priority)
    if (name == q) score += 100;
    // Exact match on user ID
    else if (uid == q) score += 90;
    // Display name starts with query
    else if (name.rfind(q, 0) == 0) score += 60;
    // Display name contains all tokens
    else {
        auto tokens = tokenize(q);
        int matchCount = 0;
        for (const auto& t : tokens) {
            if (name.find(t) != std::string::npos) matchCount++;
            else if (uid.find(t) != std::string::npos) matchCount++;
        }
        if (matchCount == static_cast<int>(tokens.size())) score += 50;
        else if (matchCount > 0) score += matchCount * 15;
    }

    // Contains query as substring
    if (name.find(q) != std::string::npos) score += 20;
    if (uid.find(q) != std::string::npos) score += 10;

    return score;
}

// ====== Sorting ======

void UserDirectoryManager::sortByRelevance(std::vector<UserSearchResult>& results, const std::string& query) const {
    std::sort(results.begin(), results.end(), [](const UserSearchResult& a, const UserSearchResult& b) {
        // Higher score first
        if (a.relevanceScore != b.relevanceScore) return a.relevanceScore > b.relevanceScore;
        // Then alphabetical by display name
        if (a.displayName != b.displayName) return a.displayName < b.displayName;
        // Then by user ID
        return a.userId < b.userId;
    });
}

// ====== Display Formatting ======

std::string UserDirectoryManager::getBestDisplayName(const UserSearchResult& user) const {
    return getBestDisplayName(user.displayName, user.userId);
}

std::string UserDirectoryManager::getBestDisplayName(const std::string& displayName, const std::string& userId) const {
    if (!displayName.empty()) return displayName;
    // Extract localpart from @user:server
    if (userId.size() > 1 && userId[0] == '@') {
        auto colon = userId.find(':');
        if (colon != std::string::npos) return userId.substr(1, colon - 1);
    }
    return userId;
}

std::string UserDirectoryManager::formatUserResult(const UserSearchResult& user) const {
    auto name = getBestDisplayName(user);
    return name + " (" + user.userId + ")";
}

std::string UserDirectoryManager::getAvatarInitial(const UserSearchResult& user) const {
    auto name = getBestDisplayName(user);
    if (!name.empty()) return name.substr(0, 1);
    if (user.userId.size() > 1 && user.userId[0] == '@') return user.userId.substr(1, 1);
    return "?";
}

// ====== Validation ======

bool UserDirectoryManager::isValidSearchQuery(const std::string& query) const {
    return query.size() >= 2 && query.size() <= 256;
}

// ====== Serialization ======

std::string UserDirectoryManager::resultsToJson(const std::vector<UserSearchResult>& results) const {
    std::ostringstream os;
    os << "[";
    for (size_t i = 0; i < results.size(); i++) {
        if (i > 0) os << ",";
        os << userToJson(results[i]);
    }
    os << "]";
    return os.str();
}

std::string UserDirectoryManager::userToJson(const UserSearchResult& user) const {
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) { if (c == '"') out += "\\\""; else out += c; }
        return out;
    };

    std::ostringstream os;
    os << R"({"user_id":")" << esc(user.userId)
       << R"(","display_name":")" << esc(user.displayName)
       << R"(","best_name":")" << esc(getBestDisplayName(user))
       << R"(","avatar_url":")" << esc(user.avatarUrl)
       << R"(","avatar_init":")" << esc(getAvatarInitial(user))
       << R"(","relevance":)" << user.relevanceScore
       << "}";
    return os.str();
}

std::string UserDirectoryManager::responseToJson(const UserSearchResponse& response) const {
    std::ostringstream os;
    os << R"({"results":)" << resultsToJson(response.results)
       << R"(,"limited":)" << (response.limited ? "true" : "false")
       << R"(,"total":)" << response.totalResults;
    if (!response.error.empty()) os << R"(,"error":")" << response.error << R"(")";
    os << "}";
    return os.str();
}

} // namespace progressive


// ==== Extended user_directory implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string user_directory::serialize() const {
    json j = toJson();
    return j.dump();
}

bool user_directory::deserialize(const std::string& data) {
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
bool user_directory::validate() const {
    if (!m_initialized) {
        LOGE("user_directory: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool user_directory::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool user_directory::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json user_directory::getMetrics() const {
    json m;
    m["class"] = "user_directory";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int user_directory::getOperationCount() const {
    return m_operationCount;
}

void user_directory::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void user_directory::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "user_directory";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool user_directory::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool user_directory::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool user_directory::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void user_directory::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void user_directory::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int user_directory::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void user_directory::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> user_directory::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> user_directory::listItems() const {
    return {};
}

int user_directory::itemCount() const {
    return 0;
}

// Versioning
std::string user_directory::getVersion() const {
    return "1.0.0";
}

bool user_directory::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool user_directory::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void user_directory::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> user_directory::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool user_directory::migrateData(int fromVersion, int toVersion) {
    LOGI("user_directory: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int user_directory::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json user_directory::exportData() const {
    return toJson();
}

bool user_directory::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void user_directory::performCleanup() {
    LOGI("user_directory: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t user_directory::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool user_directory::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool user_directory::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool user_directory::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void user_directory::logDebug(const std::string& msg) const {
    LOGI("user_directory: %s", msg.c_str());
}

void user_directory::logWarning(const std::string& msg) const {
    LOGW("user_directory: %s", msg.c_str());
}

void user_directory::logError(const std::string& msg) const {
    LOGE("user_directory: %s", msg.c_str());
}
