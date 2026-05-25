#include "matrix_sdk/matrix_sdk_types.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <map>
#include <mutex>
#include <regex>
#include <set>
#include <sstream>
#include <unordered_map>

namespace matrix_sdk {

using json = nlohmann::json;

// ────────────────────────────────────────────────────────────
// UrlPreviewResult
// ────────────────────────────────────────────────────────────
struct UrlPreviewResult {
    std::string url;
    std::string title;
    std::string description;
    std::string imageUrl;
    std::string siteName;
    std::string faviconUrl;
    std::string ogType;
    std::string ogLocale;
    std::string twitterCard;
    int imageWidth = 0;
    int imageHeight = 0;
    int64_t imageSize = 0;
    int64_t fetchTimestamp = 0;
    bool valid = false;
    std::string mimeType;

    // Matrix-compatible formatted output
    std::string toOpenGraphJson() const {
        json j;
        j["og:title"] = title;
        j["og:description"] = description;
        j["og:image"] = imageUrl;
        j["og:image:width"] = imageWidth;
        j["og:image:height"] = imageHeight;
        j["og:type"] = ogType;
        j["og:site_name"] = siteName;
        j["og:url"] = url;
        if (!ogLocale.empty()) j["og:locale"] = ogLocale;
        if (!twitterCard.empty()) j["twitter:card"] = twitterCard;
        return j.dump();
    }
};

// ────────────────────────────────────────────────────────────
// HtmlParser: lightweight HTML/XML parser for metadata extraction
// ────────────────────────────────────────────────────────────
class HtmlParser {
public:
    explicit HtmlParser(const std::string& html) : html_(html), pos_(0) {}

    struct MetaTag {
        std::string name;
        std::string property;
        std::string content;
        std::string itemprop;
    };

    std::vector<MetaTag> extractMetaTags() {
        std::vector<MetaTag> tags;
        std::regex metaRegex(R"(<meta\s+([^>]+)/?>)", std::regex::icase);
        auto begin = std::sregex_iterator(html_.begin(), html_.end(), metaRegex);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            MetaTag tag;
            std::string attrs = (*it)[1].str();
            tag.name = extractAttribute(attrs, "name");
            tag.property = extractAttribute(attrs, "property");
            tag.content = extractAttribute(attrs, "content");
            tag.itemprop = extractAttribute(attrs, "itemprop");
            if (!tag.content.empty()) {
                tags.push_back(tag);
            }
        }
        return tags;
    }

    std::string extractTitle() {
        std::regex titleRegex(R"(<title[^>]*>([^<]+)</title>)", std::regex::icase);
        std::smatch match;
        if (std::regex_search(html_, match, titleRegex)) {
            return trim(match[1].str());
        }
        return "";
    }

    std::vector<std::string> extractLinks() {
        std::vector<std::string> links;
        std::regex linkRegex(R"(<link\s+([^>]+)/?>)", std::regex::icase);
        auto begin = std::sregex_iterator(html_.begin(), html_.end(), linkRegex);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            std::string attrs = (*it)[1].str();
            std::string rel = extractAttribute(attrs, "rel");
            std::string href = extractAttribute(attrs, "href");
            std::transform(rel.begin(), rel.end(), rel.begin(), ::tolower);
            if (rel.find("icon") != std::string::npos ||
                rel.find("shortcut icon") != std::string::npos ||
                rel == "apple-touch-icon") {
                links.push_back(href);
            }
        }
        return links;
    }

    struct ImageInfo {
        std::string src;
        int width = 0;
        int height = 0;
    };

    std::vector<ImageInfo> extractImages() {
        std::vector<ImageInfo> images;
        std::regex imgRegex(R"(<img\s+([^>]+)>?)", std::regex::icase);
        auto begin = std::sregex_iterator(html_.begin(), html_.end(), imgRegex);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            ImageInfo img;
            std::string attrs = (*it)[1].str();
            img.src = extractAttribute(attrs, "src");
            std::string w = extractAttribute(attrs, "width");
            std::string h = extractAttribute(attrs, "height");
            if (!w.empty()) img.width = std::stoi(w);
            if (!h.empty()) img.height = std::stoi(h);
            if (!img.src.empty()) {
                images.push_back(img);
            }
        }
        return images;
    }

    std::string extractDescription() {
        // Try meta description first
        std::regex descRegex(R"(<meta\s+name=["']description["']\s+content=["']([^"']+)["'])", std::regex::icase);
        std::smatch match;
        if (std::regex_search(html_, match, descRegex)) {
            return match[1].str();
        }
        return "";
    }

