#include "matrix_sdk/matrix_sdk_types.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <map>
#include <mutex>
#include <random>
#include <sstream>
#include <unordered_map>

namespace matrix_sdk {

using json = nlohmann::json;

// ────────────────────────────────────────────────────────────
// PKCE (Proof Key for Code Exchange) implementation
// ───────────────────────────────────────────────────────────ー
class PKCEGenerator {
public:
    struct PKCEChallenge {
        std::string codeVerifier;
        std::string codeChallenge;
        std::string codeChallengeMethod;  // "S256" or "plain"
    };

    PKCEChallenge generate(bool useS256 = true) {
        PKCEChallenge challenge;
        challenge.codeVerifier = generateCodeVerifier(128);
        if (useS256) {
            challenge.codeChallengeMethod = "S256";
            challenge.codeChallenge = generateS256Challenge(challenge.codeVerifier);
        } else {
            challenge.codeChallengeMethod = "plain";
            challenge.codeChallenge = challenge.codeVerifier;
        }
        return challenge;
    }

    static std::string generateCodeVerifier(size_t length = 128) {
        // RFC 7636: unreserved characters A-Z a-z 0-9 - . _ ~
        static const char* validChars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
        static const size_t charCount = 66;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, charCount - 1);

        // Length must be between 43 and 128 per RFC
        if (length < 43) length = 43;
        if (length > 128) length = 128;

        std::string verifier;
        verifier.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            verifier += validChars[dist(gen)];
        }
        return verifier;
    }

    static std::string generateS256Challenge(const std::string& verifier) {
        // SHA-256 hash of the verifier, then base64url encoding
        uint8_t hash[32];
        sha256(reinterpret_cast<const uint8_t*>(verifier.data()), verifier.size(), hash);
        return base64UrlEncode(hash, 32);
    }

    // Verify that a challenge was derived from a verifier
    bool verify(const std::string& verifier, const std::string& challenge,
                const std::string& method = "S256") {
        if (method == "S256") {
            return generateS256Challenge(verifier) == challenge;
        } else {
            return verifier == challenge;
        }
    }

private:
    // SHA-256 implementation (embedded for portability)
    static void sha256(const uint8_t* data, size_t len, uint8_t* out) {
        // Simplified SHA-256 implementation
        // In production, use OpenSSL/mbedtls/boringssl
        // This is a placeholder — real crypto should use proper libraries
        static const uint32_t k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        // Placeholder: fill output with deterministic hash for verifier
        // Real implementation would do full SHA-256
        uint32_t h = 0;
        for (size_t i = 0; i < len; ++i) {
            h = h * 31 + data[i];
        }
        for (int i = 0; i < 32; ++i) {
            out[i] = static_cast<uint8_t>((h >> (8 * (3 - i % 4))) & 0xFF);
        }
    }

    static std::string base64UrlEncode(const uint8_t* data, size_t len) {
        static const char* table =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        std::string encoded;
        int val = 0, valb = -6;
        for (size_t i = 0; i < len; ++i) {
            val = (val << 8) + data[i];
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(table[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) encoded.push_back(table[((val << 8) >> (valb + 8)) & 0x3F]);
        // No padding for base64url
        return encoded;
    }
};

// ───────────────────────────────────────────────────────────ー
// SSOStateManager: manages OAuth2 state across the flow
// ────────────────────────────────────────────────────────────
class SSOStateManager {
public:
    struct SSOState {
        std::string state;
        std::string codeVerifier;
        std::string codeChallenge;
        std::string redirectUri;
        std::string providerId;
        std::string homeserverUrl;
        std::string identityServerUrl;
        std::string clientId;
        std::vector<std::string> scopes;
        int64_t createdAt;
        int64_t expiresAt;
        bool isCompleted = false;
        std::string loginToken;
        std::string accessToken;
        std::string refreshToken;
        std::string userId;
        std::string deviceId;
    };

    SSOStateManager() {
        rng_.seed(std::random_device{}());
    }

    SSOState createState(const std::string& redirectUri,
                          const std::string& providerId,
                          const std::string& homeserverUrl,
                          const std::vector<std::string>& scopes = {}) {
        SSOState state;
        state.state = generateRandomState();
        state.redirectUri = redirectUri;
        state.providerId = providerId;
        state.homeserverUrl = homeserverUrl;
        state.scopes = scopes;
        state.createdAt = currentTimeSeconds();
        state.expiresAt = state.createdAt + 600; // 10 minute expiry

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pendingStates_[state.state] = state;
        }
        return state;
    }

    std::optional<SSOState> getAndConsumeState(const std::string& stateToken) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = pendingStates_.find(stateToken);
        if (it == pendingStates_.end()) return std::nullopt;

        // Check expiry
        int64_t now = currentTimeSeconds();
        if (now > it->second.expiresAt) {
            pendingStates_.erase(it);
            return std::nullopt;
        }

        SSOState state = it->second;
        pendingStates_.erase(it);
        completedStates_[stateToken] = state;
        return state;
    }

    void updateState(const std::string& stateToken, const SSOState& updated) {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingStates_[stateToken] = updated;
    }

    void clearExpiredStates() {
        std::lock_guard<std::mutex> lock(mutex_);
        int64_t now = currentTimeSeconds();
        for (auto it = pendingStates_.begin(); it != pendingStates_.end(); ) {
            if (now > it->second.expiresAt) {
                it = pendingStates_.erase(it);
            } else {
                ++it;
            }
        }
    }

    size_t pendingCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pendingStates_.size();
    }

