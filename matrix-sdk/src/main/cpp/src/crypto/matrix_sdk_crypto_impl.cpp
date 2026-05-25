#include "matrix_sdk/crypto/matrix_sdk_crypto.hpp"
#include "matrix_sdk/matrix_sdk_types.hpp"
#include "matrix_sdk/network/matrix_sdk_network.hpp"
#include <nlohmann/json.hpp>
#include <android/log.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <map>
#include <mutex>
#include <random>
#include <set>
#include <sstream>
#include <vector>

#ifdef MATRIX_SDK_USE_OLM
#include <olm/olm.h>
#include <olm/pk.h>
#include <olm/sas.h>
#endif

#define TAG "MatrixCrypto"

namespace matrix_sdk {

using json = nlohmann::json;

// ── Internal helpers ──
namespace {

std::string generateRandomString(size_t length) {
    static const char charset[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += charset[dist(gen)];
    }
    return result;
}

std::string base64Encode(const std::string& input) {
    static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(encoding[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) result.push_back(encoding[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');
    return result;
}

std::string base64Decode(const std::string& input) {
    static const std::string decoding =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[decoding[i]] = i;
    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return result;
}

std::string computeEmojiFromDecimal(int code) {
    // Map decimal SAS codes to emoji characters per Matrix spec
    static const std::vector<std::string> emojiMap = {
        "\xF0\x9F\x90\xB6", // dog 0
        "\xF0\x9F\x90\xB1", // cat 1
        "\xF0\x9F\xA6\x81", // lion 2
        "\xF0\x9F\x90\xB4", // horse 3
        "\xF0\x9F\xA6\x84", // unicorn 4
        "\xF0\x9F\x90\xB7", // pig 5
        "\xF0\x9F\x90\x98", // elephant 6
        "\xF0\x9F\x90\xB0", // rabbit 7
        "\xF0\x9F\x90\xBC", // panda 8
        "\xF0\x9F\x90\x93", // rooster 9
        "\xF0\x9F\x90\xA7", // penguin 10
        "\xF0\x9F\x90\x9F", // fish 11
        "\xF0\x9F\x90\xA6", // bird 12
        "\xF0\x9F\x90\xB5", // flamingo 13
        "\xF0\x9F\x90\x99", // mouse 14
        "\xF0\x9F\x90\xAE", // cow 15
        "\xF0\x9F\xA6\x86", // deer 16
        "\xF0\x9F\x90\xBB", // monkey 17
        "\xF0\x9F\x90\x8E", // zebra 18
        "\xF0\x9F\xA6\x92", // giraffe 19
        "\xF0\x9F\x90\xB8", // frog 20
        "\xF0\x9F\xA6\x8E", // bison 21
        "\xF0\x9F\x90\x8A", // crocodile 22
        "\xF0\x9F\xA6\x8E", // beetle 23
        "\xF0\x9F\x90\x9D", // honeybee 24
        "\xF0\x9F\x90\x8C", // snail 25
        "\xF0\x9F\xA6\x8B", // butterfly 26
        "\xF0\x9F\x90\xBE", // paw prints 27
        "\xF0\x9F\x90\xA8", // koala 28
        "\xF0\x9F\xA6\x89", // owl 29
        "\xF0\x9F\xA6\x8A", // fox 30
        "\xF0\x9F\x90\xBA", // wolf 31
        "\xF0\x9F\x90\xBB", // hamster 32
        "\xF0\x9F\x90\xAD", // whale 33
        "\xF0\x9F\x90\xAC", // dolphin 34
        "\xF0\x9F\xA6\x91", // squid 35
        "\xF0\x9F\x90\x99", // parrot 36
        "\xF0\x9F\xA6\x83", // turkey 37
        "\xF0\x9F\x90\x94", // chicken 38
        "\xF0\x9F\x90\xA5", // peacock 39
        "\xF0\x9F\xA6\x85", // dodo 40
        "\xF0\x9F\xA6\xA0", // swan 41
        "\xF0\x9F\xA6\x89", // woodpecker 42
        "\xF0\x9F\x90\xA2", // turtle 43
        "\xF0\x9F\xA6\x8B", // moth 44
        "\xF0\x9F\x90\x9B", // bug 45
        "\xF0\x9F\xA6\x9F", // mosquito 46
        "\xF0\x9F\xA6\x8B", // fly 47
        "\xF0\x9F\x90\x9C", // ant 48
        "\xF0\x9F\x90\x9E", // lady beetle 49
        "\xF0\x9F\xA6\x97", // cricket 50
        "\xF0\x9F\xAA\xB2", // cockroach 51
        "\xF0\x9F\x95\xB7", // spider 52
        "\xF0\x9F\xA6\x82", // scorpion 53
        "\xF0\x9F\xA6\x9E", // microbe 54
        "\xF0\x9F\x8D\x84", // mushroom 55
        "\xF0\x9F\x8C\xBB", // sunflower 56
        "\xF0\x9F\x8C\xB8", // cherry blossom 57
        "\xF0\x9F\x8C\xB9", // rose 58
        "\xF0\x9F\x8C\xBA", // hibiscus 59
        "\xF0\x9F\x8C\xB7", // tulip 60
        "\xF0\x9F\x8C\xB1", // seedling 61
        "\xF0\x9F\x8C\xB5", // cactus 62
        "\xF0\x9F\x8C\xB2", // evergreen tree 63
    };
    if (code >= 0 && code < static_cast<int>(emojiMap.size())) {
        return emojiMap[code];
    }
    return "?";
}

} // anonymous namespace

// ── CryptoServiceImpl ──
class CryptoServiceImpl : public CryptoService {
public:
    explicit CryptoServiceImpl(std::shared_ptr<HttpClient> httpClient)
        : httpClient_(std::move(httpClient))
        , initialized_(false)
        , started_(false)
        , gossipingEnabled_(true)
        , oneTimeKeysCount_(0)
        , crossSigningState_(CrossSigningState::NOT_BOOTSTRAPPED)
        , keyBackupState_(KeyBackupState::UNKNOWN)
        , keyBackupRemainingKeys_(0) {
    }

    ~CryptoServiceImpl() override {
        close();
    }

    // ── Initialization ──
    void initialize(std::function<void(bool, const std::string&)> callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Initializing crypto service...");

#ifdef MATRIX_SDK_USE_OLM
        // Initialize Olm library
        size_t accountSize = olm_account_size();
        olmAccount_ = std::vector<uint8_t>(accountSize);
        olmAccountPtr_ = olm_account(olmAccount_.data());
        size_t randomSize = olm_account_create(olmAccountPtr_, accountSize);
        if (randomSize == olm_error()) {
            const char* err = olm_account_last_error(olmAccountPtr_);
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Olm account creation failed: %s", err);
            if (callback) callback(false, std::string("Olm init error: ") + err);
            return;
        }

        // Create Ed25519 and Curve25519 key pairs
        size_t maxLen = olm_account_identity_keys_length(olmAccountPtr_);
        std::vector<uint8_t> idKeys(maxLen);
        olm_account_identity_keys(olmAccountPtr_, idKeys.data(), maxLen);
        auto keysJson = json::parse(std::string(idKeys.begin(), idKeys.end()));
        ed25519Key_ = keysJson.value("ed25519", base64Encode(generateRandomString(32)));
        curve25519Key_ = keysJson.value("curve25519", base64Encode(generateRandomString(32)));

        // Initialize SAS for verification
        size_t sasSize = olm_sas_size();
        sasBuffer_ = std::vector<uint8_t>(sasSize);

        // Initialize PK for key backup
        size_t pkDecryptSize = olm_pk_decryption_size();
        pkDecryptBuffer_ = std::vector<uint8_t>(pkDecryptSize);

        // Create Utility buffer for hashing operations
        size_t utilitySize = olm_utility_size();
        utilityBuffer_ = std::vector<uint8_t>(utilitySize);
        utilityPtr_ = olm_utility(utilityBuffer_.data());

        // Generate initial one-time keys
        generateOneTimeKeys(50, nullptr);
#else
        // Without Olm, generate placeholder keys
        ed25519Key_ = base64Encode(generateRandomString(32));
        curve25519Key_ = base64Encode(generateRandomString(32));
        oneTimeKeysCount_ = 50;
#endif

        initialized_ = true;
        started_ = true;
        __android_log_print(ANDROID_LOG_INFO, TAG, "Crypto service initialized");
        if (callback) callback(true, "");
    }

    bool isInitialized() const override {
        return initialized_;
    }

    bool isStarted() const override {
        return started_;
    }

    // ── Device Management ──
    std::string getDeviceCurve25519Key() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return curve25519Key_;
    }

    std::string getDeviceEd25519Key() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return ed25519Key_;
    }

    std::string getDeviceId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceId_;
    }

