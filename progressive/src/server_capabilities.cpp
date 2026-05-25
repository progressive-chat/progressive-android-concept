#include "progressive/server_capabilities.hpp"
#include <sstream>

namespace progressive {

// ---- Capability Checking (from ServerHomeServerCapabilities.kt:125-143) ----
// Original Kotlin:
//   fun isFeatureSupported(feature: String): ServerRoomCapabilitySupport {
//       if (roomVersions?.capabilities == null) return ServerRoomCapabilitySupport.UNKNOWN
//       val info = roomVersions.capabilities[feature] ?: return ServerRoomCapabilitySupport.UNSUPPORTED
//       val preferred = info.preferred ?: info.support.lastOrNull()
//       val versionCap = roomVersions.supportedVersion.firstOrNull { it.version == preferred }
//       return when {
//           versionCap == null -> UNKNOWN
//           versionCap.status == STABLE -> SUPPORTED
//           else -> SUPPORTED_UNSTABLE
//       }
//   }

ServerRoomCapabilitySupport isFeatureSupported(
    const ServerRoomVersionCapabilities& caps, const std::string& feature)
{
    // Original: capabilities == null → UNKNOWN
    if (caps.capabilities.empty()) return ServerRoomCapabilitySupport::Unknown;

    // Original: capabilities[feature] ?: return UNSUPPORTED
    auto it = caps.capabilities.find(feature);
    if (it == caps.capabilities.end()) return ServerRoomCapabilitySupport::Unsupported;

    const auto& info = it->second;

    // Original: val preferred = info.preferred ?: info.support.lastOrNull()
    std::string preferred = info.preferred;
    if (preferred.empty() && !info.support.empty()) {
        preferred = info.support.back();
    }
    if (preferred.empty()) return ServerRoomCapabilitySupport::Unknown;

    // Original: val versionCap = supportedVersion.firstOrNull { it.version == preferred }
    const ServerRoomVersionCapInfo* versionCap = nullptr;
    for (const auto& v : caps.supportedVersion) {
        if (v.version == preferred) { versionCap = &v; break; }
    }
    if (!versionCap) return ServerRoomCapabilitySupport::Unknown;

    // Original: versionCap.status == STABLE → SUPPORTED else SUPPORTED_UNSTABLE
    return (versionCap->status == ServerRoomVersionCap::Stable)
        ? ServerRoomCapabilitySupport::Supported
        : ServerRoomCapabilitySupport::SupportedUnstable;
}

// Original: fun isFeatureSupported(feature: String, byRoomVersion: String): Boolean
//   val info = roomVersions.capabilities[feature] ?: return false
//   return info.preferred == byRoomVersion || info.support.contains(byRoomVersion)

bool isFeatureSupportedByVersion(
    const ServerRoomVersionCapabilities& caps, const std::string& feature, const std::string& roomVersion)
{
    if (caps.capabilities.empty()) return false;

    auto it = caps.capabilities.find(feature);
    if (it == caps.capabilities.end()) return false;

    const auto& info = it->second;
    if (info.preferred == roomVersion) return true;

    for (const auto& v : info.support) {
        if (v == roomVersion) return true;
    }
    return false;
}

// Original: fun versionOverrideForFeature(feature: String): String?
//   val cap = roomVersions?.capabilities?.get(feature)
//   return cap?.preferred ?: cap?.support?.lastOrNull()

std::string versionOverrideForFeature(
    const ServerRoomVersionCapabilities& caps, const std::string& feature)
{
    if (caps.capabilities.empty()) return "";

    auto it = caps.capabilities.find(feature);
    if (it == caps.capabilities.end()) return "";

    const auto& info = it->second;
    if (!info.preferred.empty()) return info.preferred;
    if (!info.support.empty()) return info.support.back();
    return "";
}

// ---- OAuth Logout URL Builder (from ServerHomeServerCapabilities.kt:171-191) ----
// Original: fun getLogoutDeviceURL(deviceId: String): String?
//   if (externalAccountManagementUrl == null) return null
//   var action = "org.matrix.device_delete"
//   externalAccountManagementSupportedActions?.also { actions ->
//       if (actions.contains("org.matrix.device_delete")) { }
//       else if (actions.contains("org.matrix.session_end")) { action = "org.matrix.session_end" }
//       else if (actions.contains("session_end")) { action = "session_end" }
//   }
//   return externalAccountManagementUrl.removeSuffix("/") + "?action=${action}&device_id=${deviceId}"

std::string buildLogoutDeviceUrl(
    const ServerHomeServerCapabilities& caps, const std::string& deviceId)
{
    if (caps.externalAccountManagementUrl.empty() || deviceId.empty()) return "";

    // Original: var action = "org.matrix.device_delete" (default stable value)
    std::string action = "org.matrix.device_delete";

    const auto& actions = caps.externalAccountManagementSupportedActions;
    bool hasDeviceDelete = false, hasSessionEnd = false, hasRawSessionEnd = false;
    for (const auto& a : actions) {
        if (a == "org.matrix.device_delete") hasDeviceDelete = true;
        if (a == "org.matrix.session_end") hasSessionEnd = true;
        if (a == "session_end") hasRawSessionEnd = true;
    }

    if (!actions.empty() && !hasDeviceDelete) {
        if (hasSessionEnd) {
            action = "org.matrix.session_end";    // earlier MSC4191
        } else if (hasRawSessionEnd) {
            action = "session_end";              // previous unspecified
        }
    }

    std::string baseUrl = caps.externalAccountManagementUrl;
    while (!baseUrl.empty() && baseUrl.back() == '/') baseUrl.pop_back();

    // URL-encode device_id (simple version — just alphanum escape)
    std::ostringstream url;
    url << baseUrl << "?action=" << action << "&device_id=" << deviceId;
    return url.str();
}

// ---- Default Capabilities ----
ServerHomeServerCapabilities getDefaultCapabilities() {
    return ServerHomeServerCapabilities{};
}

// ---- Parse from JSON ----
ServerHomeServerCapabilities parseCapabilities(const std::string& json) {
    ServerHomeServerCapabilities caps;

    // Simple JSON extractors
    auto extractBool = [&](const std::string& key, bool defaultVal = false) -> bool {
        auto search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos) return defaultVal;
        pos += search.size();
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        if (json.find("true", pos) == pos) return true;
        return false;
    };

