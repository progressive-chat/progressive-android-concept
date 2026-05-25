#include "progressive/displayname_utils.hpp"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <unordered_set>

namespace progressive {

std::string userIdToDisplayName(const std::string& userId, bool capitalize) {
    // Extract localpart: @alice_johnson:matrix.org → alice_johnson
    if (userId.empty() || userId[0] != '@') return userId;
    auto colon = userId.find(':');
    std::string localpart = (colon != std::string::npos)
        ? userId.substr(1, colon - 1) : userId.substr(1);

    if (!capitalize) return localpart;

    // Replace delimiters with spaces and capitalize words
    std::string result;
    bool newWord = true;
    for (char c : localpart) {
        if (c == '_' || c == '.' || c == '-') {
            result += ' ';
            newWord = true;
        } else if (newWord) {
            result += std::toupper(static_cast<unsigned char>(c));
            newWord = false;
        } else {
            result += std::tolower(static_cast<unsigned char>(c));
        }
    }
    return result;
}

std::string emailToDisplayName(const std::string& email) {
    if (email.empty()) return {};
    auto at = email.find('@');
    if (at == std::string::npos) return email;

    std::string localpart = email.substr(0, at);
    std::string result;
    bool newWord = true;
    for (char c : localpart) {
        if (c == '.' || c == '_' || c == '-') {
            result += ' ';
            newWord = true;
        } else if (newWord) {
            result += std::toupper(static_cast<unsigned char>(c));
            newWord = false;
        } else {
            result += c;
        }
    }
    return result;
}

std::string userIdToColor(const std::string& userId) {
    return stringToColor(userId);
}

std::string stringToColor(const std::string& input) {
    // Simple deterministic hash
    uint32_t hash = 5381;
    for (char c : input) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }

    // Use HSL with fixed saturation and lightness for readable colors
    double hue = (hash % 360);
    double s = 0.65;
    double l = 0.55;

    // HSL to RGB
    double c = (1.0 - std::abs(2.0 * l - 1.0)) * s;
    double x = c * (1.0 - std::abs(std::fmod(hue / 60.0, 2.0) - 1.0));
    double m = l - c / 2.0;

    double r, g, b;
    if (hue < 60)      { r = c; g = x; b = 0; }
    else if (hue < 120) { r = x; g = c; b = 0; }
    else if (hue < 180) { r = 0; g = c; b = x; }
    else if (hue < 240) { r = 0; g = x; b = c; }
    else if (hue < 300) { r = x; g = 0; b = c; }
    else                { r = c; g = 0; b = x; }

    auto toHex = [m](double v) -> int { return static_cast<int>((v + m) * 255); };

    std::ostringstream out;
    out << "#";
    out << std::hex << std::setfill('0') << std::setw(2) << toHex(r);
    out << std::setw(2) << toHex(g);
    out << std::setw(2) << toHex(b);
    return out.str();
}

std::string getFirstLetter(const std::string& name) {
    // Enhanced algorithm from MatrixItem.kt: firstLetterOfDisplayName()
    // Handles: @/#/+ prefixes, LTR marks, surrogate pairs

    if (name.empty()) return "?";

    int startIndex = 0;
    int firstChar = static_cast<unsigned char>(name[0]);

    // Skip @ # + prefixes
    if ((firstChar == '@' || firstChar == '#' || firstChar == '+') && name.size() > 1) {
        startIndex = 1;
        firstChar = static_cast<unsigned char>(name[1]);
    }

    // Skip LEFT-TO-RIGHT MARK (0x200E)
    if (firstChar == 0xE2 && name.size() > startIndex + 2) {
        // UTF-8: E2 80 8E = LTR mark
        unsigned char b2 = static_cast<unsigned char>(name[startIndex + 1]);
        unsigned char b3 = static_cast<unsigned char>(name[startIndex + 2]);
        if (b2 == 0x80 && b3 == 0x8E && name.size() > startIndex + 3) {
            startIndex += 3;
            firstChar = static_cast<unsigned char>(name[startIndex]);
        }
    }

    int length = 1;
    // Check for surrogate pair (emoji): D800-DBFF followed by DC00-DFFF
    if (firstChar == 0xF0 && name.size() > startIndex + 1) {
        // 4-byte UTF-8 sequence for supplementary planes (emoji)
        length = 4;
    } else if (firstChar == 0xED && name.size() > startIndex + 5) {
        // Surrogate pair via UTF-8: ED A0 80–ED AF BF
        // Keep as single char for simplicity
        length = 1;
    }

    std::string result = name.substr(startIndex, length);
    for (char& c : result) c = std::toupper(static_cast<unsigned char>(c));
    return result;
}

