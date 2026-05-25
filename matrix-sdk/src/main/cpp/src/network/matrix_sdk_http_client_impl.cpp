#include "matrix_sdk/network/matrix_sdk_network.hpp"
#include "matrix_sdk/matrix_sdk_types.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>
#include <thread>

namespace matrix_sdk {

using json = nlohmann::json;

// ────────────────────────────────────────────────────────────
// Forward declarations
// ────────────────────────────────────────────────────────────
static std::string urlEncode(const std::string& value);
static std::string methodToString(HttpMethod method);
static std::string generateRequestId();

// ────────────────────────────────────────────────────────────
// HttpUrl: URL construction and parsing helpers
// ────────────────────────────────────────────────────────────
struct HttpUrlParts {
    std::string scheme;
    std::string host;
    int port = 0;
    std::string path;
    std::string query;
};

static HttpUrlParts parseUrl(const std::string& url) {
    HttpUrlParts parts;
    const char* p = url.c_str();
    // Scheme
    const char* schemeStart = p;
    while (*p && *p != ':') ++p;
    parts.scheme = std::string(schemeStart, p - schemeStart);
    if (*p == ':') p++;
    if (*p == '/' && *(p + 1) == '/') p += 2;
    // Host
    const char* hostStart = p;
    while (*p && *p != ':' && *p != '/' && *p != '?') ++p;
    parts.host = std::string(hostStart, p - hostStart);
    // Port
    if (*p == ':') {
        ++p;
        parts.port = 0;
        while (*p >= '0' && *p <= '9') {
            parts.port = parts.port * 10 + (*p - '0');
            ++p;
        }
    }
    if (parts.port == 0) {
        parts.port = (parts.scheme == "https") ? 443 : 80;
    }
    // Path
    if (*p == '/') {
        const char* pathStart = p;
        while (*p && *p != '?') ++p;
        parts.path = std::string(pathStart, p - pathStart);
    } else {
        parts.path = "/";
    }
    // Query
    if (*p == '?') {
        ++p;
        parts.query = std::string(p);
    }
    return parts;
}

static std::string buildUrl(const std::string& baseUrl, const std::string& path,
                            const std::map<std::string, std::string>& queryParams) {
    std::ostringstream url;
    // Strip trailing slash from baseUrl
    std::string base = baseUrl;
    while (!base.empty() && base.back() == '/') base.pop_back();
    url << base;
    // Ensure path starts with '/'
    if (!path.empty() && path[0] != '/') url << '/';
    url << path;
    // Append query params
    bool first = (path.find('?') == std::string::npos);
    for (const auto& [key, value] : queryParams) {
        url << (first ? '?' : '&');
        url << urlEncode(key) << '=' << urlEncode(value);
        first = false;
    }
    return url.str();
}

static std::string urlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (unsigned char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else if (c == ' ') {
            escaped << "%20";
        } else {
            escaped << '%' << std::setw(2) << static_cast<int>(c);
        }
    }
    return escaped.str();
}

static std::string methodToString(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET:     return "GET";
        case HttpMethod::POST:    return "POST";
        case HttpMethod::PUT:     return "PUT";
        case HttpMethod::DELETE:  return "DELETE";
        case HttpMethod::PATCH:   return "PATCH";
        case HttpMethod::HEAD:    return "HEAD";
        case HttpMethod::OPTIONS: return "OPTIONS";
        default:                  return "GET";
    }
}

static std::string generateRequestId() {
    static std::atomic<uint64_t> counter{0};
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    uint64_t id = counter.fetch_add(1);
    std::ostringstream oss;
    oss << std::hex << now << "-" << id;
    return oss.str();
}

// ────────────────────────────────────────────────────────────
// SocketBasedHttpClient: Internal socket-level HTTP implementation
// ────────────────────────────────────────────────────────────
class SocketBasedHttpClient {
public:
    struct SocketConfig {
        int connectTimeoutMs = 10000;
        int readTimeoutMs = 30000;
        int writeTimeoutMs = 10000;
        int maxConnections = 16;
        int keepAliveTimeoutSec = 30;
        bool enableKeepAlive = true;
        bool enableTls = true;
        bool verifyTls = true;
        std::string caCertPath;
        std::string clientCertPath;
        std::string clientKeyPath;
        std::string proxyHost;
        int proxyPort = 0;
        std::string proxyUser;
        std::string proxyPass;
        bool proxyEnabled = false;
    };

    explicit SocketBasedHttpClient(const SocketConfig& config) : config_(config) {}

