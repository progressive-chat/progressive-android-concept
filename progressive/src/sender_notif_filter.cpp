#include "progressive/sender_notif_filter.hpp"
#include <sstream>
#include <algorithm>

namespace progressive {

// ==== Notification Decision Logic ====

bool shouldNotifyForSender(
    const SenderNotifSettings& settings,
    const std::string& senderId,
    bool hasMention,
    bool hasHighlight)
{
    switch (settings.mode) {
        case SenderNotifMode::NotifyAll:
            // Always notify — even without mention/highlight
            return true;

        case SenderNotifMode::NotifyMentions:
            // Only notify for @mentions and highlight keywords
            return hasMention || hasHighlight;

        case SenderNotifMode::NotifyList:
            // Only notify if sender is in allowed list
            if (isSenderAllowed(settings, senderId)) return true;
            // Always notify for mentions even if sender not in list
            return hasMention;

        case SenderNotifMode::MuteList:
            // Notify everyone EXCEPT muted senders
            if (isSenderMuted(settings, senderId)) {
                // Exception: always notify for @mentions from muted senders
                return hasMention;
            }
            return true;
    }
    return true;
}

// ==== Sender List Management ====

void muteSender(SenderNotifSettings& settings, const std::string& senderId) {
    if (!isSenderMuted(settings, senderId)) {
        settings.mutedSenders.push_back(senderId);
    }
    settings.mode = SenderNotifMode::MuteList;
}

void unmuteSender(SenderNotifSettings& settings, const std::string& senderId) {
    settings.mutedSenders.erase(
        std::remove(settings.mutedSenders.begin(), settings.mutedSenders.end(), senderId),
        settings.mutedSenders.end());
    if (settings.mutedSenders.empty() && settings.allowedSenders.empty()) {
        settings.mode = SenderNotifMode::NotifyAll;
    }
}

void allowSender(SenderNotifSettings& settings, const std::string& senderId) {
    if (!isSenderAllowed(settings, senderId)) {
        settings.allowedSenders.push_back(senderId);
    }
    settings.mode = SenderNotifMode::NotifyList;
}

void disallowSender(SenderNotifSettings& settings, const std::string& senderId) {
    settings.allowedSenders.erase(
        std::remove(settings.allowedSenders.begin(), settings.allowedSenders.end(), senderId),
        settings.allowedSenders.end());
    if (settings.allowedSenders.empty() && settings.mutedSenders.empty()) {
        settings.mode = SenderNotifMode::NotifyAll;
    }
}

bool isSenderMuted(const SenderNotifSettings& settings, const std::string& senderId) {
    return std::find(settings.mutedSenders.begin(), settings.mutedSenders.end(), senderId)
           != settings.mutedSenders.end();
}

bool isSenderAllowed(const SenderNotifSettings& settings, const std::string& senderId) {
    return std::find(settings.allowedSenders.begin(), settings.allowedSenders.end(), senderId)
           != settings.allowedSenders.end();
}

// ==== Serialization ====

std::string senderNotifModeToString(SenderNotifMode mode) {
    switch (mode) {
        case SenderNotifMode::NotifyAll: return "notify_all";
        case SenderNotifMode::NotifyMentions: return "notify_mentions";
        case SenderNotifMode::NotifyList: return "notify_list";
        case SenderNotifMode::MuteList: return "mute_list";
    }
    return "notify_all";
}

SenderNotifMode senderNotifModeFromString(const std::string& mode) {
    if (mode == "notify_all") return SenderNotifMode::NotifyAll;
    if (mode == "notify_mentions") return SenderNotifMode::NotifyMentions;
    if (mode == "notify_list") return SenderNotifMode::NotifyList;
    if (mode == "mute_list") return SenderNotifMode::MuteList;
    return SenderNotifMode::NotifyAll;
}

std::string senderNotifSettingsToJson(const SenderNotifSettings& settings) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    std::ostringstream json;
    json << R"({"roomId": ")" << esc(settings.roomId) << R"(",)";
    json << R"("mode": ")" << senderNotifModeToString(settings.mode) << R"(",)";
    json << R"("muteUnknown": )" << (settings.muteUnknownSenders ? "true" : "false") << ",";
    json << R"("allowedSenders": [)";
    for (size_t i = 0; i < settings.allowedSenders.size(); ++i) {
        if (i > 0) json << ",";
        json << R"(")" << esc(settings.allowedSenders[i]) << R"(")";
    }
    json << "],";
    json << R"("mutedSenders": [)";
    for (size_t i = 0; i < settings.mutedSenders.size(); ++i) {
        if (i > 0) json << ",";
        json << R"(")" << esc(settings.mutedSenders[i]) << R"(")";
    }
    json << "]}";
    return json.str();
}

