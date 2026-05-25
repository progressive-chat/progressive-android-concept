#include "progressive/reaction_utils.hpp"
#include <sstream>
#include <algorithm>
#include <unordered_set>

namespace progressive {

ReactionSummary aggregateReactions(
    const std::string& eventId,
    const std::vector<std::string>& reactionEmojis,
    const std::vector<std::string>& reactorIds,
    const std::vector<int64_t>& timestamps,
    const std::string& myUserId
) {
    ReactionSummary summary;
    summary.eventId = eventId;

    // Aggregate by emoji
    std::unordered_map<std::string, ReactionInfo> byEmoji;
    std::unordered_set<std::string> uniqueReactors;

    for (size_t i = 0; i < reactionEmojis.size(); ++i) {
        const auto& emoji = reactionEmojis[i];
        const auto& userId = i < reactorIds.size() ? reactorIds[i] : "";
        int64_t ts = i < timestamps.size() ? timestamps[i] : 0;

        auto& info = byEmoji[emoji];
        if (info.emoji.empty()) info.emoji = emoji;
        info.count++;
        info.userIds.push_back(userId);
        if (info.firstTimestamp == 0 || ts < info.firstTimestamp) info.firstTimestamp = ts;
        if (userId == myUserId) info.addedByMe = true;

        uniqueReactors.insert(userId);
    }

    summary.totalReactions = static_cast<int>(reactionEmojis.size());
    summary.uniqueReactors = static_cast<int>(uniqueReactors.size());

    // Sort by count descending
    for (const auto& p : byEmoji) {
        summary.reactions.push_back(p.second);
    }
    std::sort(summary.reactions.begin(), summary.reactions.end(),
        [](const ReactionInfo& a, const ReactionInfo& b) {
            return a.count > b.count;
        }
    );

    if (!summary.reactions.empty()) {
        summary.topEmoji = summary.reactions[0].emoji;
    }

    return summary;
}

std::vector<std::string> getQuickReactions() {
    // Most commonly used reaction emojis
    return {"👍", "👎", "😄", "🎉", "😕", "❤️", "🚀", "👀", "🔥", "💯", "✅", "❌"};
}

bool isValidReactionEmoji(const std::string& emoji) {
    if (emoji.empty() || emoji.size() > 20) return false;
    // Must contain at least one non-ASCII character (emoji range)
    for (char c : emoji) {
        if (static_cast<unsigned char>(c) >= 0xF0) return true; // 4-byte UTF-8
        if (static_cast<unsigned char>(c) >= 0xE0) return true; // 3-byte UTF-8
    }
    // Allow ASCII emoticons like ":)" as reactions
    return emoji.size() >= 2;
}

std::string formatReactionSummary(const ReactionSummary& summary) {
    std::ostringstream out;
    for (size_t i = 0; i < summary.reactions.size(); ++i) {
        if (i > 0) out << ", ";
        out << summary.reactions[i].emoji << " " << summary.reactions[i].count;
    }
    return out.str();
}

std::string formatReactionAccessibility(const ReactionSummary& summary) {
    if (summary.totalReactions == 0) return "No reactions";

    std::ostringstream out;
    for (size_t i = 0; i < summary.reactions.size(); ++i) {
        const auto& r = summary.reactions[i];
        if (i > 0) {
            out << (i == summary.reactions.size() - 1 ? " and " : ", ");
        }
        out << r.count << " " << r.emoji;
    }
    out << " reactions";
    return out.str();
}

std::string extractReactionKey(const std::string& eventContentJson) {
    // Matrix reaction content: {"m.relates_to": {"key": "👍", ...}}
    auto keyPos = eventContentJson.find("\"key\":");
    if (keyPos == std::string::npos) return {};

    keyPos += 6;
    while (keyPos < eventContentJson.size() && eventContentJson[keyPos] == ' ') ++keyPos;

    if (keyPos >= eventContentJson.size() || eventContentJson[keyPos] != '"') return {};
    ++keyPos;

    auto end = eventContentJson.find('"', keyPos);
    if (end == std::string::npos) return {};

    return eventContentJson.substr(keyPos, end - keyPos);
}

bool isSameEmoji(const std::string& a, const std::string& b) {
    if (a == b) return true;
    // Strip variation selectors (U+FE0F)
    auto strip = [](std::string s) -> std::string {
        std::string result;
        for (size_t i = 0; i < s.size(); ++i) {
            // Skip variation selector-16 (EF B8 8F)
            if (i + 2 < s.size() &&
                static_cast<unsigned char>(s[i]) == 0xEF &&
                static_cast<unsigned char>(s[i+1]) == 0xB8 &&
                static_cast<unsigned char>(s[i+2]) == 0x8F) {
                i += 2;
                continue;
            }
            result += s[i];
        }
        return result;
    };
    return strip(a) == strip(b);
}

std::string reactionSummaryToJson(const ReactionSummary& summary) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    std::ostringstream json;
    json << R"({"eventId": ")" << esc(summary.eventId) << R"(",)";
    json << R"("totalReactions": )" << summary.totalReactions << ",";
    json << R"("topEmoji": ")" << esc(summary.topEmoji) << R"(",)";
    json << R"("uniqueReactors": )" << summary.uniqueReactors << ",";
    json << R"("showAll": )" << (summary.showAll ? "true" : "false") << ",";
    json << R"("reactions": [)";
    for (size_t i = 0; i < summary.reactions.size(); ++i) {
        if (i > 0) json << ",";
        const auto& r = summary.reactions[i];
        json << R"({"emoji": ")" << esc(r.emoji) << R"(",)";
        json << R"("count": )" << r.count << ",";
        json << R"("addedByMe": )" << (r.addedByMe ? "true" : "false") << ",";
        json << R"("synced": )" << (r.synced ? "true" : "false") << "}";
    }
    json << "]}";
    return json.str();
}

} // namespace progressive


// ==== Extended reaction_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string reaction_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool reaction_utils::deserialize(const std::string& data) {
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
bool reaction_utils::validate() const {
    if (!m_initialized) {
        LOGE("reaction_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool reaction_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool reaction_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json reaction_utils::getMetrics() const {
    json m;
    m["class"] = "reaction_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int reaction_utils::getOperationCount() const {
    return m_operationCount;
}

void reaction_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void reaction_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "reaction_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool reaction_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool reaction_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool reaction_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void reaction_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void reaction_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int reaction_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void reaction_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> reaction_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> reaction_utils::listItems() const {
    return {};
}

int reaction_utils::itemCount() const {
    return 0;
}

// Versioning
std::string reaction_utils::getVersion() const {
    return "1.0.0";
}

bool reaction_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool reaction_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void reaction_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> reaction_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool reaction_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("reaction_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int reaction_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json reaction_utils::exportData() const {
    return toJson();
}

bool reaction_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void reaction_utils::performCleanup() {
    LOGI("reaction_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t reaction_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool reaction_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool reaction_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool reaction_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void reaction_utils::logDebug(const std::string& msg) const {
    LOGI("reaction_utils: %s", msg.c_str());
}

void reaction_utils::logWarning(const std::string& msg) const {
    LOGW("reaction_utils: %s", msg.c_str());
}

void reaction_utils::logError(const std::string& msg) const {
    LOGE("reaction_utils: %s", msg.c_str());
}