    HttpResponse execute(const HttpRequest& request) {
        HttpResponse response;
        auto startTime = std::chrono::steady_clock::now();

        // Parse the target URL
        bool useProxy = config_.proxyEnabled && !config_.proxyHost.empty();
        std::string targetUrl = useProxy ?
            buildProxyUrl(request.url) : request.url;
        auto parts = parseUrl(request.url);

        // Build raw HTTP request
        std::string rawRequest = buildRawRequest(request, parts);

        // Determine host/port to connect to
        std::string connectHost = useProxy ? config_.proxyHost : parts.host;
        int connectPort = useProxy ? config_.proxyPort : parts.port;

        // Execute via socket or libcurl (platform-dependent)
        bool useTls = (parts.scheme == "https") || (useProxy && config_.enableTls);
        if (!executeRequestPlatform(connectHost, connectPort, useTls,
                                     rawRequest, response)) {
            response.httpCode = -1;
            response.errorMessage = "Connection failed";
        }

        auto endTime = std::chrono::steady_clock::now();
        response.requestDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime).count();

        // Parse response headers for retry-after
        parseRetryAfter(response);

        return response;
    }

private:
    SocketConfig config_;

    std::string buildProxyUrl(const std::string& targetUrl) const {
        // For HTTP proxy, we send the full URL in the request line
        return targetUrl;
    }

    std::string buildRawRequest(const HttpRequest& request, const HttpUrlParts& parts) const {
        std::ostringstream raw;
        // Request line: METHOD path HTTP/1.1
        std::string requestPath = parts.path;
        if (!parts.query.empty()) requestPath += "?" + parts.query;

        raw << methodToString(request.method) << " "
            << requestPath << " HTTP/1.1\r\n";

        // Host header
        raw << "Host: " << parts.host;
        if (parts.port != 80 && parts.port != 443) {
            raw << ":" << parts.port;
        }
        raw << "\r\n";

        // User-Agent
        raw << "User-Agent: matrix-sdk-cpp/1.0\r\n";

        // Connection
        raw << "Connection: " << (config_.enableKeepAlive ? "keep-alive" : "close") << "\r\n";
        if (config_.enableKeepAlive) {
            raw << "Keep-Alive: timeout=" << config_.keepAliveTimeoutSec << "\r\n";
        }

        // Content-Type (default to application/json for Matrix API)
        bool hasContentType = false;
        for (const auto& [key, value] : request.headers) {
            std::string lowerKey = key;
            std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
            if (lowerKey == "host") continue; // skip — we manage it
            raw << key << ": " << value << "\r\n";
            if (lowerKey == "content-type") hasContentType = true;
        }

        // Default Content-Type for JSON bodies
        if (!hasContentType && !request.body.empty() &&
            (request.method == HttpMethod::POST || request.method == HttpMethod::PUT ||
             request.method == HttpMethod::PATCH)) {
            // Auto-detect JSON
            if (!request.body.empty() && (request.body[0] == '{' || request.body[0] == '[')) {
                raw << "Content-Type: application/json\r\n";
            } else {
                raw << "Content-Type: application/x-www-form-urlencoded\r\n";
            }
        }

        // Content-Length
        raw << "Content-Length: " << request.body.size() << "\r\n";

        // Accept
        raw << "Accept: application/json, */*\r\n";

        // Empty line to end headers
        raw << "\r\n";

        // Body
        if (!request.body.empty()) {
            raw << request.body;
        }

        return raw.str();
    }

    bool executeRequestPlatform(const std::string& host, int port, bool useTls,
                                 const std::string& rawRequest, HttpResponse& response) {
        // Platform-independent socket implementation
        // In a real build, this would be conditionally compiled for:
        // - POSIX sockets (Linux/macOS/Android)
        // - WinSock (Windows)
        // - Or link against libcurl

        int sockfd = -1;
        bool success = false;

        // Create socket
        sockfd = socketCreate(useTls);
        if (sockfd < 0) {
            response.errorMessage = "Failed to create socket";
            return false;
        }

        // Set timeouts
        socketSetTimeout(sockfd, config_.connectTimeoutMs, config_.readTimeoutMs);

        // Resolve and connect
        if (!socketConnect(sockfd, host, port, config_.connectTimeoutMs)) {
            response.errorMessage = "Failed to connect to " + host + ":" + std::to_string(port);
            socketClose(sockfd);
            return false;
        }

        // TLS handshake if needed
        if (useTls) {
            if (!socketTlsHandshake(sockfd, host)) {
                response.errorMessage = "TLS handshake failed";
                socketClose(sockfd);
                return false;
            }
        }

        // Send request
        if (!socketSend(sockfd, rawRequest)) {
            response.errorMessage = "Failed to send request";
            socketClose(sockfd);
            return false;
        }

        // Receive response
        std::string rawResponse;
        if (!socketReceive(sockfd, rawResponse, config_.readTimeoutMs)) {
            response.errorMessage = "Failed to receive response";
            socketClose(sockfd);
            return false;
        }

        // Parse HTTP response
        parseRawResponse(rawResponse, response);

        // Keep-alive: close if server requested
        if (config_.enableKeepAlive) {
            auto it = response.headers.find("connection");
            if (it != response.headers.end()) {
                std::string conn = it->second;
                std::transform(conn.begin(), conn.end(), conn.begin(), ::tolower);
                if (conn == "close") {
                    socketClose(sockfd);
                } else {
                    // Return to connection pool
                    socketClose(sockfd); // Simplified: close for now
                }
            }
        } else {
            socketClose(sockfd);
        }

        success = true;
        return success;
    }

    // ── Platform socket stubs ──
    int socketCreate(bool useTls) {
        // Stub: in production, create AF_INET/SOCK_STREAM socket
        // and wrap in TLS context if useTls is true
        return 1; // Dummy valid fd
    }

    void socketSetTimeout(int fd, int connectMs, int readMs) {
        // Stub: setsockopt SO_RCVTIMEO / SO_SNDTIMEO
    }

    bool socketConnect(int fd, const std::string& host, int port, int timeoutMs) {
        // Stub: getaddrinfo + connect with non-blocking + poll/select for timeout
        // Resolve DNS, attempt connection
        return true; // Assume success for stub
    }

    bool socketTlsHandshake(int fd, const std::string& hostname) {
        // Stub: OpenSSL SSL_connect or similar
        // Perform certificate verification if config_.verifyTls
        // Apply certificate pinning if configured
        return true;
    }

    bool socketSend(int fd, const std::string& data) {
        // Stub: send() in a loop until all data sent
        return true;
    }

    bool socketReceive(int fd, std::string& outData, int timeoutMs) {
        // Stub: recv() in a loop, parse HTTP response boundaries
        // Handle chunked transfer encoding
        // Handle content-length based reads
        return true;
    }

    void socketClose(int fd) {
        // Stub: close/shutdown socket
    }

    // ── Response parsing ──
    void parseRawResponse(const std::string& raw, HttpResponse& response) {
        // Parse HTTP status line: "HTTP/1.1 200 OK"
        size_t statusEnd = raw.find("\r\n");
        if (statusEnd == std::string::npos) return;

        std::string statusLine = raw.substr(0, statusEnd);
        // Extract status code
        size_t codeStart = statusLine.find(' ');
        if (codeStart == std::string::npos) return;
        size_t codeEnd = statusLine.find(' ', codeStart + 1);
        std::string codeStr = statusLine.substr(codeStart + 1, codeEnd - codeStart - 1);
        response.httpCode = std::stoi(codeStr);

        // Parse headers
        size_t headerEnd = raw.find("\r\n\r\n", statusEnd + 2);
        if (headerEnd == std::string::npos) return;

        std::string headerBlock = raw.substr(statusEnd + 2, headerEnd - (statusEnd + 2));
        std::istringstream headerStream(headerBlock);
        std::string headerLine;
        while (std::getline(headerStream, headerLine)) {
            if (!headerLine.empty() && headerLine.back() == '\r') headerLine.pop_back();
            if (headerLine.empty()) break;
            size_t colonPos = headerLine.find(':');
            if (colonPos != std::string::npos) {
                std::string key = headerLine.substr(0, colonPos);
                std::string value = headerLine.substr(colonPos + 1);
                // Trim leading whitespace from value
                while (!value.empty() && value[0] == ' ') value.erase(0, 1);
                std::string lowerKey = key;
                std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
                response.headers[lowerKey] = value;
            }
        }

        // Body
        size_t bodyStart = headerEnd + 4;
        response.body = raw.substr(bodyStart);

        // Handle chunked transfer encoding
        auto it = response.headers.find("transfer-encoding");
        if (it != response.headers.end() && it->second.find("chunked") != std::string::npos) {
            response.body = decodeChunkedBody(response.body);
        }
    }

    std::string decodeChunkedBody(const std::string& chunked) const {
        std::string decoded;
        std::istringstream stream(chunked);
        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            size_t chunkSize = std::stoul(line, nullptr, 16);
            if (chunkSize == 0) break;
            std::vector<char> buffer(chunkSize + 2);
            stream.read(buffer.data(), static_cast<std::streamsize>(chunkSize + 2));
            decoded.append(buffer.data(), chunkSize);
        }
        return decoded;
    }

    void parseRetryAfter(HttpResponse& response) {
        auto it = response.headers.find("retry-after");
        if (it != response.headers.end()) {
            const std::string& value = it->second;
            try {
                response.retryAfterMs = std::stoi(value) * 1000; // seconds to ms
            } catch (...) {
                // Try parsing as HTTP-date or just default
                response.retryAfterMs = 5000;
            }
        }
    }
};