private:
    std::string html_;
    size_t pos_;

    static std::string extractAttribute(const std::string& attrs, const std::string& attrName) {
        std::string pattern = attrName + R"(=["']([^"']+)["'])";
        std::regex attrRegex(pattern, std::regex::icase);
        std::smatch match;
        if (std::regex_search(attrs, match, attrRegex)) {
            return match[1].str();
        }
        return "";
    }

    static std::string trim(const std::string& str) {
        auto start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        auto end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }
};

// ────────────────────────────────────────────────────────────
// OpenGraphParser: structured OpenGraph metadata parsing
// ────────────────────────────────────────────────────────────
class OpenGraphParser {
public:
    struct OgData {
        std::string title;
        std::string type;
        std::string image;
        std::string url;
        std::string description;
        std::string siteName;
        std::string locale;
        std::string determiner;
        int imageWidth = 0;
        int imageHeight = 0;
        std::string imageType;
        std::string audio;
        std::string video;
        std::vector<std::string> imageVariants;  // og:image:url variants
    };

    OgData parse(const std::string& html) {
        HtmlParser parser(html);
        auto metaTags = parser.extractMetaTags();
        OgData og;

        for (const auto& tag : metaTags) {
            if (!tag.property.empty()) {
                std::string prop = tag.property;
                if (prop == "og:title") og.title = tag.content;
                else if (prop == "og:type") og.type = tag.content;
                else if (prop == "og:image") og.image = tag.content;
                else if (prop == "og:image:width") og.imageWidth = std::stoi(tag.content);
                else if (prop == "og:image:height") og.imageHeight = std::stoi(tag.content);
                else if (prop == "og:image:type") og.imageType = tag.content;
                else if (prop == "og:image:url") og.imageVariants.push_back(tag.content);
                else if (prop == "og:url") og.url = tag.content;
                else if (prop == "og:description") og.description = tag.content;
                else if (prop == "og:site_name") og.siteName = tag.content;
                else if (prop == "og:locale") og.locale = tag.content;
                else if (prop == "og:determiner") og.determiner = tag.content;
                else if (prop == "og:audio") og.audio = tag.content;
                else if (prop == "og:video") og.video = tag.content;
            }
        }
        return og;
    }
};

// ────────────────────────────────────────────────────────────
// TwitterCardParser: Twitter Card metadata parsing
// ────────────────────────────────────────────────────────────
class TwitterCardParser {
public:
    struct TwitterCard {
        std::string card;       // summary, summary_large_image, app, player
        std::string site;
        std::string creator;
        std::string title;
        std::string description;
        std::string image;
        std::string imageAlt;
        int imageWidth = 0;
        int imageHeight = 0;
        std::string playerUrl;
        int playerWidth = 0;
        int playerHeight = 0;
        std::string appNameIphone;
        std::string appIdIphone;
        std::string appNameIpad;
        std::string appIdIpad;
        std::string appNameGoogleplay;
        std::string appIdGoogleplay;
    };