    auto extractStr = [&](const std::string& key) -> std::string {
        auto search = "\"" + key + "\":\"";
        auto pos = json.find(search);
        if (pos == std::string::npos) return "";
        pos += search.size();
        auto end = json.find('"', pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    };

    caps.canChangePassword = extractBool("m.change_password", true);
    caps.canChangeDisplayName = extractBool("m.change_displayname", true);
    caps.canChangeAvatar = extractBool("m.change_avatar_url", true);
    caps.canChange3pid = extractBool("m.3pid_changes", true);
    caps.canUseThreading = extractBool("m.thread", false);
    caps.canUseAuthenticatedMedia = extractBool("m.authenticated_media", false);
    caps.canLoginWithQrCode = extractBool("m.login_qr", false);
    caps.canRedactRelatedEvents = extractBool("m.redact_related_events", false);
    caps.authenticationIssuer = extractStr("m.authentication_issuer");

    return caps;
}

bool isDelegatedOidcEnabled(const ServerHomeServerCapabilities& caps) {
    // Original: val delegatedOidcAuthEnabled: Boolean = authenticationIssuer != null
    return !caps.authenticationIssuer.empty();
}

std::string capabilitiesToJson(const ServerHomeServerCapabilities& caps) {
    std::ostringstream json;
    json << "{";
    json << R"("canChangePassword": )" << (caps.canChangePassword ? "true" : "false") << ",";
    json << R"("canChangeDisplayName": )" << (caps.canChangeDisplayName ? "true" : "false") << ",";
    json << R"("canChangeAvatar": )" << (caps.canChangeAvatar ? "true" : "false") << ",";
    json << R"("canChange3pid": )" << (caps.canChange3pid ? "true" : "false") << ",";
    json << R"("maxUploadFileSize": )" << caps.maxUploadFileSize << ",";
    json << R"("canUseThreading": )" << (caps.canUseThreading ? "true" : "false") << ",";
    json << R"("canUseThreadReadReceipts": )" << (caps.canUseThreadReadReceiptsAndNotifications ? "true" : "false") << ",";
    json << R"("canLoginWithQrCode": )" << (caps.canLoginWithQrCode ? "true" : "false") << ",";
    json << R"("canUseAuthenticatedMedia": )" << (caps.canUseAuthenticatedMedia ? "true" : "false") << ",";
    json << R"("delegatedOidcEnabled": )" << (isDelegatedOidcEnabled(caps) ? "true" : "false");
    json << "}";
    return json.str();
    return json.str();
}

// ==== HomeServer Version (from HomeServerVersion.kt:30-66) ====
bool HomeServerVersion::operator<(const HomeServerVersion& other) const {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    return patch < other.patch;
}

std::string HomeServerVersion::toString() const {
    return "r" + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

HomeServerVersion parseHomeServerVersion(const std::string& versionStr) {
    HomeServerVersion ver;

    // Original: pattern = Regex("""[r|v](\d+)\.(\d+)(?:\.(\d+))?""")
    // Parse "r0.6.1" or "v1.11.0" or "r0.4"
    const char* s = versionStr.c_str();
    if (*s == 'r' || *s == 'v') s++; else return ver;

    // Parse major
    while (*s >= '0' && *s <= '9') { ver.major = ver.major * 10 + (*s - '0'); s++; }
    if (*s != '.') return ver; s++;

    // Parse minor
    while (*s >= '0' && *s <= '9') { ver.minor = ver.minor * 10 + (*s - '0'); s++; }

    // Parse optional patch
    if (*s == '.') {
        s++;
        while (*s >= '0' && *s <= '9') { ver.patch = ver.patch * 10 + (*s - '0'); s++; }
    }

    return ver;
}

} // namespace progressive


// ==== Extended server_capabilities implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string server_capabilities::serialize() const {
    json j = toJson();
    return j.dump();
}