// ────────────────────────────────────────────────────────────
// ConnectionPool: manage persistent socket connections
// ────────────────────────────────────────────────────────────
class ConnectionPool {
public:
    struct PooledConnection {
        std::string host;
        int port = 0;
        int fd = -1;
        std::chrono::steady_clock::time_point lastUsed;
        bool inUse = false;
    };

    explicit ConnectionPool(int maxConnections = 16, int idleTimeoutSec = 60)
        : maxConnections_(maxConnections), idleTimeoutSec_(idleTimeoutSec) {}

    ~ConnectionPool() { drain(); }

    int acquire(const std::string& host, int port) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();

        // Look for existing idle connection
        for (auto& conn : connections_) {
            if (!conn.inUse && conn.host == host && conn.port == port) {
                auto idleDuration = std::chrono::duration_cast<std::chrono::seconds>(
                    now - conn.lastUsed).count();
                if (idleDuration < idleTimeoutSec_) {
                    conn.inUse = true;
                    return conn.fd;
                }
            }
        }
        return -1; // No available connection
    }

    void release(const std::string& host, int port, int fd, bool keepAlive) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (keepAlive) {
            for (auto& conn : connections_) {
                if (conn.fd == fd) {
                    conn.inUse = false;
                    conn.lastUsed = std::chrono::steady_clock::now();
                    return;
                }
            }
            // Add new pooled connection
            if (static_cast<int>(connections_.size()) < maxConnections_) {
                connections_.push_back({
                    host, port, fd,
                    std::chrono::steady_clock::now(), false
                });
            }
        }
    }

    void drain() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& conn : connections_) {
            // Close socket
        }
        connections_.clear();
    }

    void evictIdleConnections() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                [&](const PooledConnection& conn) {
                    if (conn.inUse) return false;
                    auto idle = std::chrono::duration_cast<std::chrono::seconds>(
                        now - conn.lastUsed).count();
                    return idle >= idleTimeoutSec_;
                }),
            connections_.end());
    }

    int activeCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        int count = 0;
        for (const auto& conn : connections_) {
            if (conn.inUse) count++;
        }
        return count;
    }