    TwitterCard parse(const std::string& html) {
        HtmlParser parser(html);
        auto metaTags = parser.extractMetaTags();
        TwitterCard card;

        for (const auto& tag : metaTags) {
            if (!tag.name.empty()) {
                std::string name = tag.name;
                if (name == "twitter:card") card.card = tag.content;
                else if (name == "twitter:site") card.site = tag.content;
                else if (name == "twitter:creator") card.creator = tag.content;
                else if (name == "twitter:title") card.title = tag.content;
                else if (name == "twitter:description") card.description = tag.content;
                else if (name == "twitter:image") card.image = tag.content;
                else if (name == "twitter:image:alt") card.imageAlt = tag.content;
                else if (name == "twitter:image:width") card.imageWidth = std::stoi(tag.content);
                else if (name == "twitter:image:height") card.imageHeight = std::stoi(tag.content);
                else if (name == "twitter:player") card.playerUrl = tag.content;
                else if (name == "twitter:player:width") card.playerWidth = std::stoi(tag.content);
                else if (name == "twitter:player:height") card.playerHeight = std::stoi(tag.content);
                else if (name == "twitter:app:name:iphone") card.appNameIphone = tag.content;
                else if (name == "twitter:app:id:iphone") card.appIdIphone = tag.content;
                else if (name == "twitter:app:name:ipad") card.appNameIpad = tag.content;
                else if (name == "twitter:app:id:ipad") card.appIdIpad = tag.content;
                else if (name == "twitter:app:name:googleplay") card.appNameGoogleplay = tag.content;
                else if (name == "twitter:app:id:googleplay") card.appIdGoogleplay = tag.content;
            }
        }
        return card;
    }
};

// ───────────────────────────────────────────────────────────ー
// OEmbedParser: oEmbed discovery and parsing
// ────────────────────────────────────────────────────────────
class OEmbedParser {
public:
    struct OEmbedData {
        std::string type;       // photo, video, link, rich
        std::string version;
        std::string title;
        std::string authorName;
        std::string authorUrl;
        std::string providerName;
        std::string providerUrl;
        std::string cacheAge;
        std::string thumbnailUrl;
        int thumbnailWidth = 0;
        int thumbnailHeight = 0;
        std::string url;
        std::string html;
        int width = 0;
        int height = 0;
    };

    // Discover oEmbed endpoint from HTML
    std::string discoverEndpoint(const std::string& html) {
        HtmlParser parser(html);
        std::regex oembedRegex(
            R"(<link\s+[^>]*type=["']application/json\+oembed["'][^>]*href=["']([^"']+)["'])",
            std::regex::icase);
        std::smatch match;
        if (std::regex_search(html, match, oembedRegex)) {
            return match[1].str();
        }
        // Try alternative ordering
        std::regex oembedRegex2(
            R"(<link\s+[^>]*href=["']([^"']+)["'][^>]*type=["']application/json\+oembed["'])",
            std::regex::icase);
        if (std::regex_search(html, match, oembedRegex2)) {
            return match[1].str();
        }
        return "";
    }

    OEmbedData parse(const std::string& jsonResponse) {
        OEmbedData data;
        try {
            auto j = json::parse(jsonResponse);
            data.type = j.value("type", "");
            data.version = j.value("version", "1.0");
            data.title = j.value("title", "");
            data.authorName = j.value("author_name", "");
            data.authorUrl = j.value("author_url", "");
            data.providerName = j.value("provider_name", "");
            data.providerUrl = j.value("provider_url", "");
            data.cacheAge = j.value("cache_age", "");
            data.thumbnailUrl = j.value("thumbnail_url", "");
            data.thumbnailWidth = j.value("thumbnail_width", 0);
            data.thumbnailHeight = j.value("thumbnail_height", 0);
            data.url = j.value("url", "");
            data.html = j.value("html", "");
            data.width = j.value("width", 0);
            data.height = j.value("height", 0);
        } catch (...) {}
        return data;
    }

    bool isOEmbedSupported(const std::string& url) const {
        // Known oEmbed providers
        static const std::vector<std::string> knownDomains = {
            "youtube.com", "youtu.be", "vimeo.com", "soundcloud.com",
            "slideshare.net", "flickr.com", "twitter.com", "instagram.com",
            "tiktok.com", "spotify.com", "giphy.com", "imgur.com",
            "reddit.com", "codepen.io", "dailymotion.com"
        };
        for (const auto& domain : knownDomains) {
            if (url.find(domain) != std::string::npos) return true;
        }
        return false;
    }
};

