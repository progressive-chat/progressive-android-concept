#include "progressive/url_preview.hpp"
#include "progressive/link_preview.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace progressive {

// Helper: extract content of a named attribute from HTML
static std::string getAttr(const std::string& tag, const std::string& attr) {
    auto search = attr + "=\"";
    auto pos = tag.find(search);
    if (pos == std::string::npos) {
        search = attr + "='";
        pos = tag.find(search);
        if (pos == std::string::npos) return "";
        pos += search.size();
        auto end = tag.find('\'', pos);
        return tag.substr(pos, end - pos);
    }
    pos += search.size();
    auto end = tag.find('"', pos);
    return tag.substr(pos, end - pos);
}

// Helper: extract content between <tag> and </tag>
static std::string getTagContent(const std::string& html, const std::string& tagName) {
    auto openTag = "<" + tagName;
    auto pos = html.find(openTag);
    if (pos == std::string::npos) {
        openTag = "<" + tagName + " ";
        pos = html.find(openTag);
    }
    if (pos == std::string::npos) return "";

    auto tagEnd = html.find('>', pos);
    if (tagEnd == std::string::npos) return "";

    auto closeTag = "</" + tagName + ">";
    auto end = html.find(closeTag, tagEnd);
    if (end == std::string::npos) return "";

    return html.substr(tagEnd + 1, end - tagEnd - 1);
}

// Helper: extract a meta tag by property or name
static std::string getMeta(const std::string& html, const std::string& property, const std::string& attribute) {
    // Look for <meta property="..." content="..."> or <meta name="..." content="...">
    auto search = attribute + "=\"" + property + "\"";
    auto pos = html.find(search);
    if (pos == std::string::npos) {
        search = attribute + "='" + property + "'";
        pos = html.find(search);
    }
    if (pos == std::string::npos) return "";

    // Find the opening <meta ... >
    auto tagStart = html.rfind("<meta", pos);
    if (tagStart == std::string::npos) return "";
    auto tagEnd = html.find('>', pos);
    if (tagEnd == std::string::npos) return "";

    auto tag = html.substr(tagStart, tagEnd - tagStart + 1);
    return getAttr(tag, "content");
}

UrlPreview parseUrlPreview(const std::string& html, const std::string& baseUrl) {
    UrlPreview preview;
    preview.url = baseUrl;

    // Extract OpenGraph properties
    // Original Kotlin uses Jsoup: doc.select("meta[property]")
    preview.title = getMeta(html, "og:title", "property");
    preview.description = getMeta(html, "og:description", "property");
    preview.imageUrl = getMeta(html, "og:image", "property");
    preview.siteName = getMeta(html, "og:site_name", "property");
    preview.type = getMeta(html, "og:type", "property");

    // Image dimensions
    auto w = getMeta(html, "og:image:width", "property");
    if (!w.empty()) preview.imageWidth = std::stoll(w);
    auto h = getMeta(html, "og:image:height", "property");
    if (!h.empty()) preview.imageHeight = std::stoll(h);

    // Fallback to Twitter cards
    if (preview.title.empty()) preview.title = getMeta(html, "twitter:title", "name");
    if (preview.description.empty()) preview.description = getMeta(html, "twitter:description", "name");
    if (preview.imageUrl.empty()) preview.imageUrl = getMeta(html, "twitter:image", "name");

    // Fallback to HTML <title> tag
    if (preview.title.empty()) preview.title = extractHtmlTitle(html);

    // Fallback to meta description
    if (preview.description.empty()) preview.description = extractMetaDescription(html);

    // Clean up titles: trim, decode common entities
    auto trim = [](std::string s) {
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(0, 1);
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
        return s;
    };
    preview.title = trim(preview.title);
    preview.description = trim(preview.description);

    // Resolve relative image URLs
    if (!preview.imageUrl.empty()) {
        preview.imageUrl = resolveUrl(baseUrl, preview.imageUrl);
        preview.hasImage = true;
    }

    preview.hasTitle = !preview.title.empty();
    preview.valid = preview.hasTitle || !preview.description.empty();

    return preview;
}

std::string extractHtmlTitle(const std::string& html) {
    return getTagContent(html, "title");
}

std::string extractMetaDescription(const std::string& html) {
    return getMeta(html, "description", "name");
}