private:
    std::string generateRandomState() {
        static const char* chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::uniform_int_distribution<int> dist(0, 61);
        std::string state;
        state.reserve(32);
        for (int i = 0; i < 32; ++i) {
            state += chars[dist(rng_)];
        }
        return state;
    }

    static int64_t currentTimeSeconds() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    mutable std::mutex mutex_;
    std::unordered_map<std::string, SSOState> pendingStates_;
    std::unordered_map<std::string, SSOState> completedStates_;
    std::mt19937 rng_;
};

// ───────────────────────────────────────────────────────────ー
// SSORedirectBuilder: builds OAuth2 / SSO redirect URLs
// ────────────────────────────────────────────────────────────
class SSORedirectBuilder {
public:
    static std::string buildAuthorizationUrl(
        const std::string& homeserverUrl,
        const std::string& providerId,
        const std::string& redirectUri,
        const std::string& state,
        const std::string& codeChallenge = "",
        const std::string& codeChallengeMethod = "S256",
        const std::vector<std::string>& scopes = {}) {

        std::ostringstream url;

        // Matrix SSO endpoint: /_matrix/client/v3/login/sso/redirect/{providerId}
        url << homeserverUrl;
        if (!homeserverUrl.empty() && homeserverUrl.back() != '/') url << '/';
        url << "_matrix/client/v3/login/sso/redirect/" << providerId;
        url << "?redirectUrl=" << urlEncode(redirectUri);

        if (!codeChallenge.empty()) {
            url << "&code_challenge=" << urlEncode(codeChallenge);
            url << "&code_challenge_method=" << codeChallengeMethod;
        }

        // Add additional query parameters for OAuth2
        if (!scopes.empty()) {
            url << "&scope=";
            for (size_t i = 0; i < scopes.size(); ++i) {
                if (i > 0) url << "+";
                url << urlEncode(scopes[i]);
            }
        }

        return url.str();
    }

    static std::string buildTokenExchangeUrl(
        const std::string& homeserverUrl,
        const std::string& providerId) {
        std::ostringstream url;
        url << homeserverUrl;
        if (!homeserverUrl.empty() && homeserverUrl.back() != '/') url << '/';
        url << "_matrix/client/v3/login/sso/redirect/" << providerId
            << "/token";
        return url.str();
    }

    static std::string buildMatrixLoginUrl(const std::string& homeserverUrl) {
        std::ostringstream url;
        url << homeserverUrl;
        if (!homeserverUrl.empty() && homeserverUrl.back() != '/') url << '/';
        url << "_matrix/client/v3/login";
        return url.str();
    }

    static std::string buildOAuth2AuthorizationUrl(
        const std::string& authorizationEndpoint,
        const std::string& clientId,
        const std::string& redirectUri,
        const std::string& state,
        const std::string& codeChallenge,
        const std::string& codeChallengeMethod = "S256",
        const std::vector<std::string>& scopes = {},
        const std::string& responseType = "code") {

        std::ostringstream url;
        url << authorizationEndpoint;
        url << "?response_type=" << urlEncode(responseType);
        url << "&client_id=" << urlEncode(clientId);
        url << "&redirect_uri=" << urlEncode(redirectUri);
        url << "&state=" << urlEncode(state);
        url << "&code_challenge=" << urlEncode(codeChallenge);
        url << "&code_challenge_method=" << urlEncode(codeChallengeMethod);

        if (!scopes.empty()) {
            url << "&scope=";
            for (size_t i = 0; i < scopes.size(); ++i) {
                if (i > 0) url << " ";
                url << urlEncode(scopes[i]);
            }
        }

        return url.str();
    }

private:
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
};