// ────────────────────────────────────────────────────────────
// FaviconExtractor: favicon URL discovery
// ────────────────────────────────────────────────────────────
class FaviconExtractor {
public:
    std::string extract(const std::string& html, const std::string& baseUrl) {
        HtmlParser parser(html);
        auto links = parser.extractLinks();

        // Priority: apple-touch-icon > icon > shortcut icon
        std::vector<std::string> appleIcons, regularIcons;
        for (const auto& link : links) {
            if (link.find("apple-touch-icon") != std::string::npos) {
                appleIcons.push_back(link);
            } else {
                regularIcons.push_back(link);
            }
        }

        std::string chosen = "";
        if (!appleIcons.empty()) chosen = appleIcons[0];
        else if (!regularIcons.empty()) chosen = regularIcons[0];
        else {
            // Fallback: try /favicon.ico relative to base URL
            if (!baseUrl.empty()) {
                chosen = resolveUrl(baseUrl, "/favicon.ico");
            }
        }

        if (!chosen.empty() && chosen.find("://") == std::string::npos) {
            chosen = resolveUrl(baseUrl, chosen);
        }

        return chosen;
    }

private:
    std::string resolveUrl(const std::string& base, const std::string& relative) const {
        if (relative.find("://") != std::string::npos) return relative;
        if (relative.empty()) return base;

        // Extract base scheme + host
        size_t schemeEnd = base.find("://");
        if (schemeEnd == std::string::npos) return relative;

        size_t hostStart = schemeEnd + 3;
        size_t hostEnd = base.find('/', hostStart);
        if (hostEnd == std::string::npos) hostEnd = base.size();

        std::string baseOrigin = base.substr(0, hostEnd);

        if (relative[0] == '/') {
            return baseOrigin + relative;
        } else {
            // Relative to current path
            size_t lastSlash = base.rfind('/');
            if (lastSlash > hostEnd) {
                return base.substr(0, lastSlash + 1) + relative;
            }
            return baseOrigin + "/" + relative;
        }
    }
};

// ────────────────────────────────────────────────────────────
// ImageDimensionDetector: determine image dimensions
// ────────────────────────────────────────────────────────────
class ImageDimensionDetector {
public:
    struct ImageDimensions {
        int width = 0;
        int height = 0;
        int64_t fileSize = 0;
        std::string mimeType;
    };

    // Detect from known URL patterns (common CDNs and services)
    ImageDimensions detectFromUrl(const std::string& url) {
        ImageDimensions dims;

        // Try extracting dimensions from URL patterns
        // Example: image_800x600.jpg, w=800&h=600, etc.
        std::regex dimRegex(R"((\d{2,5})[x_](\d{2,5}))");
        std::smatch match;
        if (std::regex_search(url, match, dimRegex)) {
            dims.width = std::stoi(match[1].str());
            dims.height = std::stoi(match[2].str());
        }

        // Try w= and h= query parameters
        std::regex wRegex(R"([?&]w=(\d+))");
        std::regex hRegex(R"([?&]h=(\d+))");
        if (std::regex_search(url, match, wRegex)) dims.width = std::stoi(match[1].str());
        if (std::regex_search(url, match, hRegex)) dims.height = std::stoi(match[1].str());

        // Detect mime type from extension
        dims.mimeType = detectMimeFromUrl(url);

        return dims;
    }