private:
    mutable std::mutex mutex_;
    std::vector<PooledConnection> connections_;
    int maxConnections_;
    int idleTimeoutSec_;
};

// ────────────────────────────────────────────────────────────
// TlsConfig: TLS/certificate configuration
// ────────────────────────────────────────────────────────────
struct TlsConfig {
    bool useTls = true;
    bool verifyCertificates = true;
    std::string caBundlePath;
    std::string clientCertPath;
    std::string clientKeyPath;
    // Certificate pinning
    bool enablePinning = false;
    std::vector<std::string> pinnedPublicKeys;  // SHA256 hashes
    std::vector<std::string> pinnedSpkiFingerprints;

    bool shouldPin() const { return enablePinning && !pinnedPublicKeys.empty(); }

    bool verifyPin(const std::string& publicKeyHash) const {
        if (!enablePinning) return true;
        return std::find(pinnedPublicKeys.begin(), pinnedPublicKeys.end(),
                         publicKeyHash) != pinnedPublicKeys.end();
    }
};

// ────────────────────────────────────────────────────────────
// ProxyConfig
// ────────────────────────────────────────────────────────────
struct ProxyConfig {
    bool enabled = false;
    std::string host;
    int port = 0;
    std::string username;
    std::string password;
    enum Type { HTTP, SOCKS5, NONE } type = NONE;

    bool isConfigured() const { return enabled && !host.empty() && port > 0; }

    std::string proxyAuthorizationHeader() const {
        if (username.empty()) return "";
        std::string credentials = username + ":" + password;
        return "Basic " + base64Encode(credentials);
    }

private:
    static std::string base64Encode(const std::string& input) {
        static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string encoded;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(table[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) encoded.push_back(table[((val << 8) >> (valb + 8)) & 0x3F]);
        while (encoded.size() % 4) encoded.push_back('=');
        return encoded;
    }
};

// ────────────────────────────────────────────────────────────
// RetryPolicy: exponential backoff retry logic
// ────────────────────────────────────────────────────────────
class RetryPolicy {
public:
    struct Config {
        int maxRetries = 3;
        int initialBackoffMs = 1000;
        int maxBackoffMs = 30000;
        double backoffMultiplier = 2.0;
        double jitter = 0.1; // +/- 10% jitter
        bool retryOnConnectionError = true;
        bool retryOnRateLimit = true;
        bool retryOnServerError = true;  // 5xx
    };

    explicit RetryPolicy(const Config& config) : config_(config) {
        rng_.seed(std::random_device{}());
    }

    bool shouldRetry(int attempt, const HttpResponse& response, bool connectionError) const {
        if (attempt >= config_.maxRetries) return false;

        // Connection errors
        if (connectionError && config_.retryOnConnectionError) return true;

        // Rate limiting
        if (response.isRateLimited() && config_.retryOnRateLimit) return true;

        // Server errors (5xx)
        if (response.httpCode >= 500 && response.httpCode < 600 && config_.retryOnServerError)
            return true;

        return false;
    }

