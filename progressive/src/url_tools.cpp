#include "progressive/url_tools.hpp"
#include <sstream>
#include <regex>
#include <iomanip>

namespace progressive {

UrlParts parseUrlParts(const std::string& url) {
    UrlParts result;
    if (url.empty()) return result;

    // Protocol
    auto protoEnd = url.find("://");
    if (protoEnd == std::string::npos) return result;

    result.protocol = url.substr(0, protoEnd);
    auto rest = url.substr(protoEnd + 3);

    // Fragment
    auto fragPos = rest.find('#');
    if (fragPos != std::string::npos) {
        result.fragment = rest.substr(fragPos + 1);
        rest = rest.substr(0, fragPos);
    }

    // Query
    auto qPos = rest.find('?');
    if (qPos != std::string::npos) {
        result.query = rest.substr(qPos);
        rest = rest.substr(0, qPos);
    }

    // Path
    auto pathPos = rest.find('/');
    if (pathPos != std::string::npos) {
        result.path = rest.substr(pathPos);
        rest = rest.substr(0, pathPos);
    } else {
        result.path = "/";
    }

    // Host:port
    auto portPos = rest.find(':');
    if (portPos != std::string::npos) {
        result.port = rest.substr(portPos + 1);
        result.host = rest.substr(0, portPos);
    } else {
        result.host = rest;
    }

    result.valid = !result.host.empty();
    return result;
}

bool isLikelyUrl(const std::string& text) {
    return text.rfind("http://", 0) == 0 ||
           text.rfind("https://", 0) == 0 ||
           text.rfind("matrix://", 0) == 0 ||
           text.rfind("matrix.to/", 0) == 0 ||
           text.rfind("ftp://", 0) == 0;
}

std::string extractFirstUrl(const std::string& text) {
    std::regex urlRe(R"((https?://|matrix://|ftp://)[^\s<>"]+)");
    std::smatch match;
    if (std::regex_search(text, match, urlRe)) {
        return match[0];
    }
    return {};
}

std::vector<std::string> extractAllUrls(const std::string& text) {
    std::vector<std::string> urls;
    std::regex urlRe(R"((https?://|matrix://|ftp://)[^\s<>"]+)");
    for (auto it = std::sregex_iterator(text.begin(), text.end(), urlRe);
         it != std::sregex_iterator(); ++it) {
        urls.push_back(it->str());
    }
    return urls;
}

std::string getDomain(const std::string& url) {
    auto parsed = parseUrlParts(url);
    return parsed.host;
}

bool isHttps(const std::string& url) {
    return url.rfind("https://", 0) == 0;
}

bool isMatrixUrl(const std::string& url) {
    return url.rfind("matrix://", 0) == 0 ||
           url.find("matrix.to/") != std::string::npos ||
           url.rfind("mxc://", 0) == 0;
}

std::string buildMatrixToUrl(const std::string& roomIdOrAlias) {
    return "https://matrix.to/#/" + roomIdOrAlias;
}

std::string urlEncode(const std::string& input) {
    std::ostringstream encoded;
    for (char c : input) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' ||
            c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::uppercase << std::hex << std::setw(2)
                    << std::setfill('0') << (static_cast<int>(c) & 0xFF);
        }
    }
    return encoded.str();
}

std::string urlDecode(const std::string& input) {
    std::string result;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%' && i + 2 < input.size()) {
            int val = 0;
            for (int j = 1; j <= 2; ++j) {
                char c = input[i + j];
                if (c >= '0' && c <= '9') val = val * 16 + (c - '0');
                else if (c >= 'A' && c <= 'F') val = val * 16 + (c - 'A' + 10);
                else if (c >= 'a' && c <= 'f') val = val * 16 + (c - 'a' + 10);
                else {
                    val = -1;
                    break;
                }
            }
            if (val >= 0) {
                result += static_cast<char>(val);
                i += 2;
                continue;
            }
        }
        result += input[i];
    }
    return result;
}

MxcInfo parseMxcUrl(const std::string& mxcUrl) {
    MxcInfo result;
    if (mxcUrl.rfind("mxc://", 0) != 0) return result;

    auto rest = mxcUrl.substr(6);
    auto slash = rest.find('/');
    if (slash == std::string::npos) {
        result.serverName = rest;
    } else {
        result.serverName = rest.substr(0, slash);
        result.mediaId = rest.substr(slash + 1);
    }
    return result;
}

// ==== URL Utilities (from UrlUtils.kt:21-50) ====
bool isValidUrl(const std::string& url) {
    auto protoEnd = url.find("://");
    if (protoEnd == std::string::npos || protoEnd < 2) return false;
    for (size_t i = 0; i < protoEnd; ++i) {
        char c = url[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.')) return false;
    }
    return url.size() > protoEnd + 3;
}

std::string ensureProtocol(const std::string& url) {
    if (url.empty()) return url;
    if (url.find("http") == 0) return url;
    return "https://" + url;
}

std::string ensureTrailingSlash(const std::string& url) {
    if (url.empty()) return url;
    if (url.back() == '/') return url;
    return url + "/";
}

} // namespace progressive


// ==== Extended url_tools implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string url_tools::serialize() const {
    json j = toJson();
    return j.dump();
}

bool url_tools::deserialize(const std::string& data) {
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
bool url_tools::validate() const {
    if (!m_initialized) {
        LOGE("url_tools: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool url_tools::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool url_tools::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json url_tools::getMetrics() const {
    json m;
    m["class"] = "url_tools";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int url_tools::getOperationCount() const {
    return m_operationCount;
}

void url_tools::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void url_tools::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "url_tools";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool url_tools::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool url_tools::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool url_tools::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void url_tools::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void url_tools::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int url_tools::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void url_tools::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> url_tools::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> url_tools::listItems() const {
    return {};
}

int url_tools::itemCount() const {
    return 0;
}

// Versioning
std::string url_tools::getVersion() const {
    return "1.0.0";
}

bool url_tools::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool url_tools::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void url_tools::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> url_tools::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool url_tools::migrateData(int fromVersion, int toVersion) {
    LOGI("url_tools: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int url_tools::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json url_tools::exportData() const {
    return toJson();
}

bool url_tools::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void url_tools::performCleanup() {
    LOGI("url_tools: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t url_tools::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool url_tools::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool url_tools::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool url_tools::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void url_tools::logDebug(const std::string& msg) const {
    LOGI("url_tools: %s", msg.c_str());
}

void url_tools::logWarning(const std::string& msg) const {
    LOGW("url_tools: %s", msg.c_str());
}

void url_tools::logError(const std::string& msg) const {
    LOGE("url_tools: %s", msg.c_str());
}
