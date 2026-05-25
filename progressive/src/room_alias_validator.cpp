#include "progressive/room_alias_validator.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <mutex>

namespace progressive {
using json = nlohmann::json;

namespace {
    RoomAliasValidatorConfig g_config;
    std::mutex g_mutex;
    RoomAliasValidatorStatus g_status;
    RoomAliasValidatorStats g_stats;
    bool g_initialized = false;
} // anonymous namespace

const char* room_alias_validator_stateToString(AliasValidatorAction s) {
    switch (s) {
        case AliasValidatorAction::UNKNOWN: return "UNKNOWN";
        case AliasValidatorAction::INIT: return "INIT";
        case AliasValidatorAction::PROCESSING: return "PROCESSING";
        case AliasValidatorAction::DONE: return "DONE";
        case AliasValidatorAction::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

AliasValidatorAction room_alias_validator_stateFromString(const std::string& s) {
    if (s == "UNKNOWN") return AliasValidatorAction::UNKNOWN;
    if (s == "INIT") return AliasValidatorAction::INIT;
    if (s == "PROCESSING") return AliasValidatorAction::PROCESSING;
    if (s == "DONE") return AliasValidatorAction::DONE;
    if (s == "ERROR") return AliasValidatorAction::ERROR;
    return AliasValidatorAction::UNKNOWN;
}

bool room_alias_validator_init(const RoomAliasValidatorConfig& config) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // Validate configuration
    if (config.serverUrl.empty()) return false;
    if (config.userId.empty()) return false;

    // Store configuration
    g_config = config;
    g_initialized = true;
    g_status.state = AliasValidatorAction::INIT;
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

RoomAliasValidatorResponse room_alias_validator_handle(const RoomAliasValidatorRequest& req) {
    RoomAliasValidatorResponse resp;
    auto start = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_initialized) {
        resp.success = false;
        resp.error = "NOT_INITIALIZED";
        resp.errorMessage = "Module not initialized";
        return resp;
    }

    g_status.state = AliasValidatorAction::PROCESSING;
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

RoomAliasValidatorStatus room_alias_validator_getStatus() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_status;
}

void room_alias_validator_reset() {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_initialized = false;
    g_config = RoomAliasValidatorConfig{};
    g_status = RoomAliasValidatorStatus{};
    g_status.state = AliasValidatorAction::UNKNOWN;
}

bool room_alias_validator_validate(const std::string& input) {
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

std::string room_alias_validator_process(const std::string& input) {
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
        result["module"] = "room_alias_validator";
        return result.dump();
    } catch (const std::exception& e) {
        result["error"] = e.what();
        result["processed"] = false;

        return result.dump();
    }
}

json room_alias_validator_toJson(const std::string& input) {
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

RoomAliasValidatorStats room_alias_validator_getStats() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_stats;
}

RoomAliasValidatorConfig room_alias_validator_configFromJson(const json& j) {
    RoomAliasValidatorConfig config;
    config.serverUrl = j.value("server_url", "");
    config.userId = j.value("user_id", "");
    config.accessToken = j.value("access_token", "");
    config.timeoutMs = j.value("timeout_ms", 30000);
    config.retryCount = j.value("retry_count", 3);
    config.enableLogging = j.value("enable_logging", false);
    return config;
}

json room_alias_validator_configToJson(const RoomAliasValidatorConfig& config) {
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