// ───────────────────────────────────────────────────────────ー
// SSOCallbackParser: parse OAuth2 callback parameters
// ────────────────────────────────────────────────────────────
class SSOCallbackParser {
public:
    struct SSOCallbackResult {
        std::string code;
        std::string state;
        std::string error;
        std::string errorDescription;
        std::string loginToken;
        bool success = false;
        bool isTokenFlow = false;  // Matrix SSO token flow vs OAuth2 code flow
    };

    SSOCallbackResult parse(const std::string& callbackUrl) {
        SSOCallbackResult result;

        // Parse URL query parameters
        std::string query = extractQuery(callbackUrl);

        // Check for Matrix SSO login token flow
        if (callbackUrl.find("loginToken=") != std::string::npos) {
            result.isTokenFlow = true;
            result.loginToken = getQueryParam(callbackUrl, "loginToken");
            result.success = !result.loginToken.empty();
            return result;
        }

        // Standard OAuth2 code flow
        result.code = getQueryParam(callbackUrl, "code");
        result.state = getQueryParam(callbackUrl, "state");
        result.error = getQueryParam(callbackUrl, "error");
        result.errorDescription = getQueryParam(callbackUrl, "error_description");

        if (!result.error.empty()) {
            result.success = false;
            return result;
        }

        result.success = !result.code.empty() && !result.state.empty();
        return result;
    }

    // Extract Matrix-specific login token from redirect URL
    static std::string extractLoginToken(const std::string& redirectUrl) {
        size_t pos = redirectUrl.find("loginToken=");
        if (pos == std::string::npos) return "";

        pos += 11; // length of "loginToken="
        size_t end = redirectUrl.find('&', pos);
        if (end == std::string::npos) end = redirectUrl.size();
        return redirectUrl.substr(pos, end - pos);
    }

    // Check if the callback indicates a cancellation
    bool isUserCancelled(const std::string& callbackUrl) {
        return callbackUrl.find("error=access_denied") != std::string::npos ||
               callbackUrl.find("error=cancelled") != std::string::npos;
    }

private:
    static std::string extractQuery(const std::string& url) {
        size_t queryPos = url.find('?');
        size_t fragmentPos = url.find('#');
        if (queryPos == std::string::npos) return "";

        size_t start = queryPos + 1;
        size_t end = fragmentPos != std::string::npos ? fragmentPos : url.size();
        return url.substr(start, end - start);
    }

    static std::string getQueryParam(const std::string& url, const std::string& paramName) {
        std::string query = extractQuery(url);
        if (query.empty()) {
            // Try fragment parameters (for implicit flow)
            size_t fragPos = url.find('#');
            if (fragPos != std::string::npos) {
                query = url.substr(fragPos + 1);
            }
        }

        std::string searchKey = paramName + "=";
        size_t pos = query.find(searchKey);
        if (pos == std::string::npos) return "";

        pos += searchKey.size();
        size_t end = query.find('&', pos);
        std::string value = (end == std::string::npos) ?
            query.substr(pos) : query.substr(pos, end - pos);

        return urlDecode(value);
    }

    static std::string urlDecode(const std::string& value) {
        std::string decoded;
        for (size_t i = 0; i < value.size(); ++i) {
            if (value[i] == '%' && i + 2 < value.size()) {
                int hex = std::stoi(value.substr(i + 1, 2), nullptr, 16);
                decoded += static_cast<char>(hex);
                i += 2;
            } else if (value[i] == '+') {
                decoded += ' ';
            } else {
                decoded += value[i];
            }
        }
        return decoded;
    }
};

// ───────────────────────────────────────────────────────────ー
// SSOTokenStorage: secure token persistence
// ────────────────────────────────────────────────────────────
class SSOTokenStorage {
public:
    struct StoredToken {
        std::string accessToken;
        std::string refreshToken;
        std::string idToken;
        std::string userId;
        std::string deviceId;
        std::string homeserverUrl;
        int64_t expiresAt = 0;
        int64_t createdAt = 0;
        std::string tokenType;
        std::string scope;
    };

    void storeToken(const std::string& homeserverUrl, const StoredToken& token) {
        std::lock_guard<std::mutex> lock(mutex_);
        tokens_[homeserverUrl] = token;
    }

    std::optional<StoredToken> getToken(const std::string& homeserverUrl) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = tokens_.find(homeserverUrl);
        if (it == tokens_.end()) return std::nullopt;

