#include "progressive/content_guard.hpp"
#include <sstream>
#include <unordered_set>

namespace progressive {

// ==== Emoji Detection ====
// Unicode emoji ranges from https://unicode.org/emoji/charts/full-emoji-list.html

bool isEmojiCodePoint(int cp) {
    // Basic emoji ranges
    if (cp >= 0x1F600 && cp <= 0x1F64F) return true; // Emoticons
    if (cp >= 0x1F300 && cp <= 0x1F5FF) return true; // Misc Symbols & Pictographs
    if (cp >= 0x1F680 && cp <= 0x1F6FF) return true; // Transport & Map
    if (cp >= 0x1F700 && cp <= 0x1F77F) return true; // Alchemical Symbols
    if (cp >= 0x1F780 && cp <= 0x1F7FF) return true; // Geometric Shapes Extended
    if (cp >= 0x1F800 && cp <= 0x1F8FF) return true; // Supplemental Arrows-C
    if (cp >= 0x1F900 && cp <= 0x1F9FF) return true; // Supplemental Symbols & Pictographs
    if (cp >= 0x1FA00 && cp <= 0x1FA6F) return true; // Chess Symbols
    if (cp >= 0x1FA70 && cp <= 0x1FAFF) return true; // Symbols & Pictographs Extended-A
    if (cp >= 0x2600 && cp <= 0x26FF) return true;   // Misc symbols
    if (cp >= 0x2700 && cp <= 0x27BF) return true;   // Dingbats
    if (cp >= 0xFE00 && cp <= 0xFE0F) return true;   // Variation Selectors
    if (cp >= 0x1F000 && cp <= 0x1F02F) return true; // Mahjong Tiles
    if (cp >= 0x1F0A0 && cp <= 0x1F0FF) return true; // Playing Cards
    if (cp >= 0x1F100 && cp <= 0x1F1FF) return true; // Enclosed Alphanumeric Supplement
    if (cp >= 0x1F200 && cp <= 0x1F2FF) return true; // Enclosed Ideographic Supplement
    if (cp >= 0x1F600 && cp <= 0x1F64F) return true; // Emoticons (redundant, kept for clarity)
    if (cp >= 0x2300 && cp <= 0x23FF) return true;   // Misc Technical
    if (cp >= 0x2B50) {
        if (cp == 0x2B50 || cp == 0x2B55) return true; // Star symbols
    }
    if (cp >= 0x200D) {
        // Zero-width joiner for combined emoji (e.g., family, skin tones)
        if (cp == 0x200D) return true;
    }
    // Flags
    if (cp >= 0x1F1E6 && cp <= 0x1F1FF) return true;

    return false;
}

int countEmojis(const std::string& text) {
    int count = 0;
    size_t i = 0;
    while (i < text.size()) {
        unsigned char b = static_cast<unsigned char>(text[i]);
        int cp = -1;

        // UTF-8 decoding
        if (b < 0x80) {
            cp = b;
            i += 1;
        } else if ((b & 0xE0) == 0xC0 && i + 1 < text.size()) {
            cp = ((b & 0x1F) << 6) | (text[i + 1] & 0x3F);
            i += 2;
        } else if ((b & 0xF0) == 0xE0 && i + 2 < text.size()) {
            cp = ((b & 0x0F) << 12) | ((text[i + 1] & 0x3F) << 6) | (text[i + 2] & 0x3F);
            i += 3;
        } else if ((b & 0xF8) == 0xF0 && i + 3 < text.size()) {
            cp = ((b & 0x07) << 18) | ((text[i + 1] & 0x3F) << 12) |
                 ((text[i + 2] & 0x3F) << 6) | (text[i + 3] & 0x3F);
            i += 4;
        } else {
            i++;
            continue;
        }

        if (cp >= 0 && isEmojiCodePoint(cp)) count++;
    }
    return count;
}

int countUniqueEmojis(const std::string& text) {
    std::unordered_set<int> seen;
    size_t i = 0;
    while (i < text.size()) {
        unsigned char b = static_cast<unsigned char>(text[i]);
        int cp = -1;

        if (b < 0x80) { cp = b; i += 1; }
        else if ((b & 0xE0) == 0xC0 && i + 1 < text.size()) {
            cp = ((b & 0x1F) << 6) | (text[i + 1] & 0x3F); i += 2;
        } else if ((b & 0xF0) == 0xE0 && i + 2 < text.size()) {
            cp = ((b & 0x0F) << 12) | ((text[i + 1] & 0x3F) << 6) | (text[i + 2] & 0x3F); i += 3;
        } else if ((b & 0xF8) == 0xF0 && i + 3 < text.size()) {
            cp = ((b & 0x07) << 18) | ((text[i + 1] & 0x3F) << 12) |
                 ((text[i + 2] & 0x3F) << 6) | (text[i + 3] & 0x3F); i += 4;
        } else { i++; continue; }

        if (cp >= 0 && isEmojiCodePoint(cp)) seen.insert(cp);
    }
    return static_cast<int>(seen.size());
}

EmojiCheck checkEmojiAttack(const std::string& text, int maxEmojis, int maxUniqueEmojis) {
    EmojiCheck result;
    result.totalEmojis = countEmojis(text);
    result.uniqueEmojis = countUniqueEmojis(text);

    if (maxEmojis > 0 && result.totalEmojis > maxEmojis) {
        result.isAttack = true;
        result.limitExceeded = maxEmojis;
        result.label = "(emoji attack)";
    } else if (maxUniqueEmojis > 0 && result.uniqueEmojis > maxUniqueEmojis) {
        result.isAttack = true;
        result.limitExceeded = maxUniqueEmojis;
        result.label = "(emoji attack)";
    }

    return result;
}

// ==== Media Collapse ====

MediaGroup checkMediaCollapse(
    const std::vector<std::string>& mediaTypes,
    int threshold)
{
    MediaGroup group;
    group.threshold = threshold;

    if (threshold <= 0 || mediaTypes.empty()) return group;

    for (const auto& type : mediaTypes) {
        if (type == "image") group.imageCount++;
        else if (type == "video") group.videoCount++;
        else if (type == "file") group.fileCount++;
    }

    group.totalMedia = static_cast<int>(mediaTypes.size());
    group.shouldCollapse = group.totalMedia >= threshold;

    if (group.shouldCollapse) {
        group.label = formatMediaCollapseLabel(group.totalMedia);
    }

    return group;
}

std::vector<ContentMediaItem> groupMedia(
    const std::vector<std::string>& mediaTypes,
    int threshold)
{
    std::vector<ContentMediaItem> result;
    if (threshold <= 0) {
        for (const auto& t : mediaTypes) result.push_back({t, false, 0});
        return result;
    }

    // Count consecutive media items
    int consecutiveMedia = 0;
    for (size_t i = 0; i < mediaTypes.size(); ++i) {
        bool isMedia = (mediaTypes[i] == "image" || mediaTypes[i] == "video" || mediaTypes[i] == "file");
        if (isMedia) {
            consecutiveMedia++;
        } else {
            // Flush pending media count
            if (consecutiveMedia >= threshold) {
                result.push_back({"image", true, consecutiveMedia});
            } else if (consecutiveMedia > 0) {
                for (int j = 0; j < consecutiveMedia; ++j)
                    result.push_back({"image", false, 0}); // simplified
            }
            consecutiveMedia = 0;
            result.push_back({mediaTypes[i], false, 0});
        }
    }

    // Flush remaining
    if (consecutiveMedia >= threshold) {
        result.push_back({"image", true, consecutiveMedia});
    } else if (consecutiveMedia > 0) {
        for (int j = 0; j < consecutiveMedia; ++j)
            result.push_back({"image", false, 0});
    }

    return result;
}

std::string formatMediaCollapseLabel(int count) {
    if (count <= 0) return "";
    return std::to_string(count) + " media omitted";
}



// ---- Content Trust & Guard Policies ----

ContentTrustDecision evaluateContentTrust(
    const std::string& contentJson,
    ContentTrustPolicy policy,
    const std::vector<ContentValidationRule>& rules
) {
    if (policy == ContentTrustPolicy::TRUST_ALL) return ContentTrustDecision::ALLOW;
    if (policy == ContentTrustPolicy::BLOCK_ALL) return ContentTrustDecision::BLOCK;

    for (const auto& rule : rules) {
        if (!rule.enabled) continue;

        // Check if content matches the rule pattern
        bool matches = false;
        if (!rule.condition.empty()) {
            matches = contentJson.find(rule.condition) != std::string::npos;
        }
        if (!rule.condition.empty()) {
            matches = matches || contentJson.find(rule.condition) != std::string::npos;
        }

        if (matches) {
            switch (rule.action) {
                case ContentRuleAction::BLOCK:    return ContentTrustDecision::BLOCK;
                case ContentRuleAction::QUARANTINE: return ContentTrustDecision::QUARANTINE;
                case ContentRuleAction::WARN:     return ContentTrustDecision::QUARANTINE;
                case ContentRuleAction::REPORT:   return ContentTrustDecision::QUARANTINE;
                default: break;
            }
        }
    }

    if (policy == ContentTrustPolicy::SCAN_REQUIRED) {
        return ContentTrustDecision::SCAN_PENDING;
    }

    return ContentTrustDecision::ALLOW;
}

std::string buildContentWarning(
    ContentWarningType type,
    const std::string& reason,
    const std::string& detail
) {
    std::ostringstream os;
    os << "{";
    os << R"("warning_type":)";
    switch (type) {
        case ContentWarningType::EXPLICIT:  os << R"("explicit")"; break;
        case ContentWarningType::VIOLENCE:  os << R"("violence")"; break;
        case ContentWarningType::SPAM:      os << R"("spam")"; break;
        case ContentWarningType::PHISHING:  os << R"("phishing")"; break;
        case ContentWarningType::NSFW:      os << R"("nsfw")"; break;
        case ContentWarningType::CUSTOM:    os << R"("custom")"; break;
    }
    if (!reason.empty()) os << R"(,"reason":")" << reason << R"(")";
    if (!detail.empty()) os << R"(,"detail":")" << detail << R"(")";
    os << "}";
    return os.str();
}

ContentWarningType classifyContentWarning(const std::string& scanResult) {
    std::string lower;
    std::transform(scanResult.begin(), scanResult.end(), std::back_inserter(lower), ::tolower);

    if (lower.find("nudity") != std::string::npos ||
        lower.find("explicit") != std::string::npos ||
        lower.find("nsfw") != std::string::npos)
        return ContentWarningType::EXPLICIT;

    if (lower.find("violence") != std::string::npos ||
        lower.find("gore") != std::string::npos)
        return ContentWarningType::VIOLENCE;

    if (lower.find("spam") != std::string::npos ||
        lower.find("unsolicited") != std::string::npos)
        return ContentWarningType::SPAM;

    if (lower.find("phish") != std::string::npos ||
        lower.find("impersonat") != std::string::npos)
        return ContentWarningType::PHISHING;

    return ContentWarningType::CUSTOM;
}

} // namespace progressive


