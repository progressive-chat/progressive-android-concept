#ifndef MATRIX_SDK_CRYPTO_HPP
#define MATRIX_SDK_CRYPTO_HPP

#include "matrix_sdk_types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace matrix_sdk {

// Forward declarations
class Session;
class Room;

// ── MX Device List ──
struct MXDeviceList {
    std::string userId;
    std::string deviceId;
    std::vector<std::string> algorithms;
    std::map<std::string, std::string> keys;
    std::map<std::string, std::string> signatures;
    bool isUnsigned = false;
    bool isBlocked = false;
    bool isVerified = false;
};

// ── MX Users Device Map ──
struct MXUsersDevicesMap {
    std::map<std::string, std::map<std::string, MXDeviceList>> map;
};

// ── Key Backup Version ──
struct KeyBackupVersion {
    std::string version;
    std::string algorithm;
    std::string authData;
    int count = 0;
    std::string etag;
    bool isValid = false;
};

// ── Keys Backup Data ──
struct KeysBackupData {
    std::string version;
    std::map<std::string, std::map<std::string, std::string>> rooms;
    int totalKeys = 0;
    int backedUpKeys = 0;
};

// ── Cross Signing Key ──
struct CrossSigningKey {
    std::string userId;
    std::string publicKey;
    std::string usage;  // "master", "self_signing", "user_signing"
    std::map<std::string, std::string> signatures;
    bool isTrusted = false;
};

// ── Cross Signing Keys ──
struct CrossSigningKeys {
    std::string masterKey;
    std::string selfSignedKey;
    std::string userSignedKey;
};

// ── MXKeysExport ──
struct MXKeysExport {
    std::string userId;
    std::map<std::string, std::string> rooms;
    std::map<std::string, std::string> sessions;
    std::string passphrase;
    int version = 1;
};

// ── Import Room Keys Result ──
struct ImportRoomKeysResult {
    int total = 0;
    int imported = 0;
    int skipped = 0;
};

// ── Request ──
struct OutgoingRoomKeyRequest {
    std::string requestId;
    std::string roomId;
    std::string senderKey;
    std::string sessionId;
    std::string algorithm;
    std::vector<std::string> recipients;
};

struct IncomingRoomKeyRequest {
    std::string requestId;
    std::string userId;
    std::string deviceId;
    std::string roomId;
    std::string senderKey;
    std::string sessionId;
    std::string algorithm;
};

// ── Secret Share Request ──
struct SecretShareRequest {
    std::string requestId;
    std::string secretName;
    std::string requestingDeviceId;
    std::string requestBody;
};

// ── Gossip Request Result ──
struct GossipingRequestResult {
    std::string requestId;
    bool success = false;
    std::string error;
};

// ── Key Verification Transaction ──
struct VerificationTransaction {
    std::string transactionId;
    std::string otherUserId;
    std::string otherDeviceId;
    bool isIncoming = false;
    std::string state;
    int timestamp = 0;
};

// ── SAS Verification ──
struct SasVerification {
    std::string transactionId;
    std::string otherUserId;
    std::string otherDeviceId;
    std::string emojiCodes;    // 7 decimal emoji
    std::string decimalCodes;
    bool canVerify = false;
    std::string state;
};

// ── QR Code Verification ──
struct QrCodeVerification {
    std::string transactionId;
    std::string otherUserId;
    std::string otherDeviceId;
    std::string qrCodeData;
    bool canVerify = false;
    bool isScanned = false;
    std::string state;
};

// ── Verification State ──
enum class VerificationState {
    REQUEST,
    READY,
    STARTED,
    ACCEPTED,
    KEY_RECEIVED,
    KEY_SENT,
    MAC_RECEIVED,
    MAC_SENT,
    DONE,
    CANCELED,
    UNKNOWN
};

// ── Crypto Service ──
class CryptoService {
public:
    virtual ~CryptoService() = default;

    // ── Initialization ──
    virtual void initialize(std::function<void(bool, const std::string&)> callback) = 0;
    virtual bool isInitialized() const = 0;
    virtual bool isStarted() const = 0;

    // ── Device Management ──
    virtual std::string getDeviceCurve25519Key() const = 0;
    virtual std::string getDeviceEd25519Key() const = 0;
    virtual std::string getDeviceId() const = 0;
    virtual void getMyDevices(std::function<void(bool, const std::string&, const std::vector<DeviceInfo>&)> callback) = 0;

    // ── Device Verification ──
    virtual void setDeviceVerification(const std::string& userId, const std::string& deviceId,
        DeviceVerification status, SimpleCallback callback) = 0;
    virtual DeviceVerification getDeviceVerification(const std::string& userId, const std::string& deviceId) const = 0;
    virtual void getDevicesList(const std::string& userId,
        std::function<void(bool, const std::string&, const std::vector<CryptoDeviceInfo>&)> callback) = 0;
    virtual void downloadDeviceList(const std::string& userId, const std::string& token, SimpleCallback callback) = 0;
    virtual void fetchDevicesForUsers(const std::vector<std::string>& userIds, SimpleCallback callback) = 0;

    // ── Encryption ──
    virtual std::string encryptEvent(const std::string& roomId, const std::string& eventType,
        const std::string& contentJson) = 0;
    virtual std::string decryptEvent(const std::string& roomId, const std::string& eventJson) = 0;
    virtual bool isRoomEncrypted(const std::string& roomId) const = 0;
    virtual void ensureOlmSessionsForDevices(const MXUsersDevicesMap& devices, SimpleCallback callback) = 0;