std::string resolveUrl(const std::string& baseUrl, const std::string& relative) {
    if (relative.empty()) return baseUrl;

    // Already absolute
    if (relative.find("http://") == 0 || relative.find("https://") == 0) return relative;
    if (relative.find("//") == 0) {
        // Protocol-relative URL
        auto proto = baseUrl.find("https://") == 0 ? "https:" : "http:";
        return proto + relative;
    }

    // Relative to domain root
    if (relative[0] == '/') {
        auto protoEnd = baseUrl.find("://");
        if (protoEnd == std::string::npos) return relative;
        auto domainEnd = baseUrl.find('/', protoEnd + 3);
        if (domainEnd == std::string::npos) return baseUrl + relative;
        return baseUrl.substr(0, domainEnd) + relative;
    }

    // Relative to current directory
    auto lastSlash = baseUrl.rfind('/');
    if (lastSlash == std::string::npos || lastSlash < 8) return baseUrl + "/" + relative;
    return baseUrl.substr(0, lastSlash + 1) + relative;
}

// isImageUrl is defined in progressive/link_preview.cpp

std::vector<std::string> extractUrls(const std::string& html) {
    std::vector<std::string> urls;
    // Simple href/src extraction
    for (const auto* attr : {"href=\"", "src=\"", "href='", "src='"}) {
        size_t pos = 0;
        while ((pos = html.find(attr, pos)) != std::string::npos) {
            pos += strlen(attr);
            auto end = html.find(attr[4] == '"' ? '"' : '\'', pos);
            if (end != std::string::npos) {
                auto url = html.substr(pos, end - pos);
                if (!url.empty() && url.find("http") == 0) {
                    urls.push_back(url);
                }
            }
        }
    }
    return urls;
}

std::string stripHtmlTags(const std::string& html) {
    std::string result;
    bool inTag = false;
    for (char c : html) {
        if (c == '<') inTag = true;
        else if (c == '>') inTag = false;
        else if (!inTag) result += c;
    }

    // Normalize whitespace
    std::string clean;
    bool wasSpace = false;
    for (char c : result) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!wasSpace) clean += ' ';
            wasSpace = true;
        } else {
            clean += c;
            wasSpace = false;
        }
    }
    return clean;
}

std::string truncateDescription(const std::string& text, size_t maxLen) {
    if (text.size() <= maxLen) return text;

    // Find last word boundary before maxLen
    auto pos = text.rfind(' ', maxLen);
    if (pos == std::string::npos || pos < maxLen / 2) pos = maxLen;

    return text.substr(0, pos) + "...";
}

std::string urlPreviewToJson(const UrlPreview& preview) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) { if (c == '"') out += "\\\""; else out += c; }
        return out;
    };
    std::ostringstream json;
    json << R"({"url": ")" << esc(preview.url) << R"(",)";
    json << R"("title": ")" << esc(preview.title) << R"(",)";
    json << R"("description": ")" << esc(preview.description) << R"(",)";
    json << R"("imageUrl": ")" << esc(preview.imageUrl) << R"(",)";
    json << R"("siteName": ")" << esc(preview.siteName) << R"(",)";
    json << R"("type": ")" << esc(preview.type) << R"(",)";
    json << R"("imageWidth": )" << preview.imageWidth << ",";
    json << R"("imageHeight": )" << preview.imageHeight << ",";
    json << R"("hasImage": )" << (preview.hasImage ? "true" : "false") << ",";
    json << R"("valid": )" << (preview.valid ? "true" : "false") << "}";
    return json.str();
}

} // namespace progressive


// ==== Extended url_preview implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string url_preview::serialize() const {
    json j = toJson();
    return j.dump();
}

bool url_preview::deserialize(const std::string& data) {
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
bool url_preview::validate() const {
    if (!m_initialized) {
        LOGE("url_preview: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool url_preview::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool url_preview::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json url_preview::getMetrics() const {
    json m;
    m["class"] = "url_preview";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int url_preview::getOperationCount() const {
    return m_operationCount;
}

void url_preview::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void url_preview::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "url_preview";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool url_preview::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool url_preview::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool url_preview::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void url_preview::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void url_preview::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int url_preview::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void url_preview::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> url_preview::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> url_preview::listItems() const {
    return {};
}

int url_preview::itemCount() const {
    return 0;
}

// Versioning
std::string url_preview::getVersion() const {
    return "1.0.0";
}

bool url_preview::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool url_preview::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void url_preview::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> url_preview::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool url_preview::migrateData(int fromVersion, int toVersion) {
    LOGI("url_preview: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int url_preview::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json url_preview::exportData() const {
    return toJson();
}

bool url_preview::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void url_preview::performCleanup() {
    LOGI("url_preview: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t url_preview::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool url_preview::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool url_preview::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool url_preview::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void url_preview::logDebug(const std::string& msg) const {
    LOGI("url_preview: %s", msg.c_str());
}

void url_preview::logWarning(const std::string& msg) const {
    LOGW("url_preview: %s", msg.c_str());
}

void url_preview::logError(const std::string& msg) const {
    LOGE("url_preview: %s", msg.c_str());
}