    int getRetryDelayMs(int attempt) const {
        // Exponential backoff: initial * multiplier^attempt
        double backoff = config_.initialBackoffMs *
            std::pow(config_.backoffMultiplier, attempt);
        backoff = std::min(backoff, static_cast<double>(config_.maxBackoffMs));

        // Apply jitter
        std::uniform_real_distribution<double> jitterDist(-config_.jitter, config_.jitter);
        backoff *= (1.0 + jitterDist(rng_));

        return static_cast<int>(backoff);
    }

private:
    Config config_;
    mutable std::mt19937 rng_;
};

// ────────────────────────────────────────────────────────────
// ResponseBuffer: buffering response data during streaming reads
// ────────────────────────────────────────────────────────────
class ResponseBuffer {
public:
    void append(const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_ << data;
    }

    void append(const char* data, size_t length) {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_.write(data, static_cast<std::streamsize>(length));
    }

    std::string str() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.str();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_.str("");
        buffer_.clear();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.tellp();
    }

private:
    mutable std::mutex mutex_;
    std::ostringstream buffer_;
};

// ────────────────────────────────────────────────────────────
// HttpClientImpl: Full HttpClient implementation
// ────────────────────────────────────────────────────────────
class HttpClientImpl : public HttpClient {
public:
    HttpClientImpl()
        : requestCounter_(0),
          activeRequests_(0),
          online_(true),
          userAgent_("matrix-sdk-cpp/1.0 (Linux; Android)") {
        // Default headers
        defaultHeaders_["Accept"] = "application/json";
        defaultHeaders_["Accept-Encoding"] = "gzip, deflate";
        defaultHeaders_["User-Agent"] = userAgent_;

        // Default socket config
        SocketBasedHttpClient::SocketConfig sockConfig;
        sockConfig.connectTimeoutMs = connectTimeoutMs_;
        sockConfig.readTimeoutMs = readTimeoutMs_;
        sockConfig.writeTimeoutMs = writeTimeoutMs_;
        sockConfig.enableKeepAlive = true;
        sockConfig.maxConnections = 16;
        socketClient_ = std::make_unique<SocketBasedHttpClient>(sockConfig);

        // Default retry policy
        RetryPolicy::Config retryConfig;
        retryConfig.maxRetries = 3;
        retryConfig.initialBackoffMs = 1000;
        retryConfig.maxBackoffMs = 30000;
        retryConfig.retryOnConnectionError = true;
        retryConfig.retryOnRateLimit = true;
        retryConfig.retryOnServerError = true;
        retryPolicy_ = std::make_unique<RetryPolicy>(retryConfig);

        connectionPool_ = std::make_unique<ConnectionPool>(16, 60);
    }

    ~HttpClientImpl() override { cancelAll(); }

    // ── Execute ──
    void execute(const HttpRequest& request, NetworkCallback callback) override {
        activeRequests_++;
        auto requestId = generateRequestId();
        {
            std::lock_guard<std::mutex> lock(requestsMutex_);
            pendingCallbacks_[requestId] = callback;
        }

        // Execute on background thread
        std::thread([this, request, requestId, callback]() {
            HttpRequest finalRequest = prepareRequest(request);
            HttpResponse response;
            bool connectionError = false;

            try {
                response = socketClient_->execute(finalRequest);
            } catch (const std::exception& e) {
                connectionError = true;
                response.httpCode = -1;
                response.errorMessage = e.what();
            }

            // Apply response to rate limiter if configured
            if (rateLimiter_ && response.isRateLimited()) {
                rateLimiter_->recordRequest(request.canonicalUrl.empty() ?
                    request.url : request.canonicalUrl, response.retryAfterMs);
            }

            // Remove from pending
            {
                std::lock_guard<std::mutex> lock(requestsMutex_);
                pendingCallbacks_.erase(requestId);
            }
            activeRequests_--;

            if (callback) {
                callback(response);
            }
        }).detach();
    }