    // ── One-Time Keys ──
    virtual void uploadOneTimeKeys(SimpleCallback callback) = 0;
    virtual int getOneTimeKeysCount() const = 0;
    virtual void generateOneTimeKeys(int count, SimpleCallback callback) = 0;
    virtual void claimOneTimeKeys(const MXUsersDevicesMap& devices, SimpleCallback callback) = 0;

    // ── Cross-Signing ──
    virtual CrossSigningState getCrossSigningState() const = 0;
    virtual void initializeCrossSigning(std::function<void(bool, const std::string&)> callback) = 0;
    virtual void bootstrapCrossSigning(const std::string& passphrase, SimpleCallback callback) = 0;
    virtual void resetCrossSigning(SimpleCallback callback) = 0;
    virtual void exportCrossSigningKeys(const std::string& passphrase,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual void importCrossSigningKeys(const std::string& keyData, const std::string& passphrase,
        SimpleCallback callback) = 0;
    virtual CrossSigningKeys getCrossSigningKeys() const = 0;
    virtual bool isCrossSigningVerified() const = 0;
    virtual bool checkUserTrust(const std::string& userId) const = 0;
    virtual bool checkDeviceTrust(const std::string& userId, const std::string& deviceId) const = 0;

    // ── Key Backup ──
    virtual KeyBackupState getKeyBackupState() const = 0;
    virtual void checkKeyBackup(SimpleCallback callback) = 0;
    virtual void enableKeyBackup(const std::string& passphrase, SimpleCallback callback) = 0;
    virtual void disableKeyBackup(SimpleCallback callback) = 0;
    virtual void deleteKeyBackup(const std::string& version, SimpleCallback callback) = 0;
    virtual void restoreKeyBackup(const std::string& passphrase, ProgressCallback progress,
        std::function<void(bool, const std::string&, int, int)> callback) = 0;
    virtual void forceUploadRoomKeys(SimpleCallback callback) = 0;
    virtual std::string getKeyBackupPassphrase() const = 0;
    virtual KeyBackupVersion getKeyBackupVersion() const = 0;
    virtual int getKeyBackupRemainingKeys() const = 0;
    virtual KeysBackupData getKeyBackupData() const = 0;

    // ── Key Export/Import ──
    virtual void exportRoomKeys(const std::string& passphrase,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
    virtual void importRoomKeys(const std::string& keyData, const std::string& passphrase,
        ProgressCallback progress, std::function<void(bool, const std::string&, const ImportRoomKeysResult&)> callback) = 0;

    // ── Room Key Requests ──
    virtual void requestRoomKey(const std::string& roomId, const std::string& senderKey,
        const std::string& sessionId, const std::string& algorithm, SimpleCallback callback) = 0;
    virtual void cancelRoomKeyRequest(const std::string& requestId, SimpleCallback callback) = 0;
    virtual void reRequestRoomKeys(SimpleCallback callback) = 0;
    virtual void shareRoomKey(const std::string& requestId, const std::string& userId,
        const std::string& deviceId, SimpleCallback callback) = 0;
    virtual void ignoreRoomKeyRequest(const std::string& requestId, SimpleCallback callback) = 0;
    virtual std::vector<OutgoingRoomKeyRequest> getOutgoingRoomKeyRequests() const = 0;
    virtual std::vector<IncomingRoomKeyRequest> getIncomingRoomKeyRequests() const = 0;

    // ── Secret Sharing ──
    virtual void requestSecret(const std::string& secretName, const std::string& deviceId, SimpleCallback callback) = 0;
    virtual void cancelSecretRequest(const std::string& requestId, SimpleCallback callback) = 0;
    virtual std::vector<SecretShareRequest> getSecretShareRequests() const = 0;

    // ── Verification ──
    virtual void beginSasVerification(const std::string& userId, const std::string& deviceId,
        std::function<void(bool, const std::string&, const SasVerification&)> callback) = 0;
    virtual void beginQrCodeVerification(const std::string& userId, const std::string& deviceId,
        std::function<void(bool, const std::string&, const QrCodeVerification&)> callback) = 0;
    virtual void confirmSasVerification(const std::string& transactionId, SimpleCallback callback) = 0;
    virtual void cancelSasVerification(const std::string& transactionId, SimpleCallback callback) = 0;
    virtual std::vector<VerificationTransaction> getVerificationTransactions() const = 0;
    virtual std::string getEmojiFromDecimal(int code) const = 0;

    // ── Event Handlers ──
    virtual void onSyncCompleted(const std::string& nextBatch) = 0;
    virtual void onLiveEvent(const std::string& roomId, const std::string& eventJson) = 0;

    // ── Utilities ──
    virtual void reset() = 0;
    virtual void close() = 0;
    virtual std::string getVersion() const = 0;
    virtual bool isGossipingEnabled() const = 0;
    virtual void setGossipingEnabled(bool enabled) = 0;
};

// ── Room Crypto Service ──
class RoomCryptoService {
public:
    virtual ~RoomCryptoService() = default;
    virtual bool isEncrypted() const = 0;
    virtual std::string encrypt(const std::string& eventType, const std::string& contentJson) = 0;
    virtual std::string decrypt(const std::string& eventJson) = 0;
    virtual void setEncryptionAlgorithm(const std::string& algorithm) = 0;
    virtual std::string getEncryptionAlgorithm() const = 0;
    virtual void rotateSession(SimpleCallback callback) = 0;
    virtual void discardSession(SimpleCallback callback) = 0;
    virtual void reshareKeys(const std::string& userId, const std::string& deviceId, SimpleCallback callback) = 0;
    virtual int getOutboundSessionCount() const = 0;
    virtual int getInboundSessionCount() const = 0;
};

} // namespace matrix_sdk

#endif
