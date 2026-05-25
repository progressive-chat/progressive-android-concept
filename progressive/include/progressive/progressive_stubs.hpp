#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class ProgressiveStubsState {
    UNKNOWN = 0,
    INIT = 1,
    PROCESSING = 2,
    DONE = 3,
    ERROR = 4
};

const char* progressive_stubs_stateToString(ProgressiveStubsState s);
ProgressiveStubsState progressive_stubs_stateFromString(const std::string& s);

struct ProgressiveStubsConfig {
    std::string serverUrl;           // Matrix homeserver URL
    std::string userId;              // fully qualified MXID
    std::string accessToken;         // Bearer token
    int timeoutMs = 30000;           // request timeout
    int retryCount = 3;              // retry attempts
    bool enableLogging = false;      // debug logging flag
};

struct ProgressiveStubsRequest {
    std::string roomId;              // target room (optional)
    std::string eventId;             // target event (optional)
    json payload;                    // Matrix event payload
    std::string transactionId;       // idempotency key
    int64_t timeoutAt = 0;           // absolute timeout timestamp
};

struct ProgressiveStubsResponse {
    bool success = false;
    std::string error;               // Matrix error code
    std::string errorMessage;        // human-readable error
    json result;                     // result payload
    int statusCode = 0;              // HTTP status code
    int64_t serverTimestamp = 0;     // origin_server_ts
};

struct ProgressiveStubsStatus {
    ProgressiveStubsState state = ProgressiveStubsState::UNKNOWN;
    bool isInitialized = false;
    std::string lastError;
    int64_t lastActivity = 0;        // epoch ms
    int pendingCount = 0;
};

struct ProgressiveStubsStats {
    int totalInvocations = 0;
    int successCount = 0;
    int failureCount = 0;
    int64_t totalProcessingMs = 0;
    int64_t lastInvocationMs = 0;
    double averageMs = 0.0;
    int retryCount = 0;
};

// Initialize the progressive stubs module with configuration
bool progressive_stubs_init(const ProgressiveStubsConfig& config);

// Handle a progressive stubs request, returning a response
ProgressiveStubsResponse progressive_stubs_handle(const ProgressiveStubsRequest& req);

// Get the current operational status
ProgressiveStubsStatus progressive_stubs_getStatus();

// Reset the module to initial state
void progressive_stubs_reset();

// Validate input before processing
bool progressive_stubs_validate(const std::string& input);

// Process input and return result
std::string progressive_stubs_process(const std::string& input);

// Serialize to JSON format
json progressive_stubs_toJson(const std::string& input);

// Get runtime statistics
ProgressiveStubsStats progressive_stubs_getStats();

// Build config from JSON
ProgressiveStubsConfig progressive_stubs_configFromJson(const json& j);

// Serialize config to JSON
json progressive_stubs_configToJson(const ProgressiveStubsConfig& config);