    // ── Execute with retry ──
    void executeWithRetry(const HttpRequest& request, int maxRetries,
                           NetworkCallback callback) override {
        activeRequests_++;
        auto requestId = generateRequestId();
        {
            std::lock_guard<std::mutex> lock(requestsMutex_);
            pendingCallbacks_[requestId] = callback;
        }

        std::thread([this, request, maxRetries, requestId, callback]() {
            HttpRequest finalRequest = prepareRequest(request);
            HttpResponse response;
            bool connectionError = false;

            for (int attempt = 0; attempt <= maxRetries; ++attempt) {
                // Check rate limiter before each attempt
                std::string endpoint = request.canonicalUrl.empty() ?
                    request.url : request.canonicalUrl;
                if (rateLimiter_ && !rateLimiter_->canExecute(endpoint)) {
                    int waitMs = rateLimiter_->getRetryAfterMs(endpoint);
                    if (waitMs > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
                    }
                }

                try {
                    response = socketClient_->execute(finalRequest);
                    connectionError = false;
                } catch (const std::exception& e) {
                    connectionError = true;
                    response.httpCode = -1;
                    response.errorMessage = e.what();
                }

                if (rateLimiter_ && response.isRateLimited()) {
                    rateLimiter_->recordRequest(endpoint, response.retryAfterMs);
                }

                // Should we retry?
                bool shouldRetry = retryPolicy_->shouldRetry(attempt, response, connectionError);
                if (!shouldRetry) break;

                // Wait with exponential backoff
                int delayMs = retryPolicy_->getRetryDelayMs(attempt);
                if (response.retryAfterMs > 0) {
                    delayMs = std::max(delayMs, response.retryAfterMs);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }

            // Cleanup
            {
                std::lock_guard<std::mutex> lock(requestsMutex_);
                pendingCallbacks_.erase(requestId);
                cancelledRequests_.erase(requestId);
            }
            activeRequests_--;

            if (callback) {
                callback(response);
            }
        }).detach();
    }

    // ── Cancel ──
    void cancelAll() override {
        std::lock_guard<std::mutex> lock(requestsMutex_);
        cancelled_ = true;
        cancelledRequests_.clear();
        pendingCallbacks_.clear();
        activeRequests_ = 0;
    }

    void cancel(const std::string& requestId) override {
        std::lock_guard<std::mutex> lock(requestsMutex_);
        cancelledRequests_.insert(requestId);
        pendingCallbacks_.erase(requestId);
    }

    // ── Configuration ──
    void setBaseUrl(const std::string& baseUrl) override {
        baseUrl_ = baseUrl;
        // Rebuild endpoint builder with new base URL if needed
    }

    std::string getBaseUrl() const override { return baseUrl_; }

    void setTokenStore(std::shared_ptr<TokenStore> store) override {
        tokenStore_ = std::move(store);
    }

    void setUserAgent(const std::string& userAgent) override {
        userAgent_ = userAgent;
    }

    void addHeader(const std::string& key, const std::string& value) override {
        std::lock_guard<std::mutex> lock(headersMutex_);
        defaultHeaders_[key] = value;
    }

    void removeHeader(const std::string& key) override {
        std::lock_guard<std::mutex> lock(headersMutex_);
        defaultHeaders_.erase(key);
    }

    void setConnectTimeout(int timeoutMs) override {
        connectTimeoutMs_ = timeoutMs;
    }

    void setReadTimeout(int timeoutMs) override {
        readTimeoutMs_ = timeoutMs;
    }

    void setWriteTimeout(int timeoutMs) override {
        writeTimeoutMs_ = timeoutMs;
    }

    int getActiveRequestCount() const override {
        return activeRequests_.load();
    }

    bool isOnline() const override { return online_.load(); }

    // ── Additional configuration methods ──
    void setCertificatePinning(const std::vector<std::string>& fingerprints) {
        tlsConfig_.pinnedPublicKeys = fingerprints;
        tlsConfig_.enablePinning = !fingerprints.empty();
    }

    void setCaBundle(const std::string& caPath) {
        tlsConfig_.caBundlePath = caPath;
    }

    void setClientCertificate(const std::string& certPath, const std::string& keyPath) {
        tlsConfig_.clientCertPath = certPath;
        tlsConfig_.clientKeyPath = keyPath;
    }

    void setProxy(const std::string& host, int port,
                  const std::string& user = "", const std::string& pass = "") {
        proxyConfig_.enabled = true;
        proxyConfig_.host = host;
        proxyConfig_.port = port;
        proxyConfig_.username = user;
        proxyConfig_.password = pass;
        proxyConfig_.type = ProxyConfig::HTTP;
    }

    void clearProxy() {
        proxyConfig_.enabled = false;
        proxyConfig_.host.clear();
        proxyConfig_.port = 0;
    }

    void setRateLimiter(std::shared_ptr<RateLimiter> limiter) {
        rateLimiter_ = std::move(limiter);
    }

    void setRetryConfig(int maxRetries, int initialBackoffMs, int maxBackoffMs,
                         double multiplier = 2.0) {
        RetryPolicy::Config cfg;
        cfg.maxRetries = maxRetries;
        cfg.initialBackoffMs = initialBackoffMs;
        cfg.maxBackoffMs = maxBackoffMs;
        cfg.backoffMultiplier = multiplier;
        retryPolicy_ = std::make_unique<RetryPolicy>(cfg);
    }

private:
    // ── Internal helpers ──
    HttpRequest prepareRequest(const HttpRequest& request) const {
        HttpRequest req = request;

        // Apply base URL if set and URL is relative
        if (!baseUrl_.empty() && request.url.find("://") == std::string::npos) {
            req.url = buildUrl(baseUrl_, request.url, request.queryParams);
        }

        // Apply default headers
        {
            std::lock_guard<std::mutex> lock(headersMutex_);
            for (const auto& [key, value] : defaultHeaders_) {
                if (req.headers.find(key) == req.headers.end()) {
                    req.headers[key] = value;
                }
            }
        }

        // Apply authorization token
        if (tokenStore_ && tokenStore_->hasValidToken()) {
            if (req.headers.find("Authorization") == req.headers.end()) {
                req.headers["Authorization"] = "Bearer " + tokenStore_->getAccessToken();
            }
        }

        // Apply proxy configuration
        if (proxyConfig_.isConfigured() && !proxyConfig_.proxyAuthorizationHeader().empty()) {
            if (req.headers.find("Proxy-Authorization") == req.headers.end()) {
                req.headers["Proxy-Authorization"] = proxyConfig_.proxyAuthorizationHeader();
            }
        }

        // Apply TLS configuration
        if (tlsConfig_.shouldPin()) {
            req.headers["X-TLS-Pinned"] = "true";
        }

        return req;
    }

    // ── Members ──
    std::string baseUrl_;
    std::string userAgent_;
    std::atomic<int> requestCounter_;
    std::atomic<int> activeRequests_;
    std::atomic<bool> online_;
    std::atomic<bool> cancelled_{false};

    // Socket-based HTTP engine
    std::unique_ptr<SocketBasedHttpClient> socketClient_;
    std::unique_ptr<ConnectionPool> connectionPool_;
    std::unique_ptr<RetryPolicy> retryPolicy_;

    // Configuration
    int connectTimeoutMs_ = 10000;
    int readTimeoutMs_ = 30000;
    int writeTimeoutMs_ = 10000;
    TlsConfig tlsConfig_;
    ProxyConfig proxyConfig_;

    // Token and rate limiting
    std::shared_ptr<TokenStore> tokenStore_;
    std::shared_ptr<RateLimiter> rateLimiter_;

    // Default headers
    mutable std::mutex headersMutex_;
    std::map<std::string, std::string> defaultHeaders_;

    // Pending requests and cancellation tracking
    mutable std::mutex requestsMutex_;
    std::unordered_map<std::string, NetworkCallback> pendingCallbacks_;
    std::unordered_set<std::string> cancelledRequests_;
};

// ────────────────────────────────────────────────────────────
// RequestSignerImpl: signs outgoing HTTP requests
// ────────────────────────────────────────────────────────────
class RequestSignerImpl : public RequestSigner {
public:
    explicit RequestSignerImpl(std::shared_ptr<TokenStore> tokenStore)
        : tokenStore_(std::move(tokenStore)) {}