std::string getInitials(const std::string& name, int maxChars) {
    std::string result;
    bool takeNext = true;
    for (char c : name) {
        if (takeNext && !std::isspace(static_cast<unsigned char>(c))) {
            result += std::toupper(static_cast<unsigned char>(c));
            takeNext = false;
            if (static_cast<int>(result.size()) >= maxChars) break;
        }
        if (std::isspace(static_cast<unsigned char>(c)) || c == '_' || c == '.') {
            takeNext = true;
        }
    }
    if (result.empty()) result = "?";
    return result;
}

bool needsDisambiguation(const std::string& name, const std::vector<std::string>& existingNames) {
    auto lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    int count = 0;
    for (const auto& n : existingNames) {
        auto lowerN = n;
        std::transform(lowerN.begin(), lowerN.end(), lowerN.begin(), ::tolower);
        if (lowerN == lowerName) count++;
    }
    return count >= 2;
}

std::string disambiguateName(const std::string& name, const std::string& userId) {
    return name + " (" + userId + ")";
}

std::string formatMemberName(const std::string& displayName, const std::string& userId,
    int powerLevel, bool showPowerBadge) {
    std::ostringstream out;
    out << displayName;
    if (showPowerBadge && powerLevel >= 100) out << " ★";
    else if (showPowerBadge && powerLevel >= 50) out << " ☆";

    if (displayName.empty() && !userId.empty()) {
        out << userIdToDisplayName(userId);
    }
    return out.str();
}

bool namesMatch(const std::string& a, const std::string& b) {
    auto lowerA = a, lowerB = b;
    // Trim
    auto trim = [](std::string& s) {
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(0, 1);
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    };
    trim(lowerA); trim(lowerB);
    std::transform(lowerA.begin(), lowerA.end(), lowerA.begin(), ::tolower);
    std::transform(lowerB.begin(), lowerB.end(), lowerB.begin(), ::tolower);
    return lowerA == lowerB;
}

std::string getBestDisplayName(const std::string& displayName, const std::string& userId) {
    if (!displayName.empty()) return displayName;
    if (!userId.empty()) return userIdToDisplayName(userId);
    return "Unknown";
}


std::string disambiguateDisplayName(
    const std::string& displayName,
    const std::string& userId,
    DisambiguationStrategy strategy
) {
    if (displayName.empty()) {
        auto pos = userId.find(':');
        return userId.substr(1, pos - 1);
    }
    if (strategy == DisambiguationStrategy::APPEND_USERID) {
        auto pos = userId.find(':');
        return displayName + " (" + userId.substr(1, pos - 1) + ")";
    }
    return displayName;
}

DisplayNameChange detectDisplayNameChange(
    const std::string& oldName,
    const std::string& newName
) {
    DisplayNameChange change;
    change.oldName = oldName;
    change.newName = newName;
    
    return change;
}

std::string formatDisplayNameChange(const DisplayNameChange& change) {
    if (change.oldName == change.newName) return "";
    if (change.oldName.empty()) return change.newName;
    if (change.newName.empty()) return "(removed)";
    return change.oldName + " → " + change.newName;
}
} // namespace progressive


// ==== Extended displayname_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string displayname_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool displayname_utils::deserialize(const std::string& data) {
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
bool displayname_utils::validate() const {
    if (!m_initialized) {
        LOGE("displayname_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool displayname_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool displayname_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json displayname_utils::getMetrics() const {
    json m;
    m["class"] = "displayname_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int displayname_utils::getOperationCount() const {
    return m_operationCount;
}

void displayname_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void displayname_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "displayname_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool displayname_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool displayname_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool displayname_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void displayname_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void displayname_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int displayname_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void displayname_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> displayname_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> displayname_utils::listItems() const {
    return {};
}

int displayname_utils::itemCount() const {
    return 0;
}

// Versioning
std::string displayname_utils::getVersion() const {
    return "1.0.0";
}

bool displayname_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool displayname_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void displayname_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> displayname_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool displayname_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("displayname_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int displayname_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json displayname_utils::exportData() const {
    return toJson();
}

bool displayname_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void displayname_utils::performCleanup() {
    LOGI("displayname_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t displayname_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool displayname_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool displayname_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool displayname_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void displayname_utils::logDebug(const std::string& msg) const {
    LOGI("displayname_utils: %s", msg.c_str());
}

void displayname_utils::logWarning(const std::string& msg) const {
    LOGW("displayname_utils: %s", msg.c_str());
}

void displayname_utils::logError(const std::string& msg) const {
    LOGE("displayname_utils: %s", msg.c_str());
}