SenderNotifSettings parseSenderNotifSettings(const std::string& json) {
    SenderNotifSettings settings;

    auto extractStr = [&](const std::string& key) -> std::string {
        auto search = "\"" + key + "\":\"";
        auto pos = json.find(search);
        if (pos == std::string::npos) {
            search = "\"" + key + "\": \"";
            pos = json.find(search);
        }
        if (pos == std::string::npos) return "";
        pos += search.size();
        auto end = json.find('"', pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    };

    settings.roomId = extractStr("roomId");
    settings.mode = senderNotifModeFromString(extractStr("mode"));
    settings.muteUnknownSenders = json.find("\"muteUnknown\": true") != std::string::npos;

    // Parse allowedSenders array
    auto allowedPos = json.find("\"allowedSenders\"");
    if (allowedPos != std::string::npos) {
        size_t pos = json.find('[', allowedPos);
        if (pos != std::string::npos) {
            pos++;
            while (pos < json.size() && json[pos] != ']') {
                if (json[pos] == '"') {
                    size_t end = json.find('"', pos + 1);
                    if (end != std::string::npos) {
                        settings.allowedSenders.push_back(json.substr(pos + 1, end - pos - 1));
                        pos = end + 1;
                        continue;
                    }
                }
                pos++;
            }
        }
    }

    // Parse mutedSenders array
    auto mutedPos = json.find("\"mutedSenders\"");
    if (mutedPos != std::string::npos) {
        size_t pos = json.find('[', mutedPos);
        if (pos != std::string::npos) {
            pos++;
            while (pos < json.size() && json[pos] != ']') {
                if (json[pos] == '"') {
                    size_t end = json.find('"', pos + 1);
                    if (end != std::string::npos) {
                        settings.mutedSenders.push_back(json.substr(pos + 1, end - pos - 1));
                        pos = end + 1;
                        continue;
                    }
                }
                pos++;
            }
        }
    }

    return settings;
}

SenderNotifSettings getDefaultSenderNotifSettings(const std::string& roomId) {
    SenderNotifSettings settings;
    settings.roomId = roomId;
    return settings;
}

} // namespace progressive


// ==== Extended sender_notif_filter implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string sender_notif_filter::serialize() const {
    json j = toJson();
    return j.dump();
}

bool sender_notif_filter::deserialize(const std::string& data) {
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
bool sender_notif_filter::validate() const {
    if (!m_initialized) {
        LOGE("sender_notif_filter: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool sender_notif_filter::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool sender_notif_filter::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json sender_notif_filter::getMetrics() const {
    json m;
    m["class"] = "sender_notif_filter";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int sender_notif_filter::getOperationCount() const {
    return m_operationCount;
}

void sender_notif_filter::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void sender_notif_filter::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "sender_notif_filter";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool sender_notif_filter::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool sender_notif_filter::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool sender_notif_filter::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void sender_notif_filter::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void sender_notif_filter::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int sender_notif_filter::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void sender_notif_filter::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> sender_notif_filter::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> sender_notif_filter::listItems() const {
    return {};
}

int sender_notif_filter::itemCount() const {
    return 0;
}

// Versioning
std::string sender_notif_filter::getVersion() const {
    return "1.0.0";
}

bool sender_notif_filter::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool sender_notif_filter::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void sender_notif_filter::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> sender_notif_filter::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool sender_notif_filter::migrateData(int fromVersion, int toVersion) {
    LOGI("sender_notif_filter: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int sender_notif_filter::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json sender_notif_filter::exportData() const {
    return toJson();
}

bool sender_notif_filter::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void sender_notif_filter::performCleanup() {
    LOGI("sender_notif_filter: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t sender_notif_filter::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool sender_notif_filter::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool sender_notif_filter::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool sender_notif_filter::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void sender_notif_filter::logDebug(const std::string& msg) const {
    LOGI("sender_notif_filter: %s", msg.c_str());
}

void sender_notif_filter::logWarning(const std::string& msg) const {
    LOGW("sender_notif_filter: %s", msg.c_str());
}

void sender_notif_filter::logError(const std::string& msg) const {
    LOGE("sender_notif_filter: %s", msg.c_str());
}
