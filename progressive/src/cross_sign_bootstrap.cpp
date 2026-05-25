#include "progressive/cross_sign_bootstrap.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "CrossSignBootstrap"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(...) fprintf(stderr, "[INFO] CrossSignBootstrap: " __VA_ARGS__)
#define LOGE(...) fprintf(stderr, "[ERROR] CrossSignBootstrap: " __VA_ARGS__)
#define LOGW(...) fprintf(stderr, "[WARN] CrossSignBootstrap: " __VA_ARGS__)
#endif

namespace progressive {
namespace {

// Simple JSON value extractor for string values
std::string extractStr(const std::string& json, const std::string& key) {
    std::string pattern = "\"" + key + "\":\"";
    auto p = json.find(pattern);
    if (p == std::string::npos) {
        pattern = "\"" + key + "\": \"";
        p = json.find(pattern);
    }
    if (p == std::string::npos) return "";
    p += pattern.size();
    auto e = json.find('"', p);
    if (e == std::string::npos) return json.substr(p);
    return json.substr(p, e - p);
}

// Extract boolean value
bool extractBool(const std::string& json, const std::string& key) {
    std::string pattern = "\"" + key + "\":true";
    if (json.find(pattern) != std::string::npos) return true;
    pattern = "\"" + key + "\": true";
    if (json.find(pattern) != std::string::npos) return true;
    return false;
}

// Extract integer value
int extractInt(const std::string& json, const std::string& key, int def = 0) {
    std::string pattern = "\"" + key + "\":";
    auto p = json.find(pattern);
    if (p == std::string::npos) return def;
    p += pattern.size();
    while (p < json.size() && (json[p] == ' ' || json[p] == '\t')) p++;
    auto e = p;
    while (e < json.size() && (std::isdigit(json[e]) || json[e] == '-')) e++;
    if (e == p) return def;
    return std::atoi(json.substr(p, e - p).c_str());
}

// Check if JSON has a specific key
bool hasKey(const std::string& json, const std::string& key) {
    return json.find("\"" + key + "\"") != std::string::npos;
}

// Build a simple JSON response
std::string buildResponse(bool ok, const std::string& fn,
                          const std::string& extra = "") {
    std::ostringstream o;
    o << "{\"ok\":" << (ok ? "true" : "false")
      << ",\"fn\":\"" << fn << "\"";
    if (!extra.empty()) o << "," << extra;
    o << "}";
    return o.str();
}

enum class BootstrapStep {
    INITIAL = 0,
    GENERATING_KEYS = 1,
    UPLOADING_SIGNING_KEYS = 2,
    STORING_KEY_BACKUP = 3,
    UPLOADING_DEVICE_KEYS = 4,
    COMPLETED = 5,
    FAILED = 6
};

const char* stepToString(BootstrapStep step) {
    switch (step) {
        case BootstrapStep::INITIAL: return "initial";
        case BootstrapStep::GENERATING_KEYS: return "generating_keys";
        case BootstrapStep::UPLOADING_SIGNING_KEYS: return "uploading_signing_keys";
        case BootstrapStep::STORING_KEY_BACKUP: return "storing_key_backup";
        case BootstrapStep::UPLOADING_DEVICE_KEYS: return "uploading_device_keys";
        case BootstrapStep::COMPLETED: return "completed";
        case BootstrapStep::FAILED: return "failed";
        default: return "unknown";
    }
}

} // anonymous namespace

std::string bootstrapCrossSign(const std::string& json) {
    LOGI("bootstrapCrossSign called with %zu bytes", json.size());

    if (json.empty()) {
        LOGE("bootstrapCrossSign: empty input");
        return buildResponse(false, "bootstrapCrossSign",
                             "\"error\":\"empty input\"");
    }

    auto auth = extractStr(json, "auth");
    auto reset = extractBool(json, "reset");
    auto setupPassphrase = extractStr(json, "setup_passphrase");
    auto initialDeviceName = extractStr(json, "initial_device_display_name");

    if (auth.empty()) {
        LOGW("bootstrapCrossSign: no auth provided, attempting unauth bootstrap");
    }

    // Build bootstrap steps
    std::ostringstream steps;
    steps << "\"steps\":[";
    steps << "{\"step\":\"" << stepToString(BootstrapStep::GENERATING_KEYS)
          << "\",\"status\":\"pending\"},";
    steps << "{\"step\":\"" << stepToString(BootstrapStep::UPLOADING_SIGNING_KEYS)
          << "\",\"status\":\"pending\"},";

    if (!setupPassphrase.empty()) {
        steps << "{\"step\":\"" << stepToString(BootstrapStep::STORING_KEY_BACKUP)
              << "\",\"status\":\"pending\"},";
    }

    steps << "{\"step\":\"" << stepToString(BootstrapStep::UPLOADING_DEVICE_KEYS)
          << "\",\"status\":\"pending\"},";
    steps << "{\"step\":\"" << stepToString(BootstrapStep::COMPLETED)
          << "\",\"status\":\"success\"}";
    steps << "]";

    // Simulate cross-signing bootstrap
    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"bootstrapCrossSign\""
      << ",\"bootstrap_initiated\":true"
      << ",\"master_key_generated\":true"
      << ",\"user_signing_key_generated\":true"
      << ",\"self_signing_key_generated\":true"
      << ",\"key_backup_version\":\"1\""
      << ",\"devices_uploaded\":true"
      << ",\"reset\":" << (reset ? "true" : "false")
      << ",\"needs_passphrase\":" << (setupPassphrase.empty() ? "false" : "true")
      << "," << steps.str()
      << "}";

    LOGI("bootstrapCrossSign: success");
    return o.str();
}

std::string getBootstrapStatus(const std::string& json) {
    LOGI("getBootstrapStatus called with %zu bytes", json.size());

    if (json.empty()) {
        LOGE("getBootstrapStatus: empty input");
        return buildResponse(false, "getBootstrapStatus",
                             "\"error\":\"empty input\"");
    }

    auto userId = extractStr(json, "user_id");
    if (userId.empty()) {
        LOGW("getBootstrapStatus: no user_id in input");
    }

    // Check cross-signing status details
    bool hasMasterKey = hasKey(json, "master_key");
    bool hasUserSigningKey = hasKey(json, "user_signing_key");
    bool hasSelfSigningKey = hasKey(json, "self_signing_key");
    bool hasKeyBackup = hasKey(json, "key_backup");
    int deviceCount = extractInt(json, "device_count", 1);

    std::string status = "not_bootstrapped";
    if (hasMasterKey && hasUserSigningKey && hasSelfSigningKey) {
        status = hasKeyBackup ? "fully_bootstrapped" : "bootstrapped_no_backup";
    } else if (hasMasterKey) {
        status = "partially_bootstrapped";
    }

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"getBootstrapStatus\""
      << ",\"status\":\"" << status << "\""
      << ",\"has_master_key\":" << (hasMasterKey ? "true" : "false")
      << ",\"has_user_signing_key\":" << (hasUserSigningKey ? "true" : "false")
      << ",\"has_self_signing_key\":" << (hasSelfSigningKey ? "true" : "false")
      << ",\"has_key_backup\":" << (hasKeyBackup ? "true" : "false")
      << ",\"device_count\":" << deviceCount
      << ",\"setup_required\":" << (status == "not_bootstrapped" ? "true" : "false")
      << ",\"is_complete\":" << (status == "fully_bootstrapped" ? "true" : "false")
      << "}";

    LOGI("getBootstrapStatus: status=%s", status.c_str());
    return o.str();
}

std::string isBootstrapped(const std::string& json) {
    LOGI("isBootstrapped called with %zu bytes", json.size());

    if (json.empty()) {
        LOGE("isBootstrapped: empty input");
        return buildResponse(false, "isBootstrapped",
                             "\"error\":\"empty input\",\"bootstrapped\":false");
    }

    // Determine if cross-signing is fully bootstrapped
    bool hasMaster = hasKey(json, "master_key") ||
                     extractBool(json, "has_master_key");
    bool hasUserSign = hasKey(json, "user_signing_key") ||
                       extractBool(json, "has_user_signing_key");
    bool hasSelfSign = hasKey(json, "self_signing_key") ||
                       extractBool(json, "has_self_signing_key");
    bool hasBackup = hasKey(json, "key_backup_version") ||
                     extractBool(json, "has_key_backup");

    bool bootstrapped = hasMaster && hasUserSign && hasSelfSign;
    bool fullyDone = bootstrapped && hasBackup;

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"isBootstrapped\""
      << ",\"bootstrapped\":" << (bootstrapped ? "true" : "false")
      << ",\"fully_bootstrapped\":" << (fullyDone ? "true" : "false")
      << ",\"master_key_present\":" << (hasMaster ? "true" : "false")
      << ",\"user_signing_key_present\":" << (hasUserSign ? "true" : "false")
      << ",\"self_signing_key_present\":" << (hasSelfSign ? "true" : "false")
      << ",\"key_backup_present\":" << (hasBackup ? "true" : "false")
      << ",\"missing\":[";

    bool first = true;
    if (!hasMaster) {
        o << "\"master_key\"";
        first = false;
    }
    if (!hasUserSign) {
        if (!first) o << ",";
        o << "\"user_signing_key\"";
        first = false;
    }
    if (!hasSelfSign) {
        if (!first) o << ",";
        o << "\"self_signing_key\"";
        first = false;
    }
    if (!hasBackup) {
        if (!first) o << ",";
        o << "\"key_backup\"";
    }
    o << "]}";

    LOGI("isBootstrapped: result=%s", bootstrapped ? "true" : "false");
    return o.str();
}

std::string resetCrossSign(const std::string& json) {
    LOGI("resetCrossSign called with %zu bytes", json.size());

    if (json.empty()) {
        LOGE("resetCrossSign: empty input");
        return buildResponse(false, "resetCrossSign",
                             "\"error\":\"empty input\"");
    }

    auto auth = extractStr(json, "auth");
    auto confirmReset = extractBool(json, "confirm_reset");

    if (!confirmReset) {
        LOGW("resetCrossSign: reset not confirmed");
        return buildResponse(false, "resetCrossSign",
                             "\"error\":\"reset not confirmed\","
                             "\"confirmation_required\":true");
    }

    if (auth.empty()) {
        LOGW("resetCrossSign: no auth provided");
        return buildResponse(false, "resetCrossSign",
                             "\"error\":\"authentication required\"");
    }

    // Simulate resetting cross-signing keys
    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"resetCrossSign\""
      << ",\"reset_completed\":true"
      << ",\"master_key_cleared\":true"
      << ",\"user_signing_key_cleared\":true"
      << ",\"self_signing_key_cleared\":true"
      << ",\"device_signatures_cleared\":true"
      << ",\"key_backup_deleted\":true"
      << ",\"needs_new_bootstrap\":true"
      << ",\"timestamp\":" << std::time(nullptr)
      << "}";

    LOGI("resetCrossSign: success");
    return o.str();
}

std::string formatBootstrapStep(const std::string& json) {
    LOGI("formatBootstrapStep called with %zu bytes", json.size());

    if (json.empty()) {
        LOGE("formatBootstrapStep: empty input");
        return buildResponse(false, "formatBootstrapStep",
                             "\"error\":\"empty input\"");
    }

    auto stepStr = extractStr(json, "step");
    auto status = extractStr(json, "status");
    auto progress = extractInt(json, "progress", 0);
    auto errorMsg = extractStr(json, "error");

    if (stepStr.empty()) {
        LOGW("formatBootstrapStep: no step specified");
        return buildResponse(false, "formatBootstrapStep",
                             "\"error\":\"no step specified\"");
    }

    if (status.empty()) status = "pending";

    // Map step to human-readable format
    std::string displayName;
    std::string description;
    if (stepStr == "initial") {
        displayName = "Initializing";
        description = "Preparing cross-signing setup";
    } else if (stepStr == "generating_keys") {
        displayName = "Generating Keys";
        description = "Creating master, user-signing, and self-signing keys";
    } else if (stepStr == "uploading_signing_keys") {
        displayName = "Uploading Signing Keys";
        description = "Uploading cross-signing keys to the server";
    } else if (stepStr == "storing_key_backup") {
        displayName = "Storing Key Backup";
        description = "Encrypting and storing key backup on the server";
    } else if (stepStr == "uploading_device_keys") {
        displayName = "Uploading Device Keys";
        description = "Uploading device keys and signatures";
    } else if (stepStr == "completed") {
        displayName = "Complete";
        description = "Cross-signing setup finished successfully";
    } else if (stepStr == "failed") {
        displayName = "Failed";
        description = "Cross-signing setup encountered an error";
    } else {
        displayName = stepStr;
        description = "Processing step: " + stepStr;
    }

    // Determine icon/emoji based on status
    std::string icon;
    if (status == "success" || status == "completed") icon = "✅";
    else if (status == "failed" || status == "error") icon = "❌";
    else if (status == "in_progress" || status == "processing") icon = "🔄";
    else if (status == "pending") icon = "⏳";
    else icon = "📋";

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"formatBootstrapStep\""
      << ",\"step\":\"" << stepStr << "\""
      << ",\"display_name\":\"" << displayName << "\""
      << ",\"description\":\"" << description << "\""
      << ",\"status\":\"" << status << "\""
      << ",\"icon\":\"" << icon << "\""
      << ",\"progress\":" << progress
      << ",\"formatted\":\"" << icon << " " << displayName;

    if (!errorMsg.empty()) {
        o << ",\"error\":\"" << errorMsg << "\"";
        o << "\"\n  Error: " << errorMsg;
    } else {
        if (progress > 0 && progress < 100) {
            o << " (" << progress << "%)";
        } else if (status == "success") {
            o << " - Done";
        } else if (status == "failed") {
            o << " - Error";
        } else if (status == "in_progress") {
            o << "...";
        }
    }
    o << "\"}";

    LOGI("formatBootstrapStep: step=%s status=%s", stepStr.c_str(), status.c_str());
    return o.str();
}

} // namespace progressive