    // Detect dimensions from raw image bytes (stub)
    ImageDimensions detectFromBytes(const std::vector<uint8_t>& data) {
        ImageDimensions dims;
        if (data.size() < 16) return dims;

        dims.fileSize = static_cast<int64_t>(data.size());

        // PNG: 8 bytes signature, then IHDR at offset 16 (width:4, height:4)
        if (data.size() >= 24 &&
            data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) {
            dims.mimeType = "image/png";
            dims.width = (data[16] << 24) | (data[17] << 16) | (data[18] << 8) | data[19];
            dims.height = (data[20] << 24) | (data[21] << 16) | (data[22] << 8) | data[23];
        }
        // JPEG: search for SOF0 marker (0xFF 0xC0)
        else if (data[0] == 0xFF && data[1] == 0xD8) {
            dims.mimeType = "image/jpeg";
            size_t i = 2;
            while (i + 9 < data.size()) {
                if (data[i] == 0xFF) {
                    uint8_t marker = data[i + 1];
                    if (marker == 0xC0 || marker == 0xC2) {  // SOF0 or SOF2
                        dims.height = (data[i + 5] << 8) | data[i + 6];
                        dims.width = (data[i + 7] << 8) | data[i + 8];
                        break;
                    }
                    uint16_t segmentLen = (data[i + 2] << 8) | data[i + 3];
                    i += segmentLen + 2;
                } else {
                    break;
                }
            }
        }
        // GIF: bytes 6-7 width, 8-9 height
        else if (data.size() >= 10 &&
                 data[0] == 'G' && data[1] == 'I' && data[2] == 'F') {
            dims.mimeType = "image/gif";
            dims.width = data[6] | (data[7] << 8);
            dims.height = data[8] | (data[9] << 8);
        }
        // WebP: RIFF header then 'WEBP', VP8/VP8L/VP8X chunk
        else if (data.size() >= 30 &&
                 data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F' &&
                 data[8] == 'W' && data[9] == 'E' && data[10] == 'B' && data[11] == 'P') {
            dims.mimeType = "image/webp";
            if (data[12] == 'V' && data[13] == 'P' && data[14] == '8' &&
                (data[15] == ' ' || data[15] == 'X' || data[15] == 'L')) {
                if (data[15] == ' ') {
                    // Lossy VP8
                    dims.width = (data[26] | (data[27] << 8)) & 0x3FFF;
                    dims.height = (data[28] | (data[29] << 8)) & 0x3FFF;
                } else if (data[15] == 'L') {
                    // Lossless VP8L
                    uint32_t bits = data[21] | (data[22] << 8) | (data[23] << 16) | (data[24] << 24);
                    dims.width = (bits & 0x3FFF) + 1;
                    dims.height = ((bits >> 14) & 0x3FFF) + 1;
                } else if (data[15] == 'X') {
                    // Extended VP8X
                    dims.width = (data[24] | (data[25] << 8) | (data[26] << 16)) + 1;
                    dims.height = (data[27] | (data[28] << 8) | (data[29] << 16)) + 1;
                }
            }
        }
        // BMP
        else if (data.size() >= 26 &&
                 data[0] == 'B' && data[1] == 'M') {
            dims.mimeType = "image/bmp";
            dims.width = data[18] | (data[19] << 8) | (data[20] << 16) | (data[21] << 24);
            dims.height = data[22] | (data[23] << 8) | (data[24] << 16) | (data[25] << 24);
        }

        return dims;
    }

private:
    std::string detectMimeFromUrl(const std::string& url) const {
        static const std::map<std::string, std::string> extensionMap = {
            {".jpg", "image/jpeg"},  {".jpeg", "image/jpeg"},
            {".png", "image/png"},   {".gif", "image/gif"},
            {".webp", "image/webp"}, {".bmp", "image/bmp"},
            {".svg", "image/svg+xml"}, {".ico", "image/x-icon"},
        };
        std::string lower = url;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        for (const auto& [ext, mime] : extensionMap) {
            if (lower.find(ext) != std::string::npos) return mime;
        }
        return "image/jpeg"; // default
    }
};

// ───────────────────────────────────────────────────────────ー
// PreviewSizeCalculator: calculate appropriate preview dimensions
// ────────────────────────────────────────────────────────────
class PreviewSizeCalculator {
public:
    struct PreviewSize {
        int width = 0;
        int height = 0;
        bool needsResize = false;
    };

    // Matrix recommends max 640x480 for thumbnails
    static constexpr int MAX_WIDTH = 640;
    static constexpr int MAX_HEIGHT = 480;

