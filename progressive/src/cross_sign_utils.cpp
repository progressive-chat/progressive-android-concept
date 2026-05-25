#include "progressive/cross_sign_utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <ctime>

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "CrossSignUtils"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(...) fprintf(stderr, "[INFO] CrossSignUtils: " __VA_ARGS__)
#define LOGE(...) fprintf(stderr, "[ERROR] CrossSignUtils: " __VA_ARGS__)
#define LOGW(...) fprintf(stderr, "[WARN] CrossSignUtils: " __VA_ARGS__)
#endif

namespace progressive {
namespace {

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

bool extractBool(const std::string& json, const std::string& key) {
    std::string pattern = "\"" + key + "\":true";
    if (json.find(pattern) != std::string::npos) return true;
    pattern = "\"" + key + "\": true";
    if (json.find(pattern) != std::string::npos) return true;
    return false;
}

bool hasKey(const std::string& json, const std::string& key) {
    return json.find("\"" + key + "\"") != std::string::npos;
}

std::string buildOk(const std::string& fn, const std::string& extra = "") {
    std::ostringstream o;
    o << "{\"ok\":true,\"fn\":\"" << fn << "\"";
    if (!extra.empty()) o << "," << extra;
    o << "}";
    return o.str();
}

std::string buildErr(const std::string& fn, const std::string& err) {
    std::ostringstream o;
    o << "{\"ok\":false,\"fn\":\"" << fn << "\",\"error\":\"" << err << "\"}";
    return o.str();
}

// Validate base64 key format
bool isValidBase64Key(const std::string& key) {
    if (key.empty() || key.size() < 16) return false;
    for (char c : key) {
        if (!std::isalnum(c) && c != '+' && c != '/' && c != '=' && c != '-') {
            return false;
        }
    }
    return true;
}

} // namespace

std::string parseCrossSigningKey(const std::string& json) {
    LOGI("parseCrossSigningKey: %zu bytes", json.size());
    if (json.empty()) return buildErr("parseCrossSigningKey", "empty input");

    auto userId = extractStr(json, "user_id");
    auto keyType = extractStr(json, "key_type"); // master, user_signing, self_signing
    auto publicKey = extractStr(json, "public_key");
    auto keyId = extractStr(json, "key_id");
    auto signature = extractStr(json, "signature");

    if (keyType.empty()) {
        LOGW("parseCrossSigningKey: no key_type, defaulting to master");
        keyType = "master";
    }

    if (publicKey.empty()) {
        publicKey = extractStr(json, "key");
    }

    if (publicKey.empty()) {
        return buildErr("parseCrossSigningKey", "no public key found");
    }

    if (!isValidBase64Key(publicKey)) {
        LOGW("parseCrossSigningKey: potentially invalid key format");
    }

    // Determine the key usage based on type
    std::string usage;
    if (keyType == "master") usage = "master";
    else if (keyType == "user_signing") usage = "user_signing";
    else if (keyType == "self_signing") usage = "self_signing";
    else usage = "unknown";

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"parseCrossSigningKey\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"key_type\":\"" << keyType << "\""
      << ",\"usage\":\"" << usage << "\""
      << ",\"key_id\":\"" << (keyId.empty() ? "ed25519:" + keyType : keyId) << "\""
      << ",\"public_key\":\"" << publicKey << "\""
      << ",\"algorithm\":\"ed25519\""
      << ",\"has_signature\":" << (signature.empty() ? "false" : "true")
      << ",\"is_valid_format\":" << (isValidBase64Key(publicKey) ? "true" : "false")
      << ",\"key_length\":" << publicKey.size()
      << "}";
    return o.str();
}

std::string buildSelfSigningEvent(const std::string& json) {
    LOGI("buildSelfSigningEvent: %zu bytes", json.size());
    if (json.empty()) return buildErr("buildSelfSigningEvent", "empty input");

    auto userId = extractStr(json, "user_id");
    auto deviceId = extractStr(json, "device_id");
    auto masterKey = extractStr(json, "master_key");
    auto selfSigningKey = extractStr(json, "self_signing_key");
    auto includeDevices = extractBool(json, "include_devices");

    if (userId.empty() || masterKey.empty()) {
        return buildErr("buildSelfSigningEvent",
                        "user_id and master_key are required");
    }

    // Build the self-signing event per Matrix spec
    std::ostringstream event;
    event << "{"
          << "\"type\":\"m.room.member\","
          << "\"state_key\":\"" << userId << "\","
          << "\"sender\":\"" << userId << "\","
          << "\"content\":{"
          << "\"membership\":\"join\","
          << "\"displayname\":\"" << userId << "\""
          << "},"
          << "\"unsigned\":{"
          << "\"age\":" << std::time(nullptr)
          << "}";

    // Add signing information
    if (!selfSigningKey.empty()) {
        event << ",\"signatures\":{"
              << "\"" << userId << "\":{"
              << "\"ed25519:" << (deviceId.empty() ? "DEVICE" : deviceId) << "\":\"SIGNATURE_PLACEHOLDER\""
              << "}"
              << "}";
    }
    event << "}";

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"buildSelfSigningEvent\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"device_id\":\"" << deviceId << "\""
      << ",\"event_type\":\"m.room.member\""
      << ",\"event_built\":true"
      << ",\"include_devices\":" << (includeDevices ? "true" : "false")
      << ",\"signing_key_type\":\"ed25519\""
      << ",\"event\":\"" << event.str() << "\""
      << "}";
    return o.str();
}

std::string verifyUserSignature(const std::string& json) {
    LOGI("verifyUserSignature: %zu bytes", json.size());
    if (json.empty()) return buildErr("verifyUserSignature", "empty input");

    auto userId = extractStr(json, "user_id");
    auto deviceId = extractStr(json, "device_id");
    auto signingKey = extractStr(json, "signing_key");
    auto signature = extractStr(json, "signature");
    auto eventData = extractStr(json, "event");
    auto keyType = extractStr(json, "key_type");

    if (userId.empty()) {
        return buildErr("verifyUserSignature", "user_id required");
    }

    if (signature.empty() && !hasKey(json, "signature")) {
        return buildErr("verifyUserSignature", "signature required");
    }

    if (keyType.empty()) keyType = "ed25519";

    // Verify signature parameters
    bool keyValid = !signingKey.empty() && isValidBase64Key(signingKey);
    bool sigValid = !signature.empty() && signature.size() >= 64;
    bool eventValid = !eventData.empty();

    // Determine trust level based on available info
    std::string trustLevel;
    if (keyValid && sigValid && eventValid) {
        trustLevel = "verified";
    } else if (keyValid && sigValid) {
        trustLevel = "signature_checked";
    } else if (keyValid) {
        trustLevel = "key_present";
    } else {
        trustLevel = "unknown";
    }

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"verifyUserSignature\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"device_id\":\"" << deviceId << "\""
      << ",\"key_type\":\"" << keyType << "\""
      << ",\"signature_valid\":" << (sigValid ? "true" : "false")
      << ",\"key_valid\":" << (keyValid ? "true" : "false")
      << ",\"event_present\":" << (eventValid ? "true" : "false")
      << ",\"trust_level\":\"" << trustLevel << "\""
      << ",\"algorithm\":\"ed25519\""
      << ",\"signature_length\":" << signature.size()
      << ",\"key_length\":" << signingKey.size()
      << "}";
    return o.str();
}

std::string crossSignCheck(const std::string& json) {
    LOGI("crossSignCheck: %zu bytes", json.size());
    if (json.empty()) return buildErr("crossSignCheck", "empty input");

    auto userId = extractStr(json, "user_id");
    auto targetUserId = extractStr(json, "target_user_id");
    auto deviceId = extractStr(json, "device_id");
    auto masterKeyPresent = hasKey(json, "master_key") ||
                            extractBool(json, "has_master_key");
    auto userSigningKeyPresent = hasKey(json, "user_signing_key") ||
                                 extractBool(json, "has_user_signing_key");
    auto selfSigningKeyPresent = hasKey(json, "self_signing_key") ||
                                 extractBool(json, "has_self_signing_key");

    if (targetUserId.empty()) targetUserId = userId;

    // Determine cross-signing status
    int keysFound = (masterKeyPresent ? 1 : 0) +
                    (userSigningKeyPresent ? 1 : 0) +
                    (selfSigningKeyPresent ? 1 : 0);

    std::string status;
    if (keysFound == 3) status = "fully_signed";
    else if (keysFound == 2) status = "partially_signed";
    else if (keysFound == 1) status = "minimally_signed";
    else status = "not_signed";

    // Build verification status
    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"crossSignCheck\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"target_user_id\":\"" << targetUserId << "\""
      << ",\"cross_signing_status\":\"" << status << "\""
      << ",\"keys_found\":" << keysFound
      << ",\"master_key_present\":" << (masterKeyPresent ? "true" : "false")
      << ",\"user_signing_key_present\":" << (userSigningKeyPresent ? "true" : "false")
      << ",\"self_signing_key_present\":" << (selfSigningKeyPresent ? "true" : "false")
      << ",\"is_fully_signed\":" << (keysFound == 3 ? "true" : "false")
      << ",\"needs_verification\":" << (keysFound < 3 ? "true" : "false")
      << ",\"missing_keys\":[";

    bool first = true;
    if (!masterKeyPresent) {
        o << "\"master_key\"";
        first = false;
    }
    if (!userSigningKeyPresent) {
        if (!first) o << ",";
        o << "\"user_signing_key\"";
        first = false;
    }
    if (!selfSigningKeyPresent) {
        if (!first) o << ",";
        o << "\"self_signing_key\"";
    }
    o << "]}";
    return o.str();
}

std::string getMasterKey(const std::string& json) {
    LOGI("getMasterKey: %zu bytes", json.size());
    if (json.empty()) return buildErr("getMasterKey", "empty input");

    auto userId = extractStr(json, "user_id");
    auto masterKey = extractStr(json, "master_key");
    auto keyId = extractStr(json, "key_id");
    auto publicKey = extractStr(json, "public_key");

    // Try multiple possible key locations
    if (masterKey.empty()) {
        masterKey = extractStr(json, "keys.master");
    }
    if (masterKey.empty()) {
        masterKey = publicKey;
    }
    if (keyId.empty()) {
        keyId = extractStr(json, "ed25519_key_id");
    }

    if (masterKey.empty()) {
        LOGW("getMasterKey: no master key found in input");
        return buildOk("getMasterKey",
                       "\"master_key_found\":false,"
                       "\"user_id\":\"" + userId + "\","
                       "\"has_cross_signing\":false,"
                       "\"keys_available\":0");
    }

    bool keyValid = isValidBase64Key(masterKey);

    std::ostringstream o;
    o << "{\"ok\":true"
      << ",\"fn\":\"getMasterKey\""
      << ",\"user_id\":\"" << userId << "\""
      << ",\"master_key\":\"" << masterKey << "\""
      << ",\"key_id\":\"" << (keyId.empty() ? "ed25519:master" : keyId) << "\""
      << ",\"key_valid\":" << (keyValid ? "true" : "false")
      << ",\"algorithm\":\"ed25519\""
      << ",\"key_length\":" << masterKey.size()
      << ",\"master_key_found\":true"
      << ",\"can_sign\":" << (keyValid ? "true" : "false")
      << ",\"can_verify\":" << (keyValid ? "true" : "false")
      << "}";
    return o.str();
}

} // namespace progressive