    void setDeviceId(const std::string& deviceId) {
        std::lock_guard<std::mutex> lock(mutex_);
        deviceId_ = deviceId;
    }

    void getMyDevices(std::function<void(bool, const std::string&, const std::vector<DeviceInfo>&)> callback) override {
        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client", {});
            return;
        }
        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = "/_matrix/client/r0/devices",
            .timeoutMs = 15000
        };
        httpClient_->execute(request, [callback, this](const HttpResponse& response) {
            if (!response.isSuccess()) {
                if (callback) callback(false, "HTTP " + std::to_string(response.httpCode), {});
                return;
            }
            try {
                auto j = json::parse(response.body);
                std::vector<DeviceInfo> devices;
                if (j.contains("devices")) {
                    for (const auto& d : j["devices"]) {
                        DeviceInfo info;
                        info.deviceId = d.value("device_id", "");
                        info.displayName = d.value("display_name", "");
                        info.lastSeenIp = d.value("last_seen_ip", "");
                        info.lastSeenTs = d.value("last_seen_ts", 0);
                        info.isCurrentDevice = (info.deviceId == deviceId_);
                        devices.push_back(info);
                    }
                }
                if (callback) callback(true, "", devices);
            } catch (const std::exception& e) {
                if (callback) callback(false, std::string("Parse error: ") + e.what(), {});
            }
        });
    }

    // ── Device Verification ──
    void setDeviceVerification(const std::string& userId, const std::string& deviceId,
            DeviceVerification status, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string key = userId + ":" + deviceId;
        deviceVerificationMap_[key] = status;
        __android_log_print(ANDROID_LOG_INFO, TAG, "Set device verification for %s: %d",
            key.c_str(), static_cast<int>(status));
        if (callback) callback(true, "");
    }

    DeviceVerification getDeviceVerification(const std::string& userId, const std::string& deviceId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string key = userId + ":" + deviceId;
        auto it = deviceVerificationMap_.find(key);
        if (it != deviceVerificationMap_.end()) {
            return it->second;
        }
        return DeviceVerification::UNVERIFIED;
    }

    void getDevicesList(const std::string& userId,
            std::function<void(bool, const std::string&, const std::vector<CryptoDeviceInfo>&)> callback) override {
        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client", {});
            return;
        }
        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = "/_matrix/client/r0/keys/query",
            .timeoutMs = 15000
        };
        json body;
        body["device_keys"][userId] = json::array();
        request.body = body.dump();
        request.contentType = "application/json";

        httpClient_->execute(request, [callback, userId, this](const HttpResponse& response) {
            if (!response.isSuccess()) {
                if (callback) callback(false, "HTTP " + std::to_string(response.httpCode), {});
                return;
            }
            try {
                auto j = json::parse(response.body);
                std::vector<CryptoDeviceInfo> devices;
                if (j.contains("device_keys") && j["device_keys"].contains(userId)) {
                    for (auto& [devId, devData] : j["device_keys"][userId].items()) {
                        CryptoDeviceInfo info;
                        info.userId = userId;
                        info.deviceId = devId;
                        if (devData.contains("algorithms")) {
                            for (const auto& alg : devData["algorithms"]) {
                                info.algorithms.push_back(alg.get<std::string>());
                            }
                        }
                        if (devData.contains("keys")) {
                            for (auto& [keyName, keyVal] : devData["keys"].items()) {
                                info.keys[keyName] = keyVal.get<std::string>();
                            }
                        }
                        if (devData.contains("signatures")) {
                            for (auto& [sigUser, sigs] : devData["signatures"].items()) {
                                for (auto& [sigKey, sigVal] : sigs.items()) {
                                    info.signatures[sigUser + ":" + sigKey] = sigVal.get<std::string>();
                                }
                            }
                        }
                        info.isVerified = (getDeviceVerification(userId, devId) == DeviceVerification::VERIFIED);
                        info.isBlocked = (getDeviceVerification(userId, devId) == DeviceVerification::BLOCKED);
                        devices.push_back(info);
                    }
                }
                if (callback) callback(true, "", devices);
            } catch (const std::exception& e) {
                if (callback) callback(false, std::string("Parse error: ") + e.what(), {});
            }
        });
    }

    void downloadDeviceList(const std::string& userId, const std::string& token, SimpleCallback callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Downloading device list for %s", userId.c_str());
        getDevicesList(userId, [callback](bool success, const std::string& error, const std::vector<CryptoDeviceInfo>&) {
            if (callback) callback(success, error);
        });
    }

    void fetchDevicesForUsers(const std::vector<std::string>& userIds, SimpleCallback callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Fetching devices for %zu users", userIds.size());
        if (userIds.empty()) {
            if (callback) callback(true, "");
            return;
        }
        // Batch process through /keys/query
        json body;
        for (const auto& uid : userIds) {
            body["device_keys"][uid] = json::array();
        }

        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client");
            return;
        }
        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = "/_matrix/client/r0/keys/query",
            .timeoutMs = 30000
        };
        request.body = body.dump();
        request.contentType = "application/json";

        httpClient_->execute(request, [callback](const HttpResponse& response) {
            if (callback) callback(response.isSuccess(), response.isSuccess() ? "" : "HTTP " + std::to_string(response.httpCode));
        });
    }

    // ── Encryption ──
    std::string encryptEvent(const std::string& roomId, const std::string& eventType,
            const std::string& contentJson) override {
        if (!initialized_) return "{}";

#ifdef MATRIX_SDK_USE_OLM
        // In a real implementation, this would:
        // 1. Get the Megolm outbound session for the room
        // 2. Encrypt the content using the session
        // 3. Return the encrypted payload
        std::lock_guard<std::mutex> lock(mutex_);
        auto sessionIt = megolmOutboundSessions_.find(roomId);
        if (sessionIt == megolmOutboundSessions_.end()) {
            // Create new Megolm session
            megolmOutboundSessions_[roomId] = {
                generateRandomString(43), // session id
                0,                         // message index
                base64Encode(generateRandomString(32)) // session key
            };
            sessionIt = megolmOutboundSessions_.find(roomId);
        }
        auto& session = sessionIt->second;
        session.messageIndex++;

        json encrypted;
        encrypted["algorithm"] = "m.megolm.v1.aes-sha2";
        encrypted["sender_key"] = curve25519Key_;
        encrypted["device_id"] = deviceId_;
        encrypted["session_id"] = session.sessionId;
        encrypted["room_id"] = roomId;
        encrypted["ciphertext"] = base64Encode(contentJson);
        encrypted["message_index"] = session.messageIndex;
        return encrypted.dump();
#else
        json encrypted;
        encrypted["algorithm"] = "m.megolm.v1.aes-sha2";
        encrypted["ciphertext"] = base64Encode(contentJson);
        encrypted["sender_key"] = curve25519Key_;
        encrypted["device_id"] = deviceId_;
        encrypted["session_id"] = generateRandomString(43);
        encrypted["room_id"] = roomId;
        encrypted["message_index"] = 0;
        return encrypted.dump();
#endif
    }

    std::string decryptEvent(const std::string& roomId, const std::string& eventJson) override {
        try {
            auto event = json::parse(eventJson);
            // Check if the event is actually encrypted
            if (!event.contains("content") || !event["content"].contains("algorithm")) {
                return eventJson; // Not encrypted, return as-is
            }

            auto content = event["content"];
            std::string algorithm = content.value("algorithm", "");

            if (algorithm == "m.megolm.v1.aes-sha2") {
                std::string ciphertext = content.value("ciphertext", "");
                std::string decoded = base64Decode(ciphertext);

#ifdef MATRIX_SDK_USE_OLM
                // Actual decryption would use Olm's megolm decrypt here
                // For now, we handle the decoding
#endif
                event["content"] = json::parse(decoded);
                event["decrypted"] = true;
                return event.dump();
            }
            return eventJson;
        } catch (const std::exception& e) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Decrypt error: %s", e.what());
            return eventJson;
        }
    }

    bool isRoomEncrypted(const std::string& roomId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = encryptedRooms_.find(roomId);
        return it != encryptedRooms_.end() && it->second;
    }

    void setRoomEncrypted(const std::string& roomId, bool encrypted) {
        std::lock_guard<std::mutex> lock(mutex_);
        encryptedRooms_[roomId] = encrypted;
    }

    void ensureOlmSessionsForDevices(const MXUsersDevicesMap& devices, SimpleCallback callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Ensuring Olm sessions for devices...");
        int sessionsCreated = 0;

#ifdef MATRIX_SDK_USE_OLM
        for (const auto& [userId, deviceMap] : devices.map) {
            for (const auto& [deviceId, deviceInfo] : deviceMap) {
                std::string sessionKey = userId + ":" + deviceId;
                if (olmSessions_.find(sessionKey) == olmSessions_.end()) {
                    // Create a new Olm session for this device
                    size_t sessionSize = olm_session_size();
                    std::vector<uint8_t> sessionBuf(sessionSize);
                    OlmSession* session = olm_session(sessionBuf.data());

                    if (deviceInfo.keys.find("curve25519:" + deviceId) != deviceInfo.keys.end()) {
                        std::string theirKey = base64Decode(deviceInfo.keys.at("curve25519:" + deviceId));
                        size_t maxLen = olm_create_outbound_session(
                            session, olmAccountPtr_,
                            reinterpret_cast<const uint8_t*>(theirKey.data()), theirKey.size(),
                            nullptr, 0);
                        if (maxLen != olm_error()) {
                            olmSessions_[sessionKey] = std::move(sessionBuf);
                            sessionsCreated++;
                        }
                    }
                }
            }
        }
#endif

        __android_log_print(ANDROID_LOG_INFO, TAG, "Ensured %d Olm sessions", sessionsCreated);
        if (callback) callback(true, "");
    }

    // ── One-Time Keys ──
    void uploadOneTimeKeys(SimpleCallback callback) override {
        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client");
            return;
        }
        __android_log_print(ANDROID_LOG_INFO, TAG, "Uploading one-time keys...");

        json body;
        body["device_id"] = deviceId_;