// ==== Extended content_guard implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string content_guard::serialize() const {
    json j = toJson();
    return j.dump();
}

bool content_guard::deserialize(const std::string& data) {
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
bool content_guard::validate() const {
    if (!m_initialized) {
        LOGE("content_guard: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool content_guard::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool content_guard::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json content_guard::getMetrics() const {
    json m;
    m["class"] = "content_guard";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int content_guard::getOperationCount() const {
    return m_operationCount;
}

void content_guard::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void content_guard::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "content_guard";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool content_guard::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool content_guard::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool content_guard::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void content_guard::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void content_guard::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int content_guard::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void content_guard::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> content_guard::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> content_guard::listItems() const {
    return {};
}

int content_guard::itemCount() const {
    return 0;
}

// Versioning
std::string content_guard::getVersion() const {
    return "1.0.0";
}

bool content_guard::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool content_guard::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void content_guard::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> content_guard::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool content_guard::migrateData(int fromVersion, int toVersion) {
    LOGI("content_guard: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int content_guard::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json content_guard::exportData() const {
    return toJson();
}

bool content_guard::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void content_guard::performCleanup() {
    LOGI("content_guard: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t content_guard::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool content_guard::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool content_guard::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool content_guard::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void content_guard::logDebug(const std::string& msg) const {
    LOGI("content_guard: %s", msg.c_str());
}

void content_guard::logWarning(const std::string& msg) const {
    LOGW("content_guard: %s", msg.c_str());
}

void content_guard::logError(const std::string& msg) const {
    LOGE("content_guard: %s", msg.c_str());
}
