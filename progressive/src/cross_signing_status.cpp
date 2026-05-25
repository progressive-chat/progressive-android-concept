#include "progressive/cross_signing_status.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "CrossSignStatus"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(...) fprintf(stderr, "[INFO] CrossSignStatus: " __VA_ARGS__)
#define LOGE(...) fprintf(stderr, "[ERROR] CrossSignStatus: " __VA_ARGS__)
#define LOGW(...) fprintf(stderr, "[WARN] CrossSignStatus: " __VA_ARGS__)
#endif

namespace progressive {
namespace {

std::string extractStr(const std::string& json, const std::string& key) {
    std::string p = "\"" + key + "\":\"";
    auto pos = json.find(p);
    if (pos == std::string::npos) { p = "\"" + key + "\": \""; pos = json.find(p); }
    if (pos == std::string::npos) return "";
    pos += p.size();
    auto e = json.find('"', pos);
    return e == std::string::npos ? json.substr(pos) : json.substr(pos, e - pos);
}

bool hasKey(const std::string& json, const std::string& key) {
    return json.find("\"" + key + "\"") != std::string::npos;
}

bool extractBool(const std::string& json, const std::string& key) {
    return json.find("\"" + key + "\":true") != std::string::npos ||
           json.find("\"" + key + "\": true") != std::string::npos;
}

int extractInt(const std::string& json, const std::string& key, int def = 0) {
    auto p = json.find("\"" + key + "\":");
    if (p == std::string::npos) return def;
    p += key.size() + 3;
    while (p < json.size() && std::isspace(json[p])) p++;
    auto e = p;
    while (e < json.size() && (std::isdigit(json[e]) || json[e] == '-')) e++;
    if (e == p) return def;
    return std::atoi(json.substr(p, e - p).c_str());
}

std::string buildOk(const std::string& fn, const std::string& extra = "") {
    std::ostringstream o;
    o << "{\"ok\":true,\"fn\":\"" << fn << "\"";
    if (!extra.empty()) o << "," << extra;
    o << "}";
    return o.str();
}

} // namespace

std::string getMasterKeyStatus(const std::string& json) {
    LOGI("getMasterKeyStatus: %zu bytes", json.size());
    if (json.empty()) {
        return R"({"ok":false,"fn":"getMasterKeyStatus","error":"empty input"})";
    }

    auto userId = extractStr(json, "user_id");
    bool hasMasterKey = hasKey(json, "master_key") || extractBool(json, "has_master_key");
    auto masterKey = extractStr(json, "master_key");
    if (masterKey.empty()) masterKey = extractStr(json, "key");
    auto algorithm = extractStr(json, "algorithm");
    if (algorithm.empty()) algorithm = "ed25519";

    std::string status;
    if (hasMasterKey && !masterKey.empty()) {
        status = masterKey.size() >= 32 ? "valid" : "needs_refresh";
    } else if (hasMasterKey) {
        status = "present_but_empty";
    } else {
        status = "missing";
    }

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"getMasterKeyStatus\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"status\":\"" << status << "\""
      << ",\"master_key_present\":" << (hasMasterKey ? "true" : "false")
      << ",\"algorithm\":\"" << algorithm << "\""
      << ",\"key_length\":" << masterKey.size()
      << ",\"needs_setup\":" << (!hasMasterKey ? "true" : "false")
      << ",\"is_valid\":" << (status == "valid" ? "true" : "false")
      << ",\"can_sign\":" << (status == "valid" ? "true" : "false")
      << "}";
    return o.str();
}

std::string isCrossSigningReady(const std::string& json) {
    LOGI("isCrossSigningReady: %zu bytes", json.size());
    if (json.empty()) {
        return R"({"ok":false,"fn":"isCrossSigningReady","error":"empty input","ready":false})";
    }

    auto userId = extractStr(json, "user_id");
    bool hasMaster = hasKey(json, "master_key") || extractBool(json, "has_master_key");
    bool hasUserSign = hasKey(json, "user_signing_key") ||
                       extractBool(json, "has_user_signing_key");
    bool hasSelfSign = hasKey(json, "self_signing_key") ||
                       extractBool(json, "has_self_signing_key");
    bool hasBackup = hasKey(json, "key_backup") || extractBool(json, "has_key_backup");
    int deviceCount = extractInt(json, "device_count", 1);

    bool ready = hasMaster && hasUserSign && hasSelfSign && deviceCount > 0;

    std::string readiness;
    if (ready && hasBackup) readiness = "fully_ready";
    else if (ready) readiness = "ready_no_backup";
    else if (hasMaster && (hasUserSign || hasSelfSign)) readiness = "partially_ready";
    else if (hasMaster) readiness = "needs_signing_keys";
    else readiness = "not_ready";

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"isCrossSigningReady\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"ready\":" << (ready ? "true" : "false")
      << ",\"readiness\":\"" << readiness << "\""
      << ",\"has_master_key\":" << (hasMaster ? "true" : "false")
      << ",\"has_user_signing_key\":" << (hasUserSign ? "true" : "false")
      << ",\"has_self_signing_key\":" << (hasSelfSign ? "true" : "false")
      << ",\"has_key_backup\":" << (hasBackup ? "true" : "false")
      << ",\"device_count\":" << deviceCount
      << ",\"missing_count\":" << (!hasMaster + !hasUserSign + !hasSelfSign)
      << ",\"can_verify_others\":" << (ready ? "true" : "false")
      << ",\"setup_action\":" << (ready ? "\"none\"" : "\"bootstrap_required\"")
      << "}";
    return o.str();
}

std::string getSSSSStatus(const std::string& json) {
    LOGI("getSSSSStatus: %zu bytes", json.size());
    if (json.empty()) {
        return R"({"ok":false,"fn":"getSSSSStatus","error":"empty input"})";
    }

    auto userId = extractStr(json, "user_id");
    bool hasSSSS = hasKey(json, "ssss") || extractBool(json, "has_ssss");
    bool hasKeyBackup = hasKey(json, "key_backup") || extractBool(json, "has_key_backup");
    auto ssssKey = extractStr(json, "ssss_key");
    if (ssssKey.empty()) ssssKey = extractStr(json, "secret_storage_key");
    auto passphraseSet = extractStr(json, "passphrase");
    bool hasPassphrase = !passphraseSet.empty() || extractBool(json, "has_passphrase");
    int secretsCount = extractInt(json, "secrets_count", 0);

    // SSSS (Secure Secret Storage and Sharing) status
    std::string ssssStatus;
    if (hasSSSS && hasKeyBackup) {
        if (hasPassphrase) ssssStatus = "full";
        else ssssStatus = "key_backup_only";
    } else if (hasSSSS) {
        ssssStatus = "ssss_no_backup";
    } else if (hasKeyBackup) {
        ssssStatus = "backup_no_ssss";
    } else {
        ssssStatus = "not_set_up";
    }

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"getSSSSStatus\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"ssss_status\":\"" << ssssStatus << "\""
      << ",\"has_ssss\":" << (hasSSSS ? "true" : "false")
      << ",\"has_key_backup\":" << (hasKeyBackup ? "true" : "false")
      << ",\"has_passphrase\":" << (hasPassphrase ? "true" : "false")
      << ",\"secrets_count\":" << secretsCount
      << ",\"is_set_up\":" << (ssssStatus != "not_set_up" ? "true" : "false")
      << ",\"needs_setup\":" << (ssssStatus == "not_set_up" ? "true" : "false")
      << ",\"supports_recovery\":" << (hasPassphrase ? "true" : "false")
      << ",\"default_key_id\":\"" << (ssssKey.empty() ? "unknown" : ssssKey) << "\""
      << "}";
    return o.str();
}

std::string formatCrossSigningBanner(const std::string& json) {
    LOGI("formatCrossSigningBanner: %zu bytes", json.size());
    if (json.empty()) {
        return R"({"ok":false,"fn":"formatCrossSigningBanner","error":"empty input"})";
    }

    auto status = extractStr(json, "status");
    if (status.empty()) status = extractStr(json, "bootstrap_status");
    if (status.empty()) status = "not_bootstrapped";

    auto userId = extractStr(json, "user_id");
    bool isDismissed = extractBool(json, "dismissed");
    int remainingDays = extractInt(json, "remaining_days", -1);

    // Format the banner text and determine style
    std::string title, subtitle, action, icon, color, priority;

    if (status == "fully_bootstrapped" || status == "complete") {
        if (isDismissed) {
            title = "Encryption Ready";
            subtitle = "Cross-signing is set up and active";
            action = "manage";
            icon = "🔒";
            color = "green";
            priority = "low";
        } else {
            title = "Encryption Setup Complete";
            subtitle = "Your account is now secure with cross-signing";
            action = "dismiss";
            icon = "✅";
            color = "green";
            priority = "low";
        }
    } else if (status == "bootstrapped_no_backup") {
        title = "Key Backup Needed";
        subtitle = "Set up key backup to avoid losing encrypted messages";
        action = "setup_backup";
        icon = "⚠️";
        color = "orange";
        priority = "medium";
    } else if (status == "partially_bootstrapped") {
        title = "Setup Incomplete";
        subtitle = "Finish setting up encryption to secure your messages";
        action = "continue_setup";
        icon = "🔧";
        color = "yellow";
        priority = "high";
    } else if (status == "in_progress") {
        title = "Setting Up Encryption";
        subtitle = "Cross-signing initialization in progress...";
        action = "wait";
        icon = "🔄";
        color = "blue";
        priority = "medium";
    } else {
        title = "Set Up Encryption";
        if (remainingDays > 0) {
            subtitle = "You have " + std::to_string(remainingDays) +
                       " days to set up encryption for new devices";
        } else {
            subtitle = "Verify your devices for secure messaging";
        }
        action = "setup";
        icon = "🔐";
        color = "red";
        priority = "high";
    }

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"formatCrossSigningBanner\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"status\":\"" << status << "\""
      << ",\"title\":\"" << title << "\""
      << ",\"subtitle\":\"" << subtitle << "\""
      << ",\"action\":\"" << action << "\""
      << ",\"icon\":\"" << icon << "\""
      << ",\"color\":\"" << color << "\""
      << ",\"priority\":\"" << priority << "\""
      << ",\"dismissed\":" << (isDismissed ? "true" : "false")
      << ",\"remaining_days\":" << remainingDays
      << ",\"formatted\":\"" << icon << " " << title << "\\n" << subtitle << "\""
      << ",\"banner_type\":\"encryption_setup\""
      << ",\"can_dismiss\":" << (priority != "high" ? "true" : "false")
      << "}";
    return o.str();
}

std::string needsSetup(const std::string& json) {
    LOGI("needsSetup: %zu bytes", json.size());
    if (json.empty()) {
        return R"({"ok":false,"fn":"needsSetup","error":"empty input","needs_setup":true})";
    }

    auto userId = extractStr(json, "user_id");
    bool hasMaster = hasKey(json, "master_key") || extractBool(json, "has_master_key");
    bool hasUserSign = hasKey(json, "user_signing_key") ||
                       extractBool(json, "has_user_signing_key");
    bool hasSelfSign = hasKey(json, "self_signing_key") ||
                       extractBool(json, "has_self_signing_key");
    bool hasBackup = hasKey(json, "key_backup") || extractBool(json, "has_key_backup");
    bool hasSSSS = hasKey(json, "ssss") || extractBool(json, "has_ssss");
    auto setupPhase = extractStr(json, "setup_phase");

    // Determine if setup is needed and what specifically
    bool needsAny = !hasMaster || !hasUserSign || !hasSelfSign;
    bool needsBackup = !hasBackup && !hasSSSS;
    bool needsSigningKeys = hasMaster && (!hasUserSign || !hasSelfSign);

    std::ostringstream requirements;
    requirements << "[";
    bool first = true;
    if (!hasMaster) { requirements << "\"master_key\""; first = false; }
    if (!hasUserSign) {
        if (!first) requirements << ","; requirements << "\"user_signing_key\""; first = false;
    }
    if (!hasSelfSign) {
        if (!first) requirements << ","; requirements << "\"self_signing_key\""; first = false;
    }
    if (!hasBackup && !hasSSSS) {
        if (!first) requirements << ","; requirements << "\"key_backup\"";
    }
    requirements << "]";

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"needsSetup\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"needs_setup\":" << (needsAny ? "true" : "false")
      << ",\"needs_keys\":" << (needsAny ? "true" : "false")
      << ",\"needs_backup\":" << (needsBackup ? "true" : "false")
      << ",\"needs_signing_keys\":" << (needsSigningKeys ? "true" : "false")
      << ",\"has_master_key\":" << (hasMaster ? "true" : "false")
      << ",\"has_user_signing_key\":" << (hasUserSign ? "true" : "false")
      << ",\"has_self_signing_key\":" << (hasSelfSign ? "true" : "false")
      << ",\"has_key_backup\":" << (hasBackup ? "true" : "false")
      << ",\"has_ssss\":" << (hasSSSS ? "true" : "false")
      << ",\"setup_phase\":\"" << (setupPhase.empty() ? "none" : setupPhase) << "\""
      << ",\"requirements\":" << requirements.str()
      << ",\"is_fully_setup\":" << (!needsAny && !needsBackup ? "true" : "false")
      << "}";
    return o.str();
}

} // namespace progressive
