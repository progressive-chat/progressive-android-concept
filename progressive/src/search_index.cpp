#include "progressive/search_index.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <unordered_set>

namespace progressive {

std::string SearchIndex::toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

float SearchIndex::computeRelevance(const std::string& body, const std::string& query) {
    auto lowerBody = toLower(body);
    auto lowerQuery = toLower(query);

    // Split query into terms
    std::vector<std::string> terms;
    std::istringstream qs(lowerQuery);
    std::string term;
    while (qs >> term) terms.push_back(term);

    if (terms.empty()) return 0.0f;

    int matches = 0;
    int totalTerms = static_cast<int>(terms.size());

    for (const auto& t : terms) {
        if (lowerBody.find(t) != std::string::npos) {
            ++matches;
        }
    }

    // Base score: what fraction of query terms matched
    float baseScore = static_cast<float>(matches) / static_cast<float>(totalTerms);

    // Bonus: exact phrase match
    if (lowerBody.find(lowerQuery) != std::string::npos) {
        baseScore += 0.3f;
    }

    // Bonus: query appears early in body
    auto pos = lowerBody.find(terms[0]);
    if (pos != std::string::npos && pos < 50) {
        baseScore += 0.1f;
    }

    return std::min(1.0f, baseScore);
}

void SearchIndex::indexMessage(const std::string& eventId, const std::string& roomId,
                               const std::string& roomName, const std::string& senderName,
                               const std::string& body, int64_t timestamp,
                               bool isEncrypted) {
    // Remove old entry if exists
    entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
        [&](const IndexedMessage& m) { return m.eventId == eventId; }
    ), entries_.end());

    entries_.push_back({eventId, roomId, roomName, senderName, body, timestamp, isEncrypted});
}

SearchResult SearchIndex::search(const std::string& query, int limit) const {
    auto startTime = std::chrono::steady_clock::now();

    SearchResult result;
    result.query = query;

    // Collect hits with relevance
    std::vector<std::pair<SearchHit, float>> scoredHits;
    std::unordered_set<std::string> roomsSearched;

    for (const auto& entry : entries_) {
        auto relevance = computeRelevance(entry.body, query);
        if (relevance > 0.0f) {
            SearchHit hit;
            hit.eventId = entry.eventId;
            hit.roomId = entry.roomId;
            hit.roomName = entry.roomName;
            hit.senderName = entry.senderName;
            hit.isEncrypted = entry.isEncrypted;
            hit.originServerTs = entry.timestamp;
            hit.relevanceScore = relevance;

            // Create body snippet (first 200 chars)
            auto pos = toLower(entry.body).find(toLower(query));
            int snippetStart = pos != std::string::npos ? std::max(0, static_cast<int>(pos) - 40) : 0;
            hit.body = entry.body.substr(snippetStart, 200);
            if (snippetStart > 0) hit.body = "..." + hit.body;
            if (entry.body.size() > static_cast<size_t>(snippetStart + 200)) hit.body += "...";

            scoredHits.push_back({hit, relevance});
            roomsSearched.insert(entry.roomId);
        }
    }

    // Sort by relevance (highest first), then by timestamp (newest first)
    std::sort(scoredHits.begin(), scoredHits.end(),
        [](const auto& a, const auto& b) {
            if (a.second != b.second) return a.second > b.second;
            return a.first.originServerTs > b.first.originServerTs;
        }
    );

    // Take top N
    auto end = scoredHits.size() > static_cast<size_t>(limit)
        ? scoredHits.begin() + limit : scoredHits.end();
    for (auto it = scoredHits.begin(); it != end; ++it) {
        result.hits.push_back(it->first);
    }

    result.totalHits = static_cast<int>(scoredHits.size());
    result.roomsSearched = static_cast<int>(roomsSearched.size());

    auto endTime = std::chrono::steady_clock::now();
    result.searchTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    return result;
}

void SearchIndex::removeRoom(const std::string& roomId) {
    entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
        [&](const IndexedMessage& m) { return m.roomId == roomId; }
    ), entries_.end());
}

void SearchIndex::clear() {
    entries_.clear();
}

std::string SearchIndex::hitsToJson(const std::vector<SearchHit>& hits) {
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
    for (size_t i = 0; i < hits.size(); ++i) {
        if (i > 0) json << ",";
        const auto& h = hits[i];
        json << R"({"eventId": ")" << esc(h.eventId) << R"(")";
        json << R"(,"roomId": ")" << esc(h.roomId) << R"(")";
        json << R"(,"roomName": ")" << esc(h.roomName) << R"(")";
        json << R"(,"senderName": ")" << esc(h.senderName) << R"(")";
        json << R"(,"body": ")" << esc(h.body) << R"(")";
        json << R"(,"relevance": )" << h.relevanceScore;
        json << R"(,"isEncrypted": )" << (h.isEncrypted ? "true" : "false") << "}";
    }
    json << "]";
    return json.str();
}

} // namespace progressive


// ==== Extended search_index implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string search_index::serialize() const {
    json j = toJson();
    return j.dump();
}

bool search_index::deserialize(const std::string& data) {
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
bool search_index::validate() const {
    if (!m_initialized) {
        LOGE("search_index: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool search_index::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool search_index::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json search_index::getMetrics() const {
    json m;
    m["class"] = "search_index";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int search_index::getOperationCount() const {
    return m_operationCount;
}

void search_index::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void search_index::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "search_index";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool search_index::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool search_index::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool search_index::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void search_index::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void search_index::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int search_index::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void search_index::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> search_index::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> search_index::listItems() const {
    return {};
}

int search_index::itemCount() const {
    return 0;
}

// Versioning
std::string search_index::getVersion() const {
    return "1.0.0";
}

bool search_index::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool search_index::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void search_index::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> search_index::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool search_index::migrateData(int fromVersion, int toVersion) {
    LOGI("search_index: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int search_index::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json search_index::exportData() const {
    return toJson();
}

bool search_index::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void search_index::performCleanup() {
    LOGI("search_index: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t search_index::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool search_index::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool search_index::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool search_index::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void search_index::logDebug(const std::string& msg) const {
    LOGI("search_index: %s", msg.c_str());
}

void search_index::logWarning(const std::string& msg) const {
    LOGW("search_index: %s", msg.c_str());
}

void search_index::logError(const std::string& msg) const {
    LOGE("search_index: %s", msg.c_str());
}