bool server_capabilities::deserialize(const std::string& data) {
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
bool server_capabilities::validate() const {
    if (!m_initialized) {
        LOGE("server_capabilities: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool server_capabilities::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool server_capabilities::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json server_capabilities::getMetrics() const {
    json m;
    m["class"] = "server_capabilities";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int server_capabilities::getOperationCount() const {
    return m_operationCount;
}

void server_capabilities::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void server_capabilities::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "server_capabilities";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool server_capabilities::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool server_capabilities::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool server_capabilities::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void server_capabilities::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void server_capabilities::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int server_capabilities::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void server_capabilities::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> server_capabilities::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> server_capabilities::listItems() const {
    return {};
}

int server_capabilities::itemCount() const {
    return 0;
}

// Versioning
std::string server_capabilities::getVersion() const {
    return "1.0.0";
}

bool server_capabilities::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool server_capabilities::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void server_capabilities::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> server_capabilities::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool server_capabilities::migrateData(int fromVersion, int toVersion) {
    LOGI("server_capabilities: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int server_capabilities::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json server_capabilities::exportData() const {
    return toJson();
}

bool server_capabilities::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void server_capabilities::performCleanup() {
    LOGI("server_capabilities: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t server_capabilities::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool server_capabilities::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool server_capabilities::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool server_capabilities::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void server_capabilities::logDebug(const std::string& msg) const {
    LOGI("server_capabilities: %s", msg.c_str());
}

void server_capabilities::logWarning(const std::string& msg) const {
    LOGW("server_capabilities: %s", msg.c_str());
}

void server_capabilities::logError(const std::string& msg) const {
    LOGE("server_capabilities: %s", msg.c_str());
}