    PreviewSize calculate(int originalWidth, int originalHeight) const {
        PreviewSize size;
        if (originalWidth <= 0 || originalHeight <= 0) {
            size.width = MAX_WIDTH;
            size.height = MAX_HEIGHT;
            size.needsResize = false;
            return size;
        }

        if (originalWidth <= MAX_WIDTH && originalHeight <= MAX_HEIGHT) {
            size.width = originalWidth;
            size.height = originalHeight;
            size.needsResize = false;
        } else {
            double scaleW = static_cast<double>(MAX_WIDTH) / originalWidth;
            double scaleH = static_cast<double>(MAX_HEIGHT) / originalHeight;
            double scale = std::min(scaleW, scaleH);
            size.width = static_cast<int>(originalWidth * scale);
            size.height = static_cast<int>(originalHeight * scale);
            size.needsResize = true;
        }
        return size;
    }

    // For Twitter card summary_large_image: 2:1 aspect ratio
    bool isLargeImageRatio(int width, int height) const {
        if (width <= 0 || height <= 0) return false;
        double ratio = static_cast<double>(width) / height;
        return ratio >= 1.8 && ratio <= 2.2;
    }
};

// ────────────────────────────────────────────────────────────
// LinkPreviewCache: in-memory LRU cache for previews
// ───────────────────────────────────────────────────────────ー
class LinkPreviewCache {
public:
    struct CacheConfig {
        size_t maxEntries = 200;
        int64_t ttlSeconds = 3600;  // 1 hour default TTL
    };

    explicit LinkPreviewCache(const CacheConfig& config = {}) : config_(config) {}

    std::optional<UrlPreviewResult> get(const std::string& url) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(url);
        if (it == cache_.end()) return std::nullopt;

        auto now = std::chrono::steady_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.fetchTime).count();
        if (age > config_.ttlSeconds) {
            cache_.erase(it);
            return std::nullopt;
        }

        // Move to front of LRU
        updateAccess(url);
        return it->second.result;
    }

    void put(const std::string& url, const UrlPreviewResult& result) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();

        CacheEntry entry;
        entry.result = result;
        entry.fetchTime = now;

        cache_[url] = entry;
        lruOrder_.push_back(url);

        // Evict oldest if over limit
        while (cache_.size() > config_.maxEntries) {
            std::string oldest = lruOrder_.front();
            lruOrder_.pop_front();
            cache_.erase(oldest);
        }
    }

    void remove(const std::string& url) {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.erase(url);
        lruOrder_.remove(url);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
        lruOrder_.clear();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }

private:
    struct CacheEntry {
        UrlPreviewResult result;
        std::chrono::steady_clock::time_point fetchTime;
    };

    void updateAccess(const std::string& url) {
        lruOrder_.remove(url);
        lruOrder_.push_back(url);
    }

    CacheConfig config_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, CacheEntry> cache_;
    std::list<std::string> lruOrder_;
};

// ────────────────────────────────────────────────────────────
// UrlPreviewServiceImpl: main URL preview service implementation
// ────────────────────────────────────────────────────────────
class UrlPreviewServiceImpl {
public:
    UrlPreviewServiceImpl()
        : cache_(std::make_unique<LinkPreviewCache>()),
          ogParser_(std::make_unique<OpenGraphParser>()),
          twitterParser_(std::make_unique<TwitterCardParser>()),
          oembedParser_(std::make_unique<OEmbedParser>()),
          faviconExtractor_(std::make_unique<FaviconExtractor>()),
          dimDetector_(std::make_unique<ImageDimensionDetector>()),
          sizeCalc_(std::make_unique<PreviewSizeCalculator>()) {}

