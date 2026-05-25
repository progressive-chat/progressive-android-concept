#include "progressive/url_preview_formatter.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <mutex>

namespace progressive {
using json = nlohmann::json;

namespace {
    UrlPreviewFormatterConfig g_config;
    std::mutex g_mutex;
    UrlPreviewFormatterStatus g_status;
    UrlPreviewFormatterStats g_stats;
    bool g_initialized = false;
} // anonymous namespace

const char* url_preview_formatter_stateToString(UrlPreviewFormatterState s) {
    switch (s) {
        case UrlPreviewFormatterState::UNKNOWN: return "UNKNOWN";
        case UrlPreviewFormatterState::INIT: return "INIT";
        case UrlPreviewFormatterState::PROCESSING: return "PROCESSING";
        case UrlPreviewFormatterState::DONE: return "DONE";
        case UrlPreviewFormatterState::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

UrlPreviewFormatterState url_preview_formatter_stateFromString(const std::string& s) {
    if (s == "UNKNOWN") return UrlPreviewFormatterState::UNKNOWN;
    if (s == "INIT") return UrlPreviewFormatterState::INIT;
    if (s == "PROCESSING") return UrlPreviewFormatterState::PROCESSING;
    if (s == "DONE") return UrlPreviewFormatterState::DONE;
    if (s == "ERROR") return UrlPreviewFormatterState::ERROR;
    return UrlPreviewFormatterState::UNKNOWN;
}

bool url_preview_formatter_init(const UrlPreviewFormatterConfig& config) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // Validate configuration
    if (config.serverUrl.empty()) return false;
    if (config.userId.empty()) return false;

    // Store configuration
    g_config = config;
    g_initialized = true;
    g_status.state = UrlPreviewFormatterState::INIT;
    g_status.isInitialized = true;
    g_status.lastActivity = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    g_stats.totalInvocations = 0;
    g_stats.successCount = 0;
    g_stats.failureCount = 0;
    g_stats.totalProcessingMs = 0;
    g_stats.averageMs = 0.0;
    return true;
}

UrlPreviewFormatterResponse url_preview_formatter_handle(const UrlPreviewFormatterRequest& req) {
    UrlPreviewFormatterResponse resp;
    auto start = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_initialized) {
        resp.success = false;
        resp.error = "NOT_INITIALIZED";
        resp.errorMessage = "Module not initialized";
        return resp;
    }

    g_status.state = UrlPreviewFormatterState::PROCESSING;
    // Validate request
    if (req.roomId.empty() && req.eventId.empty()) {
        resp.success = false;
        resp.error = "INVALID_REQUEST";
        resp.errorMessage = "Missing roomId or eventId";
        resp.statusCode = 400;
        return resp;
    }

    // Process the payload
    try {
        if (!req.payload.is_null() && !req.payload.empty()) {
            // Parse and validate the payload
            std::string payloadStr = req.payload.dump();
            resp.result["type"] = req.payload.value("type", "unknown");
            resp.result["processed"] = true;
        }

        resp.success = true;
        resp.statusCode = 200;
        resp.result["transaction_id"] = req.transactionId;
        resp.serverTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    } catch (const std::exception& e) {
        resp.success = false;
        resp.error = "PROCESSING_ERROR";
        resp.errorMessage = e.what();
        resp.statusCode = 500;
    }

    // Update statistics
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    g_stats.totalInvocations++;
    if (resp.success) g_stats.successCount++;
    else g_stats.failureCount++;
    g_stats.totalProcessingMs += elapsed;
    g_stats.lastInvocationMs = elapsed;
    g_stats.averageMs = static_cast<double>(g_stats.totalProcessingMs) / g_stats.totalInvocations;
    g_status.lastActivity = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return resp;
}

UrlPreviewFormatterStatus url_preview_formatter_getStatus() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_status;
}

void url_preview_formatter_reset() {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_initialized = false;
    g_config = UrlPreviewFormatterConfig{};
    g_status = UrlPreviewFormatterStatus{};
    g_status.state = UrlPreviewFormatterState::UNKNOWN;
}

bool url_preview_formatter_validate(const std::string& input) {
    if (input.empty()) return false;

    // Check for valid JSON if applicable
    if (input.front() == '{') {
        try {
            json j = json::parse(input);
            if (j.is_null()) return false;
            return j.is_object();
        } catch (...) {
            return false;
        }
    }

    // Non-JSON: validate length
    return input.size() <= 65536;  // 64KB limit
}

std::string url_preview_formatter_process(const std::string& input) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (!g_initialized) return "";

    // Parse the input
    json result;
    try {
        json j = json::parse(input);
        result["original"] = j;
        result["processed"] = true;
        result["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        // Apply module-specific processing
        result["module"] = "url_preview_formatter";
        return result.dump();
    } catch (const std::exception& e) {
        result["error"] = e.what();
        result["processed"] = false;

        return result.dump();
    }
}

json url_preview_formatter_toJson(const std::string& input) {
    json j;
    if (input.empty()) return j;

    try {
        j = json::parse(input);
        if (j.is_object()) j["normalized"] = true;
        return j;
    } catch (...) {
        j["raw"] = input;
        j["valid_json"] = false;
        return j;
    }
}

UrlPreviewFormatterStats url_preview_formatter_getStats() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_stats;
}

UrlPreviewFormatterConfig url_preview_formatter_configFromJson(const json& j) {
    UrlPreviewFormatterConfig config;
    config.serverUrl = j.value("server_url", "");
    config.userId = j.value("user_id", "");
    config.accessToken = j.value("access_token", "");
    config.timeoutMs = j.value("timeout_ms", 30000);
    config.retryCount = j.value("retry_count", 3);
    config.enableLogging = j.value("enable_logging", false);
    return config;
}

json url_preview_formatter_configToJson(const UrlPreviewFormatterConfig& config) {
    json j;
    j["server_url"] = config.serverUrl;
    j["user_id"] = config.userId;
    j["access_token"] = config.accessToken;
    j["timeout_ms"] = config.timeoutMs;
    j["retry_count"] = config.retryCount;
    j["enable_logging"] = config.enableLogging;
    return j;
}

} // namespace progressive