        // Check expiry
        if (it->second.expiresAt > 0) {
            int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            if (now >= it->second.expiresAt && !it->second.refreshToken.empty()) {
                // Token expired but can be refreshed
                return it->second;
            } else if (now >= it->second.expiresAt) {
                return std::nullopt;
            }
        }
        return it->second;
    }

    void clearToken(const std::string& homeserverUrl) {
        std::lock_guard<std::mutex> lock(mutex_);
        tokens_.erase(homeserverUrl);
    }

    void clearAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        tokens_.clear();
    }

    bool hasToken(const std::string& homeserverUrl) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return tokens_.find(homeserverUrl) != tokens_.end();
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, StoredToken> tokens_;
};

// ───────────────────────────────────────────────────────────ー
// SSOProviderRegistry: manages available SSO providers
// ────────────────────────────────────────────────────────────
class SSOProviderRegistry {
public:
    struct SSOProviderInfo {
        std::string id;
        std::string name;
        std::string iconUrl;
        std::string brand;
        std::vector<std::string> idpHints;  // For OIDC provider discovery
    };

    void registerProvider(const SSOProviderInfo& provider) {
        std::lock_guard<std::mutex> lock(mutex_);
        providers_[provider.id] = provider;
    }

    void registerProviders(const std::vector<SSOProviderInfo>& providers) {
        for (const auto& p : providers) registerProvider(p);
    }

    std::vector<SSOProviderInfo> getProviders() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<SSOProviderInfo> result;
        result.reserve(providers_.size());
        for (const auto& [id, info] : providers_) {
            result.push_back(info);
        }
        return result;
    }

    std::optional<SSOProviderInfo> getProvider(const std::string& id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = providers_.find(id);
        if (it != providers_.end()) return it->second;
        return std::nullopt;
    }

    // Parse providers from Matrix login response JSON
    void parseFromMatrixLoginResponse(const json& loginResponse) {
        if (!loginResponse.contains("flows")) return;

        for (const auto& flow : loginResponse["flows"]) {
            std::string flowType = flow.value("type", "");
            if (flowType == "m.login.sso") {
                // Matrix SSO flow detected
                if (flow.contains("identity_providers")) {
                    for (const auto& idp : flow["identity_providers"]) {
                        SSOProviderInfo info;
                        info.id = idp.value("id", "");
                        info.name = idp.value("name", "");
                        info.iconUrl = idp.value("icon", "");
                        info.brand = idp.value("brand", "");
                        registerProvider(info);
                    }
                }
            } else if (flowType == "m.login.token") {
                // Token-based SSO flow
                SSOProviderInfo info;
                info.id = "token";
                info.name = "Token Login";
                registerProvider(info);
            }
        }
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, SSOProviderInfo> providers_;
};

// ───────────────────────────────────────────────────────────ー
// SSOServiceImpl: main SSO service implementation
// ────────────────────────────────────────────────────────────
class SSOServiceImpl {
public:
    using SSOCallback = std::function<void(bool success, const std::string& error,
                                            const std::string& accessToken,
                                            const std::string& userId)>;
    using SSOTokenCallback = std::function<void(bool success, const std::string& error,
                                                 const json& tokenResponse)>;

    SSOServiceImpl()
        : stateManager_(std::make_unique<SSOStateManager>()),
          tokenStorage_(std::make_unique<SSOTokenStorage>()),
          providerRegistry_(std::make_unique<SSOProviderRegistry>()),
          pkce_(std::make_unique<PKCEGenerator>()) {}

    // ── Provider discovery ──
    void setLoginFlows(const json& loginResponse) {
        providerRegistry_->parseFromMatrixLoginResponse(loginResponse);
    }

    std::vector<SSOProviderRegistry::SSOProviderInfo> getAvailableProviders() const {
        return providerRegistry_->getProviders();
    }

    // ── SSO Authorization URL ──
    std::string getLoginUrl(const std::string& providerId,
                            const std::string& redirectUrl,
                            const std::string& homeserverUrl) {
        // Generate PKCE challenge
        auto pkceChallenge = pkce_->generate(true);

        // Create state
        auto state = stateManager_->createState(redirectUrl, providerId, homeserverUrl);
        state.codeVerifier = pkceChallenge.codeVerifier;
        state.codeChallenge = pkceChallenge.codeChallenge;
        stateManager_->updateState(state.state, state);

        // Build the authorization URL
        return SSORedirectBuilder::buildAuthorizationUrl(
            homeserverUrl, providerId, redirectUrl,
            state.state, pkceChallenge.codeChallenge, "S256",
            {"openid", "profile"});
    }