    UrlPreviewResult fetchPreview(const std::string& url, const std::string& html) {
        // Check cache first
        auto cached = cache_->get(url);
        if (cached.has_value()) {
            return cached.value();
        }

        UrlPreviewResult result;
        result.url = url;
        result.fetchTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        // Parse HTML metadata
        HtmlParser htmlParser(html);

        // 1. Extract title
        result.title = htmlParser.extractTitle();

        // 2. Parse OpenGraph
        auto ogData = ogParser_->parse(html);
        if (!ogData.title.empty() && result.title.empty()) result.title = ogData.title;
        if (!ogData.description.empty()) result.description = ogData.description;
        if (!ogData.image.empty()) result.imageUrl = ogData.image;
        if (!ogData.siteName.empty()) result.siteName = ogData.siteName;
        if (!ogData.type.empty()) result.ogType = ogData.type;
        if (!ogData.locale.empty()) result.ogLocale = ogData.locale;
        result.imageWidth = ogData.imageWidth;
        result.imageHeight = ogData.imageHeight;

        // 3. Parse Twitter Cards
        auto twitterCard = twitterParser_->parse(html);
        if (result.title.empty() && !twitterCard.title.empty()) result.title = twitterCard.title;
        if (result.description.empty() && !twitterCard.description.empty())
            result.description = twitterCard.description;
        if (result.imageUrl.empty() && !twitterCard.image.empty())
            result.imageUrl = twitterCard.image;
        if (!twitterCard.card.empty()) result.twitterCard = twitterCard.card;
        if (result.imageWidth == 0 && twitterCard.imageWidth > 0)
            result.imageWidth = twitterCard.imageWidth;
        if (result.imageHeight == 0 && twitterCard.imageHeight > 0)
            result.imageHeight = twitterCard.imageHeight;

        // 4. Fallback: extract meta description
        if (result.description.empty()) {
            result.description = htmlParser.extractDescription();
        }

        // 5. Fallback: extract first image from body
        if (result.imageUrl.empty()) {
            auto images = htmlParser.extractImages();
            if (!images.empty()) {
                result.imageUrl = images[0].src;
                if (images[0].width > 0) result.imageWidth = images[0].width;
                if (images[0].height > 0) result.imageHeight = images[0].height;
            }
        }

        // 6. Extract favicon
        result.faviconUrl = faviconExtractor_->extract(html, url);

        // 7. Detect image dimensions from URL pattern if still unknown
        if (result.imageWidth == 0 && !result.imageUrl.empty()) {
            auto dims = dimDetector_->detectFromUrl(result.imageUrl);
            if (dims.width > 0) {
                result.imageWidth = dims.width;
                result.imageHeight = dims.height;
            }
        }

        // 8. Validate result
        result.valid = !result.title.empty() || !result.description.empty() ||
                       !result.imageUrl.empty();

        // Cache the result
        cache_->put(url, result);

        return result;
    }

    // Check if URL is supported for preview
    bool isPreviewable(const std::string& url) const {
        if (url.empty()) return false;
        if (url.find("http://") == 0 || url.find("https://") == 0) return true;
        return false;
    }

    // Format as Matrix-compatible body
    std::string toMatrixFormattedBody(const UrlPreviewResult& preview) const {
        std::ostringstream os;
        os << "<blockquote>";
        if (!preview.imageUrl.empty()) {
            os << "<img src=\"" << preview.imageUrl
               << "\" alt=\"" << preview.title << "\"";
            if (preview.imageWidth > 0 && preview.imageHeight > 0) {
                os << " width=\"" << preview.imageWidth
                   << "\" height=\"" << preview.imageHeight << "\"";
            }
            os << "/>";
        }
        os << "<a href=\"" << preview.url << "\">"
           << (preview.title.empty() ? preview.url : preview.title)
           << "</a><br/>"
           << (preview.description.empty() ? preview.url : preview.description);
        if (!preview.siteName.empty()) {
            os << "<br/><small>" << preview.siteName << "</small>";
        }
        os << "</blockquote>";
        return os.str();
    }

    void clearCache() { cache_->clear(); }
    size_t cacheSize() const { return cache_->size(); }

private:
    std::unique_ptr<LinkPreviewCache> cache_;
    std::unique_ptr<OpenGraphParser> ogParser_;
    std::unique_ptr<TwitterCardParser> twitterParser_;
    std::unique_ptr<OEmbedParser> oembedParser_;
    std::unique_ptr<FaviconExtractor> faviconExtractor_;
    std::unique_ptr<ImageDimensionDetector> dimDetector_;
    std::unique_ptr<PreviewSizeCalculator> sizeCalc_;
};

} // namespace matrix_sdk