    void sign(HttpRequest& request) override {
        if (tokenStore_ && tokenStore_->hasValidToken()) {
            request.headers["Authorization"] = "Bearer " + tokenStore_->getAccessToken();
        }
    }

    bool canSign() const override {
        return tokenStore_ && tokenStore_->hasValidToken();
    }

private:
    std::shared_ptr<TokenStore> tokenStore_;
};

// ────────────────────────────────────────────────────────────
// NetworkMonitorImpl: monitors connectivity
// ────────────────────────────────────────────────────────────
class NetworkMonitorImpl : public NetworkMonitor {
public:
    NetworkMonitorImpl() : connected_(false), running_(false) {}

    ~NetworkMonitorImpl() override { stop(); }

    bool isConnected() const override { return connected_.load(); }

    void start() override {
        if (running_.exchange(true)) return;
        monitorThread_ = std::thread([this]() {
            while (running_.load()) {
                // Simple connectivity check — ping the homeserver or use platform API
                // In a real implementation, this would use Android's ConnectivityManager
                // or Linux netlink to detect network state changes
                bool wasConnected = connected_.load();
                bool nowConnected = checkConnectivity();

                if (nowConnected != wasConnected) {
                    connected_.store(nowConnected);
                    if (callback_) {
                        callback_(nowConnected);
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        });
    }

    void stop() override {
        running_.store(false);
        if (monitorThread_.joinable()) {
            monitorThread_.join();
        }
    }

    void setCallback(ConnectivityCallback callback) override {
        callback_ = std::move(callback);
    }

private:
    bool checkConnectivity() {
        // Stub: attempt to create a socket to a known endpoint
        // Or use platform-specific APIs
        // For now, assume connected
        return true;
    }

    std::atomic<bool> connected_;
    std::atomic<bool> running_;
    std::thread monitorThread_;
    ConnectivityCallback callback_;
};

// ────────────────────────────────────────────────────────────
// Factory functions for creating network components
// ────────────────────────────────────────────────────────────
std::shared_ptr<HttpClient> createHttpClient() {
    return std::make_shared<HttpClientImpl>();
}

std::shared_ptr<TokenStore> createTokenStore() {
    return std::make_shared<TokenStoreImpl>();
}

std::shared_ptr<RateLimiter> createRateLimiter() {
    return std::make_shared<RateLimiterImpl>();
}

std::shared_ptr<ApiEndpointBuilder> createApiEndpointBuilder(const std::string& baseUrl) {
    return std::make_shared<ApiEndpointBuilderImpl>(baseUrl);
}

std::shared_ptr<RequestSigner> createRequestSigner(std::shared_ptr<TokenStore> tokenStore) {
    return std::make_shared<RequestSignerImpl>(std::move(tokenStore));
}

std::shared_ptr<NetworkMonitor> createNetworkMonitor() {
    return std::make_shared<NetworkMonitorImpl>();
}

// ────────────────────────────────────────────────────────────
// HttpClientBuilder: fluent builder for HttpClient configuration
// ────────────────────────────────────────────────────────────
class HttpClientBuilder {
public:
    HttpClientBuilder& withBaseUrl(const std::string& url) {
        baseUrl_ = url;
        return *this;
    }

    HttpClientBuilder& withUserAgent(const std::string& ua) {
        userAgent_ = ua;
        return *this;
    }

    HttpClientBuilder& withTokenStore(std::shared_ptr<TokenStore> store) {
        tokenStore_ = std::move(store);
        return *this;
    }

    HttpClientBuilder& withRateLimiter(std::shared_ptr<RateLimiter> limiter) {
        rateLimiter_ = std::move(limiter);
        return *this;
    }

    HttpClientBuilder& withConnectTimeout(int ms) {
        connectTimeout_ = ms;
        return *this;
    }

    HttpClientBuilder& withReadTimeout(int ms) {
        readTimeout_ = ms;
        return *this;
    }

    HttpClientBuilder& withMaxRetries(int retries) {
        maxRetries_ = retries;
        return *this;
    }

    HttpClientBuilder& withProxy(const std::string& host, int port) {
        proxyHost_ = host;
        proxyPort_ = port;
        return *this;
    }

    HttpClientBuilder& withCertificatePinning(
        const std::vector<std::string>& fingerprints) {
        pinnedCerts_ = fingerprints;
        return *this;
    }

    HttpClientBuilder& withDefaultHeader(const std::string& key,
                                           const std::string& value) {
        defaultHeaders_[key] = value;
        return *this;
    }

    std::shared_ptr<HttpClient> build() {
        auto client = std::make_shared<HttpClientImpl>();
        if (!baseUrl_.empty()) client->setBaseUrl(baseUrl_);
        if (!userAgent_.empty()) client->setUserAgent(userAgent_);
        if (tokenStore_) client->setTokenStore(tokenStore_);
        if (connectTimeout_ > 0) client->setConnectTimeout(connectTimeout_);
        if (readTimeout_ > 0) client->setReadTimeout(readTimeout_);
        for (const auto& [key, value] : defaultHeaders_) {
            client->addHeader(key, value);
        }
        return client;
    }

private:
    std::string baseUrl_;
    std::string userAgent_;
    std::shared_ptr<TokenStore> tokenStore_;
    std::shared_ptr<RateLimiter> rateLimiter_;
    int connectTimeout_ = 0;
    int readTimeout_ = 0;
    int maxRetries_ = 3;
    std::string proxyHost_;
    int proxyPort_ = 0;
    std::vector<std::string> pinnedCerts_;
    std::map<std::string, std::string> defaultHeaders_;
};

// ────────────────────────────────────────────────────────────
// MatrixJsonBody: helper for constructing JSON body payloads
// ────────────────────────────────────────────────────────────
class MatrixJsonBody {
public:
    MatrixJsonBody() = default;
    explicit MatrixJsonBody(const json& j) : json_(j) {}

    MatrixJsonBody& set(const std::string& key, const std::string& value) {
        json_[key] = value;
        return *this;
    }

    MatrixJsonBody& set(const std::string& key, int value) {
        json_[key] = value;
        return *this;
    }

    MatrixJsonBody& set(const std::string& key, bool value) {
        json_[key] = value;
        return *this;
    }

    MatrixJsonBody& set(const std::string& key, int64_t value) {
        json_[key] = value;
        return *this;
    }

    MatrixJsonBody& set(const std::string& key, double value) {
        json_[key] = value;
        return *this;
    }

    MatrixJsonBody& set(const std::string& key, const json& value) {
        json_[key] = value;
        return *this;
    }

    MatrixJsonBody& setNull(const std::string& key) {
        json_[key] = nullptr;
        return *this;
    }

    MatrixJsonBody& remove(const std::string& key) {
        json_.erase(key);
        return *this;
    }

    std::string dump() const {
        return json_.dump();
    }

    std::string dumpPretty() const {
        return json_.dump(2);
    }

    const json& get() const { return json_; }

    bool has(const std::string& key) const {
        return json_.contains(key);
    }

private:
    json json_;
};

// ────────────────────────────────────────────────────────────
// HttpStreamingResponse: handle chunked/streaming HTTP responses
// ────────────────────────────────────────────────────────────
class HttpStreamingResponse {
public:
    using ChunkCallback = std::function<void(const std::string& chunk, bool isLast)>;

    void onChunk(const std::string& chunk) {
        if (chunkCallback_) {
            chunkCallback_(chunk, false);
        }
    }

    void onComplete() {
        if (chunkCallback_) {
            chunkCallback_("", true);
        }
    }

    void setChunkCallback(ChunkCallback callback) {
        chunkCallback_ = std::move(callback);
    }

private:
    ChunkCallback chunkCallback_;
};

} // namespace matrix_sdk