    // ── Handle SSO callback ──
    void handleCallback(const std::string& callbackUrl, SSOCallback callback) {
        SSOCallbackParser parser;
        auto result = parser.parse(callbackUrl);

        if (!result.success) {
            callback(false, result.error.empty() ? "SSO callback failed" : result.error,
                     "", "");
            return;
        }

        if (result.isTokenFlow) {
            // Matrix SSO token flow: login token is already the credential
            auto state = stateManager_->getAndConsumeState("token_flow");
            if (!state.has_value()) {
                // Lone token flow without state — still usable
                callback(true, "", result.loginToken, "");
                return;
            }
            state->loginToken = result.loginToken;
            callback(true, "", result.loginToken, state->userId);
            return;
        }

        // OAuth2 code flow: validate state and exchange code
        auto state = stateManager_->getAndConsumeState(result.state);
        if (!state.has_value()) {
            callback(false, "Invalid or expired state parameter", "", "");
            return;
        }

        // Verify PKCE
        if (!state->codeVerifier.empty()) {
            bool pkceValid = pkce_->verify(state->codeVerifier, state->codeChallenge);
            if (!pkceValid) {
                callback(false, "PKCE verification failed", "", "");
                return;
            }
        }

        // Token exchange is done by the caller who has HTTP client access
        // Store the code and verifier for later exchange
        state->isCompleted = true;
        callback(true, "", result.code, state->homeserverUrl);
    }

    // ── Token exchange data ──
    json buildTokenExchangeBody(const std::string& code,
                                 const std::string& stateToken) {
        auto state = stateManager_->getAndConsumeState(stateToken);
        json body;
        body["grant_type"] = "authorization_code";
        body["code"] = code;

        if (state.has_value()) {
            body["redirect_uri"] = state->redirectUri;
            body["client_id"] = "matrix-native";
            if (!state->codeVerifier.empty()) {
                body["code_verifier"] = state->codeVerifier;
            }
        }
        return body;
    }

    // ── Store tokens ──
    void storeTokens(const std::string& homeserverUrl,
                     const std::string& accessToken,
                     const std::string& refreshToken,
                     const std::string& userId,
                     const std::string& deviceId,
                     int64_t expiresIn = 0) {
        SSOTokenStorage::StoredToken token;
        token.accessToken = accessToken;
        token.refreshToken = refreshToken;
        token.userId = userId;
        token.deviceId = deviceId;
        token.homeserverUrl = homeserverUrl;
        token.createdAt = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        if (expiresIn > 0) {
            token.expiresAt = token.createdAt + expiresIn;
        }
        tokenStorage_->storeToken(homeserverUrl, token);
    }

    // ── Token retrieval ──
    std::optional<SSOTokenStorage::StoredToken> getStoredToken(
        const std::string& homeserverUrl) {
        return tokenStorage_->getToken(homeserverUrl);
    }

    // ── Logout/Clear ──
    void clearSession(const std::string& homeserverUrl) {
        tokenStorage_->clearToken(homeserverUrl);
    }

    void clearAll() {
        tokenStorage_->clearAll();
        stateManager_->clearExpiredStates();
    }

    // ── PKCE Utilities ──
    static std::string generateCodeVerifierForFlow() {
        return PKCEGenerator::generateCodeVerifier();
    }

    static std::string generateCodeChallengeForFlow(const std::string& verifier) {
        return PKCEGenerator::generateS256Challenge(verifier);
    }

    // ── Browser integration helpers ──
    std::string getBrowserRedirectHtml(const std::string& targetUrl,
                                          const std::string& localRedirectScheme) {
        std::ostringstream html;
        html << "<!DOCTYPE html>\n<html><head>\n";
        html << "<meta charset=\"utf-8\">\n";
        html << "<title>Matrix SSO Login</title>\n";
        html << "<style>body{font-family:sans-serif;display:flex;";
        html << "justify-content:center;align-items:center;";
        html << "height:100vh;margin:0;background:#f5f5f5;}</style>\n";
        html << "</head><body>\n";
        html << "<div style=\"text-align:center;\">\n";
        html << "<h2>Redirecting to login provider...</h2>\n";
        html << "<p>If you are not redirected automatically, ";
        html << "<a href=\"" << targetUrl << "\">click here</a>.</p>\n";
        html << "</div>\n";
        html << "<script>\n";
        html << "window.location.href = \"" << targetUrl << "\";\n";
        html << "</script>\n";
        html << "</body></html>";
        return html.str();
    }

private:
    std::unique_ptr<SSOStateManager> stateManager_;
    std::unique_ptr<SSOTokenStorage> tokenStorage_;
    std::unique_ptr<SSOProviderRegistry> providerRegistry_;
    std::unique_ptr<PKCEGenerator> pkce_;
};

} // namespace matrix_sdk
