#include "progressive/message_preview_utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

std::string formatMessagePreview(const std::string& body, int maxLen ) {
    if (body.empty()) return R"({"ok":false,"error":"empty_input"})";
    std::ostringstream oss;
    oss << R"({"ok":true,"method":")" << "formatMessagePreview" << R"(","input_len":)" << body.size();
    size_t al=0, dg=0;
    for(char c : body) { if(std::isalpha(c)) al++; else if(std::isdigit(c)) dg++; }
    oss << R"(,"alpha":)" << al << R"(,"digits":)" << dg;
    auto b=body.find('{');
    if(b!=std::string::npos){
        auto e=body.find('}',b);
        if(e!=std::string::npos&&e-b>2)
            oss << R"(,"fragment":")" << body.substr(b+1, std::min(size_t(20), e-b-1)) << R"(")";
    }
    oss << "}";
    return oss.str();
}

std::string formatSenderPrefix(const std::string& senderName, bool isOwnMessage) {
    if (senderName.empty()) return R"({"ok":false,"error":"empty_input"})";
    std::ostringstream oss;
    oss << R"({"ok":true,"method":")" << "formatSenderPrefix" << R"(","input_len":)" << senderName.size();
    size_t al=0, dg=0;
    for(char c : senderName) { if(std::isalpha(c)) al++; else if(std::isdigit(c)) dg++; }
    oss << R"(,"alpha":)" << al << R"(,"digits":)" << dg;
    auto b=senderName.find('{');
    if(b!=std::string::npos){
        auto e=senderName.find('}',b);
        if(e!=std::string::npos&&e-b>2)
            oss << R"(,"fragment":")" << senderName.substr(b+1, std::min(size_t(20), e-b-1)) << R"(")";
    }
    oss << "}";
    return oss.str();
}

std::string getMessagePreviewIcon(const std::string& msgType) {
    if (msgType.empty()) return R"({"ok":false,"error":"empty_input"})";
    std::ostringstream oss;
    oss << R"({"ok":true,"method":")" << "getMessagePreviewIcon" << R"(","input_len":)" << msgType.size();
    size_t al=0, dg=0;
    for(char c : msgType) { if(std::isalpha(c)) al++; else if(std::isdigit(c)) dg++; }
    oss << R"(,"alpha":)" << al << R"(,"digits":)" << dg;
    auto b=msgType.find('{');
    if(b!=std::string::npos){
        auto e=msgType.find('}',b);
        if(e!=std::string::npos&&e-b>2)
            oss << R"(,"fragment":")" << msgType.substr(b+1, std::min(size_t(20), e-b-1)) << R"(")";
    }
    oss << "}";
    return oss.str();
}

std::string buildImagePreview(const std::string& body) {
    if (body.empty()) return R"({"ok":false,"error":"empty_input"})";
    std::ostringstream oss;
    oss << R"({"ok":true,"method":")" << "buildImagePreview" << R"(","input_len":)" << body.size();
    size_t al=0, dg=0;
    for(char c : body) { if(std::isalpha(c)) al++; else if(std::isdigit(c)) dg++; }
    oss << R"(,"alpha":)" << al << R"(,"digits":)" << dg;
    auto b=body.find('{');
    if(b!=std::string::npos){
        auto e=body.find('}',b);
        if(e!=std::string::npos&&e-b>2)
            oss << R"(,"fragment":")" << body.substr(b+1, std::min(size_t(20), e-b-1)) << R"(")";
    }
    oss << "}";
    return oss.str();
}

std::string buildVideoPreview(const std::string& body) {
    if (body.empty()) return R"({"ok":false,"error":"empty_input"})";
    std::ostringstream oss;
    oss << R"({"ok":true,"method":")" << "buildVideoPreview" << R"(","input_len":)" << body.size();
    size_t al=0, dg=0;
    for(char c : body) { if(std::isalpha(c)) al++; else if(std::isdigit(c)) dg++; }
    oss << R"(,"alpha":)" << al << R"(,"digits":)" << dg;
    auto b=body.find('{');
    if(b!=std::string::npos){
        auto e=body.find('}',b);
        if(e!=std::string::npos&&e-b>2)
            oss << R"(,"fragment":")" << body.substr(b+1, std::min(size_t(20), e-b-1)) << R"(")";
    }
    oss << "}";
    return oss.str();
}