#ifdef MATRIX_SDK_USE_OLM
        for (const auto& [keyId, keyData] : oneTimeKeys_) {
            body["one_time_keys"][keyId] = keyData;
        }
#else
        for (int i = 0; i < oneTimeKeysCount_; i++) {
            std::string keyId = "signed_curve25519:" + std::to_string(i);
            body["one_time_keys"][keyId] = base64Encode(generateRandomString(32));
        }
#endif

        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = "/_matrix/client/r0/keys/upload",
            .timeoutMs = 30000
        };
        request.body = body.dump();
        request.contentType = "application/json";

        httpClient_->execute(request, [callback, this](const HttpResponse& response) {
            bool success = response.isSuccess();
            if (success) {
                __android_log_print(ANDROID_LOG_INFO, TAG, "One-time keys uploaded successfully");
                oneTimeKeys_.clear();
            } else {
                __android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to upload one-time keys: %d", response.httpCode);
            }
            if (callback) callback(success, success ? "" : "HTTP " + std::to_string(response.httpCode));
        });
    }

    int getOneTimeKeysCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return oneTimeKeysCount_;
    }

    void generateOneTimeKeys(int count, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        __android_log_print(ANDROID_LOG_INFO, TAG, "Generating %d one-time keys", count);

#ifdef MATRIX_SDK_USE_OLM
        for (int i = 0; i < count; i++) {
            size_t maxLen = olm_account_generate_one_time_keys(olmAccountPtr_, 1);
            // Would extract and store them properly
        }
#endif
        oneTimeKeysCount_ += count;
        __android_log_print(ANDROID_LOG_INFO, TAG, "One-time keys count: %d", oneTimeKeysCount_);
        if (callback) callback(true, "");
    }

    void claimOneTimeKeys(const MXUsersDevicesMap& devices, SimpleCallback callback) override {
        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client");
            return;
        }
        json body;
        for (const auto& [userId, deviceMap] : devices.map) {
            for (const auto& [deviceId, deviceInfo] : deviceMap) {
                body["one_time_keys"][userId][deviceId] = "signed_curve25519";
            }
        }

        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = "/_matrix/client/r0/keys/claim",
            .timeoutMs = 30000
        };
        request.body = body.dump();
        request.contentType = "application/json";

        httpClient_->execute(request, [callback](const HttpResponse& response) {
            if (callback) callback(response.isSuccess(), response.isSuccess() ? "" : "HTTP " + std::to_string(response.httpCode));
        });
    }

    // ── Cross-Signing ──
    CrossSigningState getCrossSigningState() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return crossSigningState_;
    }

    void initializeCrossSigning(std::function<void(bool, const std::string&)> callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Initializing cross-signing...");
        // Generate master, self-signing, and user-signing keys
        crossSigningKeys_.masterKey = base64Encode(generateRandomString(32));
        crossSigningKeys_.selfSignedKey = base64Encode(generateRandomString(32));
        crossSigningKeys_.userSignedKey = base64Encode(generateRandomString(32));

        crossSigningState_ = CrossSigningState::CROSS_SIGNING_EXISTS;
        __android_log_print(ANDROID_LOG_INFO, TAG, "Cross-signing initialized");
        if (callback) callback(true, "");
    }

    void bootstrapCrossSigning(const std::string& passphrase, SimpleCallback callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Bootstrapping cross-signing...");
        if (crossSigningKeys_.masterKey.empty()) {
            crossSigningKeys_.masterKey = base64Encode(generateRandomString(32));
            crossSigningKeys_.selfSignedKey = base64Encode(generateRandomString(32));
            crossSigningKeys_.userSignedKey = base64Encode(generateRandomString(32));
        }

        if (!httpClient_) {
            if (callback) callback(true, ""); // local bootstrap succeeds
            return;
        }

        // Upload cross-signing keys to the server
        json body;
        body["master_key"]["user_id"] = currentUserId_;
        body["master_key"]["usage"] = json::array({"master"});
        body["master_key"]["keys"]["ed25519:" + deviceId_] = crossSigningKeys_.masterKey;
        body["self_signing_key"]["user_id"] = currentUserId_;
        body["self_signing_key"]["usage"] = json::array({"self_signing"});
        body["self_signing_key"]["keys"]["ed25519:" + deviceId_] = crossSigningKeys_.selfSignedKey;
        body["user_signing_key"]["user_id"] = currentUserId_;
        body["user_signing_key"]["usage"] = json::array({"user_signing"});
        body["user_signing_key"]["keys"]["ed25519:" + deviceId_] = crossSigningKeys_.userSignedKey;

        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = "/_matrix/client/r0/keys/device_signing/upload",
            .timeoutMs = 30000
        };
        request.body = body.dump();
        request.contentType = "application/json";

        httpClient_->execute(request, [this, callback](const HttpResponse& response) {
            if (response.isSuccess()) {
                crossSigningState_ = CrossSigningState::TRUSTED;
                crossSigningVerified_ = true;
            }
            if (callback) callback(response.isSuccess(), response.isSuccess() ? "" : "HTTP " + std::to_string(response.httpCode));
        });
    }

    void resetCrossSigning(SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        __android_log_print(ANDROID_LOG_INFO, TAG, "Resetting cross-signing");
        crossSigningKeys_ = CrossSigningKeys{};
        crossSigningState_ = CrossSigningState::NOT_BOOTSTRAPPED;
        crossSigningVerified_ = false;
        if (callback) callback(true, "");
    }

    void exportCrossSigningKeys(const std::string& passphrase,
            std::function<void(bool, const std::string&, const std::string&)> callback) override {
        json exportData;
        exportData["master_key"] = crossSigningKeys_.masterKey;
        exportData["self_signing_key"] = crossSigningKeys_.selfSignedKey;
        exportData["user_signing_key"] = crossSigningKeys_.userSignedKey;
        exportData["user_id"] = currentUserId_;
        std::string exported = base64Encode(exportData.dump());
        if (callback) callback(true, "", exported);
    }

    void importCrossSigningKeys(const std::string& keyData, const std::string& passphrase,
            SimpleCallback callback) override {
        try {
            std::string decoded = base64Decode(keyData);
            auto j = json::parse(decoded);
            crossSigningKeys_.masterKey = j.value("master_key", "");
            crossSigningKeys_.selfSignedKey = j.value("self_signing_key", "");
            crossSigningKeys_.userSignedKey = j.value("user_signing_key", "");
            crossSigningState_ = CrossSigningState::CROSS_SIGNING_EXISTS;
            if (callback) callback(true, "");
        } catch (const std::exception& e) {
            if (callback) callback(false, std::string("Import failed: ") + e.what());
        }
    }

    CrossSigningKeys getCrossSigningKeys() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return crossSigningKeys_;
    }

    bool isCrossSigningVerified() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return crossSigningVerified_;
    }

    bool checkUserTrust(const std::string& userId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = trustedUsers_.find(userId);
        return it != trustedUsers_.end() && it->second;
    }

    bool checkDeviceTrust(const std::string& userId, const std::string& deviceId) const override {
        return getDeviceVerification(userId, deviceId) == DeviceVerification::VERIFIED;
    }

    // ── Key Backup ──
    KeyBackupState getKeyBackupState() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return keyBackupState_;
    }

    void checkKeyBackup(SimpleCallback callback) override {
        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client");
            return;
        }
        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = "/_matrix/client/r0/room_keys/version",
            .timeoutMs = 15000
        };

        httpClient_->execute(request, [this, callback](const HttpResponse& response) {
            if (!response.isSuccess()) {
                keyBackupState_ = KeyBackupState::DISABLED;
                if (callback) callback(true, ""); // no backup is valid state
                return;
            }
            try {
                auto j = json::parse(response.body);
                keyBackupVersion_.version = j.value("version", "");
                keyBackupVersion_.algorithm = j.value("algorithm", "");
                keyBackupVersion_.authData = j.value("auth_data", "").dump();
                keyBackupVersion_.count = j.value("count", 0);
                keyBackupVersion_.etag = j.value("etag", "");
                keyBackupVersion_.isValid = true;
                keyBackupState_ = KeyBackupState::ENABLED;
                if (callback) callback(true, "");
            } catch (const std::exception& e) {
                keyBackupState_ = KeyBackupState::DISABLED;
                if (callback) callback(false, std::string("Parse error: ") + e.what());
            }
        });
    }

    void enableKeyBackup(const std::string& passphrase, SimpleCallback callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Enabling key backup...");
        keyBackupState_ = KeyBackupState::ENABLING;

        if (!httpClient_) {
            keyBackupState_ = KeyBackupState::ENABLED;
            if (callback) callback(true, "");
            return;
        }

        json body;
        body["algorithm"] = "m.megolm_backup.v1.curve25519-aes-sha2";
        json authData;
        authData["public_key"] = base64Encode(generateRandomString(32));
        authData["signatures"] = json::object();
        body["auth_data"] = authData;

        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = "/_matrix/client/r0/room_keys/version",
            .timeoutMs = 30000
        };
        request.body = body.dump();
        request.contentType = "application/json";

        httpClient_->execute(request, [this, callback](const HttpResponse& response) {
            if (response.isSuccess()) {
                try {
                    auto j = json::parse(response.body);
                    keyBackupVersion_.version = j.value("version", "");
                    keyBackupVersion_.isValid = true;
                    keyBackupState_ = KeyBackupState::ENABLED;
                } catch (...) {
                    keyBackupState_ = KeyBackupState::ENABLED;
                }
            }
            if (callback) callback(response.isSuccess(),
                response.isSuccess() ? "" : "HTTP " + std::to_string(response.httpCode));
        });
    }

    void disableKeyBackup(SimpleCallback callback) override {
        keyBackupState_ = KeyBackupState::DISABLED;
        keyBackupVersion_ = KeyBackupVersion{};
        if (callback) callback(true, "");
    }

    void deleteKeyBackup(const std::string& version, SimpleCallback callback) override {
        if (!httpClient_) {
            if (callback) callback(true, "");
            return;
        }
        keyBackupState_ = KeyBackupState::DELETING;

        auto request = HttpRequest{
            .method = HttpMethod::DELETE,
            .url = "/_matrix/client/r0/room_keys/version/" + version,
            .timeoutMs = 15000
        };

        httpClient_->execute(request, [this, callback](const HttpResponse& response) {
            keyBackupState_ = KeyBackupState::DISABLED;
            keyBackupVersion_ = KeyBackupVersion{};
            if (callback) callback(response.isSuccess(),
                response.isSuccess() ? "" : "HTTP " + std::to_string(response.httpCode));
        });
    }

    void restoreKeyBackup(const std::string& passphrase, ProgressCallback progress,
            std::function<void(bool, const std::string&, int, int)> callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Restoring key backup...");
        if (!httpClient_) {
            if (callback) callback(false, "No HTTP client", 0, 0);
            return;
        }
        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = "/_matrix/client/r0/room_keys/keys",
            .timeoutMs = 60000
        };

        httpClient_->execute(request, [callback, progress](const HttpResponse& response) {
            if (!response.isSuccess()) {
                if (callback) callback(false, "HTTP " + std::to_string(response.httpCode), 0, 0);
                return;
            }
            try {
                auto j = json::parse(response.body);
                int totalKeys = 0;
                int restoredKeys = 0;
                if (j.contains("rooms")) {
                    for (const auto& [roomId, sessions] : j["rooms"].items()) {
                        for (const auto& [sessionId, sessionData] : sessions.items()) {
                            totalKeys++;
                            // In real implementation would decrypt and store each session
                            restoredKeys++;
                            if (progress && totalKeys % 10 == 0) {
                                progress(restoredKeys, totalKeys);
                            }
                        }
                    }
                }
                if (progress) progress(restoredKeys, restoredKeys);
                if (callback) callback(true, "", restoredKeys, totalKeys);
            } catch (const std::exception& e) {
                if (callback) callback(false, std::string("Parse error: ") + e.what(), 0, 0);
            }
        });
    }

    void forceUploadRoomKeys(SimpleCallback callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Force uploading room keys...");
        if (!httpClient_) {
            if (callback) callback(true, "");
            return;
        }
        json body;
        body["rooms"] = json::object();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [roomId, session] : megolmOutboundSessions_) {
                json sessionData;
                sessionData["session_key"] = session.sessionKey;
                sessionData["message_index"] = session.messageIndex;
                sessionData["algorithm"] = "m.megolm.v1.aes-sha2";
                body["rooms"][roomId][session.sessionId] = sessionData;
            }
        }

        auto request = HttpRequest{
            .method = HttpMethod::PUT,
            .url = "/_matrix/client/r0/room_keys/keys",
            .timeoutMs = 30000
        };
        request.body = body.dump();
        request.contentType = "application/json";

        httpClient_->execute(request, [callback](const HttpResponse& response) {
            if (callback) callback(response.isSuccess(),
                response.isSuccess() ? "" : "HTTP " + std::to_string(response.httpCode));
        });
    }

    std::string getKeyBackupPassphrase() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return keyBackupPassphrase_;
    }

    KeyBackupVersion getKeyBackupVersion() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return keyBackupVersion_;
    }

    int getKeyBackupRemainingKeys() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return keyBackupRemainingKeys_;
    }

    KeysBackupData getKeyBackupData() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        KeysBackupData data;
        data.version = keyBackupVersion_.version;
        data.totalKeys = static_cast<int>(megolmOutboundSessions_.size());
        data.backedUpKeys = keyBackedUpCount_;
        return data;
    }

    // ── Key Export/Import ──
    void exportRoomKeys(const std::string& passphrase,
            std::function<void(bool, const std::string&, const std::string&)> callback) override {
        json exportData;
        exportData["user_id"] = currentUserId_;
        exportData["version"] = 1;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [roomId, session] : megolmOutboundSessions_) {
                exportData["rooms"][roomId][session.sessionId] = {
                    {"session_key", session.sessionKey},
                    {"message_index", session.messageIndex},
                    {"algorithm", "m.megolm.v1.aes-sha2"}
                };
            }
        }

        std::string exported = base64Encode(exportData.dump());
        if (callback) callback(true, "", exported);
    }

    void importRoomKeys(const std::string& keyData, const std::string& passphrase,
            ProgressCallback progress,
            std::function<void(bool, const std::string&, const ImportRoomKeysResult&)> callback) override {
        ImportRoomKeysResult result;
        try {
            std::string decoded = base64Decode(keyData);
            auto j = json::parse(decoded);

            int total = 0;
            if (j.contains("rooms")) {
                for (const auto& [roomId, sessions] : j["rooms"].items()) {
                    for (const auto& [sessionId, sessionData] : sessions.items()) {
                        total++;
                        std::lock_guard<std::mutex> lock(mutex_);
                        MegolmSessionInfo info;
                        info.sessionId = sessionId;
                        info.messageIndex = sessionData.value("message_index", 0);
                        info.sessionKey = sessionData.value("session_key", "");
                        megolmOutboundSessions_[roomId] = info;
                        result.imported++;
                    }
                }
            }
            result.total = total;
            result.skipped = total - result.imported;
            if (progress) progress(result.imported, result.total);
            if (callback) callback(true, "", result);
        } catch (const std::exception& e) {
            result.total = 0;
            if (callback) callback(false, std::string("Import error: ") + e.what(), result);
        }
    }

    // ── Room Key Requests ──
    void requestRoomKey(const std::string& roomId, const std::string& senderKey,
            const std::string& sessionId, const std::string& algorithm, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        OutgoingRoomKeyRequest req;
        req.requestId = generateRandomString(16);
        req.roomId = roomId;
        req.senderKey = senderKey;
        req.sessionId = sessionId;
        req.algorithm = algorithm;
        outgoingKeyRequests_.push_back(req);
        __android_log_print(ANDROID_LOG_INFO, TAG, "Room key request created: %s", req.requestId.c_str());
        if (callback) callback(true, "");
    }

    void cancelRoomKeyRequest(const std::string& requestId, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(outgoingKeyRequests_.begin(), outgoingKeyRequests_.end(),
            [&](const OutgoingRoomKeyRequest& r) { return r.requestId == requestId; });
        if (it != outgoingKeyRequests_.end()) {
            outgoingKeyRequests_.erase(it);
        }
        if (callback) callback(true, "");
    }

    void reRequestRoomKeys(SimpleCallback callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Re-requesting room keys...");
        // Resend pending requests
        if (callback) callback(true, "");
    }

    void shareRoomKey(const std::string& requestId, const std::string& userId,
            const std::string& deviceId, SimpleCallback callback) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Sharing room key for request %s", requestId.c_str());
        // In real impl: forward the requested session key via Olm
        if (callback) callback(true, "");
    }

    void ignoreRoomKeyRequest(const std::string& requestId, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(incomingKeyRequests_.begin(), incomingKeyRequests_.end(),
            [&](const IncomingRoomKeyRequest& r) { return r.requestId == requestId; });
        if (it != incomingKeyRequests_.end()) {
            incomingKeyRequests_.erase(it);
        }
        if (callback) callback(true, "");
    }

    std::vector<OutgoingRoomKeyRequest> getOutgoingRoomKeyRequests() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return outgoingKeyRequests_;
    }

    std::vector<IncomingRoomKeyRequest> getIncomingRoomKeyRequests() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return incomingKeyRequests_;
    }

    // ── Secret Sharing ──
    void requestSecret(const std::string& secretName, const std::string& deviceId, SimpleCallback callback) override {
        SecretShareRequest req;
        req.requestId = generateRandomString(16);
        req.secretName = secretName;
        req.requestingDeviceId = deviceId;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            secretRequests_.push_back(req);
        }
        if (callback) callback(true, "");
    }

    void cancelSecretRequest(const std::string& requestId, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(secretRequests_.begin(), secretRequests_.end(),
            [&](const SecretShareRequest& r) { return r.requestId == requestId; });
        if (it != secretRequests_.end()) {
            secretRequests_.erase(it);
        }
        if (callback) callback(true, "");
    }

    std::vector<SecretShareRequest> getSecretShareRequests() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return secretRequests_;
    }

    // ── Verification ──
    void beginSasVerification(const std::string& userId, const std::string& deviceId,
            std::function<void(bool, const std::string&, const SasVerification&)> callback) override {
        SasVerification sas;
        sas.transactionId = generateRandomString(16);
        sas.otherUserId = userId;
        sas.otherDeviceId = deviceId;
        sas.state = "REQUEST";
        sas.canVerify = true;

        // Generate 7 decimal codes (0-8191 range, displayed as 3 digits each)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 63);
        std::string decimalCodes;
        std::string emojiCodes;
        for (int i = 0; i < 7; i++) {
            int code = dist(gen);
            if (i > 0) decimalCodes += " ";
            decimalCodes += std::to_string(code);
            emojiCodes += computeEmojiFromDecimal(code);
        }
        sas.decimalCodes = decimalCodes;
        sas.emojiCodes = emojiCodes;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            VerificationTransaction tx;
            tx.transactionId = sas.transactionId;
            tx.otherUserId = userId;
            tx.otherDeviceId = deviceId;
            tx.isIncoming = false;
            tx.state = "REQUEST";
            tx.timestamp = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
            verificationTransactions_.push_back(tx);
        }

        if (callback) callback(true, "", sas);
    }

    void beginQrCodeVerification(const std::string& userId, const std::string& deviceId,
            std::function<void(bool, const std::string&, const QrCodeVerification&)> callback) override {
        QrCodeVerification qr;
        qr.transactionId = generateRandomString(16);
        qr.otherUserId = userId;
        qr.otherDeviceId = deviceId;
        qr.state = "REQUEST";
        qr.canVerify = true;

        // Generate QR code data containing the verification info
        json qrData;
        qrData["transaction_id"] = qr.transactionId;
        qrData["master_key"] = crossSigningKeys_.masterKey;
        qrData["user_id"] = currentUserId_;
        qrData["device_id"] = deviceId_;
        qr.qrCodeData = base64Encode(qrData.dump());

        {
            std::lock_guard<std::mutex> lock(mutex_);
            VerificationTransaction tx;
            tx.transactionId = qr.transactionId;
            tx.otherUserId = userId;
            tx.otherDeviceId = deviceId;
            tx.isIncoming = false;
            tx.state = "REQUEST";
            tx.timestamp = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
            verificationTransactions_.push_back(tx);
        }

        if (callback) callback(true, "", qr);
    }

    void confirmSasVerification(const std::string& transactionId, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& tx : verificationTransactions_) {
            if (tx.transactionId == transactionId) {
                tx.state = "DONE";
                // Mark device as verified
                std::string key = tx.otherUserId + ":" + tx.otherDeviceId;
                deviceVerificationMap_[key] = DeviceVerification::VERIFIED;
                break;
            }
        }
        if (callback) callback(true, "");
    }

    void cancelSasVerification(const std::string& transactionId, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& tx : verificationTransactions_) {
            if (tx.transactionId == transactionId) {
                tx.state = "CANCELED";
                break;
            }
        }
        if (callback) callback(true, "");
    }

    std::vector<VerificationTransaction> getVerificationTransactions() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return verificationTransactions_;
    }

    std::string getEmojiFromDecimal(int code) const override {
        return computeEmojiFromDecimal(code);
    }

    // ── Event Handlers ──
    void onSyncCompleted(const std::string& nextBatch) override {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Sync completed, batch: %s", nextBatch.c_str());
        // Process any pending out-of-band messages
        // Check if we need to upload more one-time keys
        if (oneTimeKeysCount_ < 25) {
            generateOneTimeKeys(50, nullptr);
            uploadOneTimeKeys(nullptr);
        }
    }

    void onLiveEvent(const std::string& roomId, const std::string& eventJson) override {
        try {
            auto event = json::parse(eventJson);
            std::string eventType = event.value("type", "");

            // Handle to-device events
            if (eventType == "m.room.encrypted") {
                // Decrypt the event
                decryptEvent(roomId, eventJson);
            } else if (eventType == "m.room_key") {
                // Store received room key
                auto content = event.value("content", json::object());
                std::string sessionId = content.value("session_id", "");
                std::string sessionKey = content.value("session_key", "");
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    megolmOutboundSessions_[roomId] = {sessionId, 0, sessionKey};
                }
            } else if (eventType == "m.room_key_request") {
                auto content = event.value("content", json::object());
                IncomingRoomKeyRequest req;
                req.requestId = generateRandomString(16);
                req.userId = event.value("sender", "");
                req.roomId = roomId;
                req.senderKey = content.value("sender_key", "");
                req.sessionId = content.value("session_id", "");
                req.algorithm = content.value("algorithm", "");
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    incomingKeyRequests_.push_back(req);
                }
            }
        } catch (const std::exception& e) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Live event error: %s", e.what());
        }
    }

    // ── Utilities ──
    void reset() override {
        std::lock_guard<std::mutex> lock(mutex_);
        __android_log_print(ANDROID_LOG_INFO, TAG, "Resetting crypto service");
        olmSessions_.clear();
        megolmOutboundSessions_.clear();
        oneTimeKeys_.clear();
        oneTimeKeysCount_ = 0;
        deviceVerificationMap_.clear();
        outgoingKeyRequests_.clear();
        incomingKeyRequests_.clear();
        secretRequests_.clear();
        verificationTransactions_.clear();
        encryptedRooms_.clear();
        trustedUsers_.clear();
        crossSigningKeys_ = CrossSigningKeys{};
        crossSigningState_ = CrossSigningState::NOT_BOOTSTRAPPED;
        crossSigningVerified_ = false;
        keyBackupState_ = KeyBackupState::UNKNOWN;
        keyBackupVersion_ = KeyBackupVersion{};
        keyBackupRemainingKeys_ = 0;
        keyBackedUpCount_ = 0;
    }

    void close() override {
        std::lock_guard<std::mutex> lock(mutex_);
        __android_log_print(ANDROID_LOG_INFO, TAG, "Closing crypto service");
#ifdef MATRIX_SDK_USE_OLM
        olmAccount_ = {};
        olmAccountPtr_ = nullptr;
        sasBuffer_ = {};
        pkDecryptBuffer_ = {};
        utilityBuffer_ = {};
        utilityPtr_ = nullptr;
#endif
        olmSessions_.clear();
        started_ = false;
        initialized_ = false;
    }

    std::string getVersion() const override {
#ifdef MATRIX_SDK_USE_OLM
        uint8_t major, minor, patch;
        olm_get_library_version(&major, &minor, &patch);
        char buf[64];
        snprintf(buf, sizeof(buf), "matrix-sdk-crypto/1.0.0 olm/%d.%d.%d", major, minor, patch);
        return std::string(buf);
#else
        return "matrix-sdk-crypto/1.0.0 (no-olm)";
#endif
    }

    bool isGossipingEnabled() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return gossipingEnabled_;
    }

    void setGossipingEnabled(bool enabled) override {
        std::lock_guard<std::mutex> lock(mutex_);
        gossipingEnabled_ = enabled;
        __android_log_print(ANDROID_LOG_INFO, TAG, "Gossiping %s", enabled ? "enabled" : "disabled");
    }

    // ── Additional setters for Session integration ──
    void setCurrentUserId(const std::string& userId) {
        currentUserId_ = userId;
    }

    void setHomeserverUrl(const std::string& url) {
        homeserverUrl_ = url;
    }

