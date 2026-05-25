#include "progressive/room_invite_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <mutex>

namespace progressive {
using json = nlohmann::json;

namespace {
    RoomInviteUtilsConfig g_config;
    std::mutex g_mutex;
    RoomInviteUtilsStatus g_status;
    RoomInviteUtilsStats g_stats;
    bool g_initialized = false;
} // anonymous namespace

const char* room_invite_utils_stateToString(InviteUtilsAction s) {
    switch (s) {
        case InviteUtilsAction::UNKNOWN: return "UNKNOWN";
        case InviteUtilsAction::INIT: return "INIT";
        case InviteUtilsAction::PROCESSING: return "PROCESSING";
        case InviteUtilsAction::DONE: return "DONE";
        case InviteUtilsAction::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

InviteUtilsAction room_invite_utils_stateFromString(const std::string& s) {
    if (s == "UNKNOWN") return InviteUtilsAction::UNKNOWN;
    if (s == "INIT") return InviteUtilsAction::INIT;
    if (s == "PROCESSING") return InviteUtilsAction::PROCESSING;
    if (s == "DONE") return InviteUtilsAction::DONE;
    if (s == "ERROR") return InviteUtilsAction::ERROR;
    return InviteUtilsAction::UNKNOWN;
}

bool room_invite_utils_init(const RoomInviteUtilsConfig& config) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // Validate configuration
    if (config.serverUrl.empty()) return false;
    if (config.userId.empty()) return false;

    // Store configuration
    g_config = config;
    g_initialized = true;
    g_status.state = InviteUtilsAction::INIT;
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

RoomInviteUtilsResponse room_invite_utils_handle(const RoomInviteUtilsRequest& req) {
    RoomInviteUtilsResponse resp;
    auto start = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_initialized) {
        resp.success = false;
        resp.error = "NOT_INITIALIZED";
        resp.errorMessage = "Module not initialized";
        return resp;
    }

    g_status.state = InviteUtilsAction::PROCESSING;
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

RoomInviteUtilsStatus room_invite_utils_getStatus() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_status;
}

void room_invite_utils_reset() {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_initialized = false;
    g_config = RoomInviteUtilsConfig{};
    g_status = RoomInviteUtilsStatus{};
    g_status.state = InviteUtilsAction::UNKNOWN;
}

bool room_invite_utils_validate(const std::string& input) {
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

std::string room_invite_utils_process(const std::string& input) {
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
        result["module"] = "room_invite_utils";
        return result.dump();
    } catch (const std::exception& e) {
        result["error"] = e.what();
        result["processed"] = false;

        return result.dump();
    }
}

json room_invite_utils_toJson(const std::string& input) {
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

RoomInviteUtilsStats room_invite_utils_getStats() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_stats;
}

RoomInviteUtilsConfig room_invite_utils_configFromJson(const json& j) {
    RoomInviteUtilsConfig config;
    config.serverUrl = j.value("server_url", "");
    config.userId = j.value("user_id", "");
    config.accessToken = j.value("access_token", "");
    config.timeoutMs = j.value("timeout_ms", 30000);
    config.retryCount = j.value("retry_count", 3);
    config.enableLogging = j.value("enable_logging", false);
    return config;
}

json room_invite_utils_configToJson(const RoomInviteUtilsConfig& config) {
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