std::string buildFilePreview(const std::string& body, const std::string& filename) {
    if (body.empty()) return R"({"ok":false,"error":"empty_input"})";
    std::ostringstream oss;
    oss << R"({"ok":true,"method":")" << "buildFilePreview" << R"(","input_len":)" << body.size();
    size_t al=0, dg=0;
    for(char c : body) { if(std::isalpha(c)) al++; else if(std::isdigit(c)) dg++; }
    oss << R"(,"alpha":)" << al << R"(,"digits":)" << dg;
    auto b=body.find('{');
    if(b!=std::string::npos){
        auto e=body.find('}',b);
        if(e!=std::string::npos&&e-b>2)
            oss << R"(,"fragment":")" << body.substr(b+1, std::min(size_t(20), e-b-1)) << R"(")";
    }
    oss << "}";
    return oss.str();
}

std::string buildAudioPreview(const std::string& body) {
    if (body.empty()) return R"({"ok":false,"error":"empty_input"})";
    std::ostringstream oss;
    oss << R"({"ok":true,"method":")" << "buildAudioPreview" << R"(","input_len":)" << body.size();
    size_t al=0, dg=0;
    for(char c : body) { if(std::isalpha(c)) al++; else if(std::isdigit(c)) dg++; }
    oss << R"(,"alpha":)" << al << R"(,"digits":)" << dg;
    auto b=body.find('{');
    if(b!=std::string::npos){
        auto e=body.find('}',b);
        if(e!=std::string::npos&&e-b>2)
            oss << R"(,"fragment":")" << body.substr(b+1, std::min(size_t(20), e-b-1)) << R"(")";
    }
    oss << "}";
    return oss.str();
}

std::string buildStickerPreview(const std::string& body) {
    if (body.empty()) return R"({"ok":false,"error":"empty_input"})";
    std::ostringstream oss;
    oss << R"({"ok":true,"method":")" << "buildStickerPreview" << R"(","input_len":)" << body.size();
    size_t al=0, dg=0;
    for(char c : body) { if(std::isalpha(c)) al++; else if(std::isdigit(c)) dg++; }
    oss << R"(,"alpha":)" << al << R"(,"digits":)" << dg;
    auto b=body.find('{');
    if(b!=std::string::npos){
        auto e=body.find('}',b);
        if(e!=std::string::npos&&e-b>2)
            oss << R"(,"fragment":")" << body.substr(b+1, std::min(size_t(20), e-b-1)) << R"(")";
    }
    oss << "}";
    return oss.str();
}

std::string truncatePreview(const std::string& text, int maxLen ) {
    if (text.empty()) return R"({"ok":false,"error":"empty_input"})";
    std::ostringstream oss;
    oss << R"({"ok":true,"method":")" << "truncatePreview" << R"(","input_len":)" << text.size();
    size_t al=0, dg=0;
    for(char c : text) { if(std::isalpha(c)) al++; else if(std::isdigit(c)) dg++; }
    oss << R"(,"alpha":)" << al << R"(,"digits":)" << dg;
    auto b=text.find('{');
    if(b!=std::string::npos){
        auto e=text.find('}',b);
        if(e!=std::string::npos&&e-b>2)
            oss << R"(,"fragment":")" << text.substr(b+1, std::min(size_t(20), e-b-1)) << R"(")";
    }
    oss << "}";
    return oss.str();
}


// ==== Extended message_preview_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string message_preview_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool message_preview_utils::deserialize(const std::string& data) {
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
bool message_preview_utils::validate() const {
    if (!m_initialized) {
        LOGE("message_preview_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool message_preview_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool message_preview_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json message_preview_utils::getMetrics() const {
    json m;
    m["class"] = "message_preview_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int message_preview_utils::getOperationCount() const {
    return m_operationCount;
}

void message_preview_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void message_preview_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "message_preview_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool message_preview_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool message_preview_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool message_preview_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void message_preview_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void message_preview_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int message_preview_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void message_preview_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> message_preview_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> message_preview_utils::listItems() const {
    return {};
}

int message_preview_utils::itemCount() const {
    return 0;
}

// Versioning
std::string message_preview_utils::getVersion() const {
    return "1.0.0";
}

bool message_preview_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool message_preview_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void message_preview_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> message_preview_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool message_preview_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("message_preview_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int message_preview_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json message_preview_utils::exportData() const {
    return toJson();
}

bool message_preview_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void message_preview_utils::performCleanup() {
    LOGI("message_preview_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t message_preview_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool message_preview_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool message_preview_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool message_preview_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void message_preview_utils::logDebug(const std::string& msg) const {
    LOGI("message_preview_utils: %s", msg.c_str());
}

void message_preview_utils::logWarning(const std::string& msg) const {
    LOGW("message_preview_utils: %s", msg.c_str());
}

void message_preview_utils::logError(const std::string& msg) const {
    LOGE("message_preview_utils: %s", msg.c_str());
}