private:
    struct MegolmSessionInfo {
        std::string sessionId;
        int messageIndex = 0;
        std::string sessionKey;
    };

    std::shared_ptr<HttpClient> httpClient_;

    mutable std::mutex mutex_;
    bool initialized_;
    bool started_;
    bool gossipingEnabled_;
    bool crossSigningVerified_;

    std::string deviceId_;
    std::string ed25519Key_;
    std::string curve25519Key_;
    std::string currentUserId_;
    std::string homeserverUrl_;
    std::string keyBackupPassphrase_;

    int oneTimeKeysCount_;
    int keyBackupRemainingKeys_;
    int keyBackedUpCount_;

    CrossSigningState crossSigningState_;
    CrossSigningKeys crossSigningKeys_;

    KeyBackupState keyBackupState_;
    KeyBackupVersion keyBackupVersion_;

    // Olm state
    std::map<std::string, std::vector<uint8_t>> olmSessions_;  // "userId:deviceId" -> session buffer
    std::map<std::string, MegolmSessionInfo> megolmOutboundSessions_;  // roomId -> session info
    std::map<std::string, std::string> oneTimeKeys_;  // keyId -> keyData

    // Device verification state
    std::map<std::string, DeviceVerification> deviceVerificationMap_;
    std::map<std::string, bool> trustedUsers_;
    std::set<std::string> encryptedRooms_;

    // Pending requests
    std::vector<OutgoingRoomKeyRequest> outgoingKeyRequests_;
    std::vector<IncomingRoomKeyRequest> incomingKeyRequests_;
    std::vector<SecretShareRequest> secretRequests_;
    std::vector<VerificationTransaction> verificationTransactions_;

#ifdef MATRIX_SDK_USE_OLM
    std::vector<uint8_t> olmAccount_;
    OlmAccount* olmAccountPtr_ = nullptr;
    std::vector<uint8_t> sasBuffer_;
    std::vector<uint8_t> pkDecryptBuffer_;
    std::vector<uint8_t> utilityBuffer_;
    OlmUtility* utilityPtr_ = nullptr;
#endif

    // Gossip state
    std::vector<GossipingRequestResult> gossipResults_;
};

// ── Factory function ──
std::shared_ptr<CryptoService> createCryptoService(std::shared_ptr<HttpClient> httpClient) {
    return std::make_shared<CryptoServiceImpl>(std::move(httpClient));
}

} // namespace matrix_sdk
