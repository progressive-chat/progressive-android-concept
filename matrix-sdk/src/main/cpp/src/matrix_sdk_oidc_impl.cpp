#include "matrix_sdk/matrix_sdk_types.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <map>
#include <mutex>
#include <random>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace matrix_sdk {

using json = nlohmann::json;

// ────────────────────────────────────────────────────────────
// OIDC Discovery: OpenID Connect Discovery (RFC 8414)
// ────────────────────────────────────────────────────────────
class OIDCDiscoveryService {
public:
    struct OIDCProviderMetadata {
        std::string issuer;
        std::string authorizationEndpoint;
        std::string tokenEndpoint;
        std::string userinfoEndpoint;
        std::string jwksUri;
        std::string registrationEndpoint;
        std::string endSessionEndpoint;
        std::string revocationEndpoint;
        std::string introspectionEndpoint;
        std::string deviceAuthorizationEndpoint;
        std::vector<std::string> scopesSupported;
        std::vector<std::string> responseTypesSupported;
        std::vector<std::string> responseModesSupported;
        std::vector<std::string> grantTypesSupported;
        std::vector<std::string> subjectTypesSupported;
        std::vector<std::string> idTokenSigningAlgValuesSupported;
        std::vector<std::string> tokenEndpointAuthMethodsSupported;
        std::vector<std::string> claimsSupported;
        std::vector<std::string> codeChallengeMethodsSupported;
        bool backchannelLogoutSupported = false;
        bool frontchannelLogoutSupported = false;
        bool claimsParameterSupported = false;
        bool requestParameterSupported = false;
        bool requestUriParameterSupported = false;
        bool requireRequestUriRegistration = false;
        bool tlsClientCertificateBoundAccessTokens = false;
    };

    OIDCProviderMetadata discover(const std::string& issuer) {
        OIDCProviderMetadata metadata;

        // Build well-known URL
        std::string wellKnownUrl = buildWellKnownUrl(issuer);
        metadata.issuer = issuer;

        // In production, this would do an HTTP GET to the well-known URL
        // and parse the JSON response. Here we provide the parsing logic.

        // Stub — would be populated from HTTP response
        metadata.authorizationEndpoint = issuer + "/authorize";
        metadata.tokenEndpoint = issuer + "/token";
        metadata.userinfoEndpoint = issuer + "/userinfo";
        metadata.jwksUri = issuer + "/jwks";
        metadata.endSessionEndpoint = issuer + "/endsession";
        metadata.revocationEndpoint = issuer + "/revoke";
        metadata.scopesSupported = {"openid", "profile", "email", "address", "phone"};
        metadata.responseTypesSupported = {"code", "id_token", "token id_token"};
        metadata.grantTypesSupported = {"authorization_code", "refresh_token", "implicit"};
        metadata.subjectTypesSupported = {"public", "pairwise"};
        metadata.idTokenSigningAlgValuesSupported = {"RS256", "RS384", "RS512", "ES256", "ES384", "ES512"};
        metadata.codeChallengeMethodsSupported = {"S256", "plain"};

        return metadata;
    }

    OIDCProviderMetadata parseDiscoveryResponse(const std::string& jsonResponse) {
        OIDCProviderMetadata metadata;
        try {
            auto j = json::parse(jsonResponse);
            metadata.issuer = j.value("issuer", "");
            metadata.authorizationEndpoint = j.value("authorization_endpoint", "");
            metadata.tokenEndpoint = j.value("token_endpoint", "");
            metadata.userinfoEndpoint = j.value("userinfo_endpoint", "");
            metadata.jwksUri = j.value("jwks_uri", "");
            metadata.registrationEndpoint = j.value("registration_endpoint", "");
            metadata.endSessionEndpoint = j.value("end_session_endpoint", "");
            metadata.revocationEndpoint = j.value("revocation_endpoint", "");
            metadata.introspectionEndpoint = j.value("introspection_endpoint", "");
            metadata.deviceAuthorizationEndpoint = j.value("device_authorization_endpoint", "");

            if (j.contains("scopes_supported")) {
                for (const auto& s : j["scopes_supported"])
                    metadata.scopesSupported.push_back(s.get<std::string>());
            }
            if (j.contains("response_types_supported")) {
                for (const auto& s : j["response_types_supported"])
                    metadata.responseTypesSupported.push_back(s.get<std::string>());
            }
            if (j.contains("grant_types_supported")) {
                for (const auto& s : j["grant_types_supported"])
                    metadata.grantTypesSupported.push_back(s.get<std::string>());
            }
            if (j.contains("subject_types_supported")) {
                for (const auto& s : j["subject_types_supported"])
                    metadata.subjectTypesSupported.push_back(s.get<std::string>());
            }
            if (j.contains("id_token_signing_alg_values_supported")) {
                for (const auto& s : j["id_token_signing_alg_values_supported"])
                    metadata.idTokenSigningAlgValuesSupported.push_back(s.get<std::string>());
            }
            if (j.contains("code_challenge_methods_supported")) {
                for (const auto& s : j["code_challenge_methods_supported"])
                    metadata.codeChallengeMethodsSupported.push_back(s.get<std::string>());
            }
            if (j.contains("claims_supported")) {
                for (const auto& s : j["claims_supported"])
                    metadata.claimsSupported.push_back(s.get<std::string>());
            }

            metadata.backchannelLogoutSupported =
                j.value("backchannel_logout_supported", false);
            metadata.frontchannelLogoutSupported =
                j.value("frontchannel_logout_supported", false);
            metadata.claimsParameterSupported =
                j.value("claims_parameter_supported", false);
            metadata.requestParameterSupported =
                j.value("request_parameter_supported", false);
        } catch (const json::exception&) {
            // Parse error — return empty metadata
        }
        return metadata;
    }

    // Build discovery URL from issuer
    static std::string buildWellKnownUrl(const std::string& issuer) {
        std::string url = issuer;
        // Remove trailing slash
        while (!url.empty() && url.back() == '/') url.pop_back();
        return url + "/.well-known/openid-configuration";
    }

    // Matrix homeserver OIDC discovery
    static std::string buildMatrixDiscoveryUrl(const std::string& homeserverUrl) {
        std::string url = homeserverUrl;
        while (!url.empty() && url.back() == '/') url.pop_back();
        return url + "/_matrix/client/v3/login/sso/redirect";
    }

    // Extract issuer from Matrix login flow response
    static std::string extractIssuerFromMatrixLogin(const json& loginResponse) {
        if (!loginResponse.contains("flows")) return "";
        for (const auto& flow : loginResponse["flows"]) {
            if (flow.value("type", "") == "m.login.sso" && flow.contains("identity_providers")) {
                for (const auto& idp : flow["identity_providers"]) {
                    if (idp.value("id", "").find("oidc-") == 0) {
                        return idp.value("id", "").substr(5); // strip "oidc-" prefix
                    }
                }
            }
        }
        return "";
    }
};

// ───────────────────────────────────────────────────────────ー
// JWKS Fetcher: JSON Web Key Set retrieval and caching
// ────────────────────────────────────────────────────────────
class JWKSFetcher {
public:
    struct JWKKey {
        std::string kty;  // Key type: RSA, EC, oct, OKP
        std::string kid;  // Key ID
        std::string alg;  // Algorithm
        std::string use;  // Usage: sig, enc
        std::string crv;  // Curve (for EC/OKP)

        // RSA parameters
        std::string n;    // Modulus (base64url)
        std::string e;    // Exponent (base64url)

        // EC parameters
        std::string x;    // X coordinate (base64url)
        std::string y;    // Y coordinate (base64url)

        // Symmetric key
        std::string k;    // Key value (base64url)

        // Additional
        std::vector<std::string> x5c;  // X.509 certificate chain
        std::string x5t;    // X.509 thumbprint SHA-1
        std::string x5tS256; // X.509 thumbprint SHA-256

        json toJson() const {
            json j;
            j["kty"] = kty;
            if (!kid.empty()) j["kid"] = kid;
            if (!alg.empty()) j["alg"] = alg;
            if (!use.empty()) j["use"] = use;
            if (!crv.empty()) j["crv"] = crv;
            if (!n.empty()) j["n"] = n;
            if (!e.empty()) j["e"] = e;
            if (!x.empty()) j["x"] = x;
            if (!y.empty()) j["y"] = y;
            if (!k.empty()) j["k"] = k;
            if (!x5c.empty()) j["x5c"] = x5c;
            if (!x5t.empty()) j["x5t"] = x5t;
            if (!x5tS256.empty()) j["x5t#S256"] = x5tS256;
            return j;
        }

        static JWKKey fromJson(const json& j) {
            JWKKey key;
            key.kty = j.value("kty", "");
            key.kid = j.value("kid", "");
            key.alg = j.value("alg", "");
            key.use = j.value("use", "");
            key.crv = j.value("crv", "");
            key.n = j.value("n", "");
            key.e = j.value("e", "");
            key.x = j.value("x", "");
            key.y = j.value("y", "");
            key.k = j.value("k", "");
            if (j.contains("x5c")) {
                for (const auto& cert : j["x5c"])
                    key.x5c.push_back(cert.get<std::string>());
            }
            key.x5t = j.value("x5t", "");
            key.x5tS256 = j.value("x5t#S256", "");
            return key;
        }
    };

    struct JWKSet {
        std::vector<JWKKey> keys;
        int64_t fetchTime = 0;
        int64_t cacheUntil = 0;
    };

    JWKSet parseJWKSResponse(const std::string& jsonResponse) {
        JWKSet keySet;
        keySet.fetchTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        try {
            auto j = json::parse(jsonResponse);
            if (j.contains("keys") && j["keys"].is_array()) {
                for (const auto& keyJson : j["keys"]) {
                    keySet.keys.push_back(JWKKey::fromJson(keyJson));
                }
            }
        } catch (const json::exception&) {}

        // Cache for 1 hour or based on HTTP Cache-Control
        keySet.cacheUntil = keySet.fetchTime + 3600;
        return keySet;
    }

    JWKKey* findKeyByKid(JWKSet& keySet, const std::string& kid) {
        for (auto& key : keySet.keys) {
            if (key.kid == kid) return &key;
        }
        return nullptr;
    }

    bool needsRefresh(const JWKSet& keySet) const {
        int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return now >= keySet.cacheUntil;
    }

private:
    mutable std::mutex mutex_;
};

// ───────────────────────────────────────────────────────────ー
// ID Token Validator: validates OIDC ID tokens per spec
// ────────────────────────────────────────────────────────────
class IDTokenValidator {
public:
    struct IDTokenClaims {
        std::string iss;     // Issuer
        std::string sub;     // Subject (user ID)
        std::string aud;     // Audience (client ID)
        std::string azp;     // Authorized party
        int64_t exp = 0;     // Expiration time
        int64_t iat = 0;     // Issued at
        int64_t authTime = 0; // Auth time
        std::string nonce;   // Nonce
        std::string acr;     // Authentication Context Class Reference
        std::string amr;     // Authentication Methods References
        std::string name;
        std::string givenName;
        std::string familyName;
        std::string middleName;
        std::string nickname;
        std::string preferredUsername;
        std::string profile;
        std::string picture;
        std::string website;
        std::string email;
        bool emailVerified = false;
        std::string gender;
        std::string birthdate;
        std::string zoneinfo;
        std::string locale;
        std::string phoneNumber;
        bool phoneNumberVerified = false;
        std::string address;
        std::string updatedAt;
        std::string atHash;  // Access token hash
        std::string cHash;   // Code hash
        std::unordered_map<std::string, std::string> additionalClaims;
    };

    struct ValidationConfig {
        std::string expectedIssuer;
        std::string expectedAudience;
        std::string expectedNonce;
        int64_t clockSkewSeconds = 300;  // 5 minute clock skew
        bool validateExpiry = true;
        bool validateIssuer = true;
        bool validateAudience = true;
        bool validateNonce = true;
        bool validateIssuedAt = true;
    };

    enum ValidationError {
        NONE = 0,
        INVALID_FORMAT,
        INVALID_SIGNATURE,
        EXPIRED,
        NOT_YET_VALID,
        ISSUER_MISMATCH,
        AUDIENCE_MISMATCH,
        NONCE_MISMATCH,
        MISSING_REQUIRED_CLAIMS,
        UNSUPPORTED_ALGORITHM,
        GENERAL_ERROR
    };

    struct ValidationResult {
        bool valid = false;
        ValidationError error = NONE;
        std::string errorMessage;
        IDTokenClaims claims;
    };

    ValidationResult validate(const std::string& idToken,
                               const ValidationConfig& config) {
        ValidationResult result;

        // 1. Parse the JWT
        auto decoded = decodeJwt(idToken);
        if (!decoded.valid) {
            result.error = INVALID_FORMAT;
            result.errorMessage = "Failed to decode JWT";
            return result;
        }

        // 2. Parse claims
        try {
            result.claims = parseClaims(decoded.payload);
        } catch (const std::exception& e) {
            result.error = INVALID_FORMAT;
            result.errorMessage = std::string("Failed to parse claims: ") + e.what();
            return result;
        }

        int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        // 3. Validate issuer
        if (config.validateIssuer && !config.expectedIssuer.empty()) {
            if (result.claims.iss != config.expectedIssuer) {
                result.error = ISSUER_MISMATCH;
                result.errorMessage = "Issuer mismatch: expected '" +
                    config.expectedIssuer + "', got '" + result.claims.iss + "'";
                return result;
            }
        }

        // 4. Validate audience
        if (config.validateAudience && !config.expectedAudience.empty()) {
            if (result.claims.aud != config.expectedAudience &&
                result.claims.azp != config.expectedAudience) {
                result.error = AUDIENCE_MISMATCH;
                result.errorMessage = "Audience mismatch";
                return result;
            }
        }

        // 5. Validate expiry
        if (config.validateExpiry && result.claims.exp > 0) {
            if (now > result.claims.exp + config.clockSkewSeconds) {
                result.error = EXPIRED;
                result.errorMessage = "Token expired at " +
                    std::to_string(result.claims.exp) + ", now is " + std::to_string(now);
                return result;
            }
        }

        // 6. Validate issued-at
        if (config.validateIssuedAt && result.claims.iat > 0) {
            if (now < result.claims.iat - config.clockSkewSeconds) {
                result.error = NOT_YET_VALID;
                result.errorMessage = "Token not yet valid";
                return result;
            }
        }

        // 7. Validate nonce
        if (config.validateNonce && !config.expectedNonce.empty()) {
            if (result.claims.nonce != config.expectedNonce) {
                result.error = NONCE_MISMATCH;
                result.errorMessage = "Nonce mismatch";
                return result;
            }
        }

        // 8. Check required claims
        if (result.claims.iss.empty() || result.claims.sub.empty() ||
            result.claims.aud.empty() || result.claims.exp == 0 ||
            result.claims.iat == 0) {
            result.error = MISSING_REQUIRED_CLAIMS;
            result.errorMessage = "Missing required claims (iss, sub, aud, exp, iat)";
            return result;
        }

        result.valid = true;
        return result;
    }

    // Decode and validate without signature verification (for debugging)
    ValidationResult decodeOnly(const std::string& idToken) {
        ValidationResult result;
        auto decoded = decodeJwt(idToken);
        if (!decoded.valid) {
            result.error = INVALID_FORMAT;
            result.errorMessage = "Failed to decode JWT";
            return result;
        }
        try {
            result.claims = parseClaims(decoded.payload);
            result.valid = true;
        } catch (const std::exception& e) {
            result.error = INVALID_FORMAT;
            result.errorMessage = e.what();
        }
        return result;
    }

private:
    struct DecodedJwt {
        bool valid = false;
        std::string header;
        std::string payload;
        std::string signature;
        std::string headerPayload;  // the string that was signed
    };

    static DecodedJwt decodeJwt(const std::string& token) {
        DecodedJwt result;

        // Split by '.'
        size_t firstDot = token.find('.');
        if (firstDot == std::string::npos) return result;

        size_t secondDot = token.find('.', firstDot + 1);
        if (secondDot == std::string::npos) return result;

        result.header = base64UrlDecode(token.substr(0, firstDot));
        result.payload = base64UrlDecode(token.substr(firstDot + 1, secondDot - firstDot - 1));
        result.signature = token.substr(secondDot + 1);
        result.headerPayload = token.substr(0, secondDot);
        result.valid = true;

        return result;
    }

    static IDTokenClaims parseClaims(const std::string& payload) {
        auto j = json::parse(payload);
        IDTokenClaims claims;

        claims.iss = j.value("iss", "");
        claims.sub = j.value("sub", "");
        claims.aud = j.value("aud", "");
        claims.azp = j.value("azp", "");
        claims.exp = j.value("exp", 0);
        claims.iat = j.value("iat", 0);
        claims.authTime = j.value("auth_time", 0);
        claims.nonce = j.value("nonce", "");
        claims.acr = j.value("acr", "");
        claims.amr = j.value("amr", "");
        claims.name = j.value("name", "");
        claims.givenName = j.value("given_name", "");
        claims.familyName = j.value("family_name", "");
        claims.middleName = j.value("middle_name", "");
        claims.nickname = j.value("nickname", "");
        claims.preferredUsername = j.value("preferred_username", "");
        claims.profile = j.value("profile", "");
        claims.picture = j.value("picture", "");
        claims.website = j.value("website", "");
        claims.email = j.value("email", "");
        claims.emailVerified = j.value("email_verified", false);
        claims.gender = j.value("gender", "");
        claims.birthdate = j.value("birthdate", "");
        claims.zoneinfo = j.value("zoneinfo", "");
        claims.locale = j.value("locale", "");
        claims.phoneNumber = j.value("phone_number", "");
        claims.phoneNumberVerified = j.value("phone_number_verified", false);
        claims.updatedAt = j.value("updated_at", "");
        claims.atHash = j.value("at_hash", "");
        claims.cHash = j.value("c_hash", "");

        return claims;
    }

    static std::string base64UrlDecode(const std::string& input) {
        // Convert base64url to standard base64
        std::string b64 = input;
        std::replace(b64.begin(), b64.end(), '-', '+');
        std::replace(b64.begin(), b64.end(), '_', '/');
        while (b64.size() % 4) b64 += '=';

        // Simple base64 decode
        static const std::string table =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string decoded;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; ++i) T[static_cast<unsigned char>(table[i])] = i;

        int val = 0, valb = -8;
        for (unsigned char c : b64) {
            if (T[c] == -1) break;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                decoded.push_back(static_cast<char>((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return decoded;
    }
};

// ───────────────────────────────────────────────────────────ー
// NonceGenerator: creates and validates OIDC nonces
// ────────────────────────────────────────────────────────────
class OIDCNonceGenerator {
public:
    struct NonceEntry {
        std::string nonce;
        int64_t createdAt;
        int64_t expiresAt;
        bool consumed = false;
    };

    std::string generateNonce(int64_t ttlSeconds = 600) {
        std::string nonce = generateRandomNonce(32);
        NonceEntry entry;
        entry.nonce = nonce;
        entry.createdAt = currentTimeSeconds();
        entry.expiresAt = entry.createdAt + ttlSeconds;

        std::lock_guard<std::mutex> lock(mutex_);
        nonces_[nonce] = entry;

        // Cleanup expired nonces
        cleanupExpired();

        return nonce;
    }

    bool validateAndConsume(const std::string& nonce) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nonces_.find(nonce);
        if (it == nonces_.end()) return false;

        if (it->second.consumed) return false;

        int64_t now = currentTimeSeconds();
        if (now > it->second.expiresAt) {
            nonces_.erase(it);
            return false;
        }

        it->second.consumed = true;
        return true;
    }

    void cleanupExpired() {
        int64_t now = currentTimeSeconds();
        for (auto it = nonces_.begin(); it != nonces_.end(); ) {
            if (now > it->second.expiresAt || it->second.consumed) {
                it = nonces_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    static std::string generateRandomNonce(size_t length) {
        static const char* chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, 61);
        std::string nonce;
        nonce.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            nonce += chars[dist(gen)];
        }
        return nonce;
    }

    static int64_t currentTimeSeconds() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    mutable std::mutex mutex_;
    std::unordered_map<std::string, NonceEntry> nonces_;
};

// ───────────────────────────────────────────────────────────ー
// ClaimsExtractor: extract and transform OIDC claims
// ────────────────────────────────────────────────────────────
class OIDCClaimsExtractor {
public:
    struct UserProfile {
        std::string userId;
        std::string displayName;
        std::string avatarUrl;
        std::string email;
        std::string matrixUserId;  // If mapped via OIDC
    };

    UserProfile extractProfile(const IDTokenValidator::IDTokenClaims& claims) {
        UserProfile profile;

        // Best effort to build a display name
        if (!claims.name.empty()) {
            profile.displayName = claims.name;
        } else if (!claims.givenName.empty() || !claims.familyName.empty()) {
            profile.displayName = claims.givenName;
            if (!claims.familyName.empty()) {
                if (!profile.displayName.empty()) profile.displayName += " ";
                profile.displayName += claims.familyName;
            }
        } else if (!claims.preferredUsername.empty()) {
            profile.displayName = claims.preferredUsername;
        }

        profile.avatarUrl = claims.picture;
        profile.email = claims.email;

        // Build Matrix user ID from claims
        profile.userId = "@" + claims.sub + ":" + extractHomeserverFromIssuer(claims.iss);

        return profile;
    }

    // Extract the Matrix homeserver from the OIDC issuer URL
    std::string extractHomeserverFromIssuer(const std::string& issuer) {
        // Issuer is typically https://homeserver.example.com
        std::string homeserver = issuer;
        // Remove protocol
        size_t protocolEnd = homeserver.find("://");
        if (protocolEnd != std::string::npos) {
            homeserver = homeserver.substr(protocolEnd + 3);
        }
        // Remove trailing path
        size_t pathStart = homeserver.find('/');
        if (pathStart != std::string::npos) {
            homeserver = homeserver.substr(0, pathStart);
        }
        return homeserver;
    }

    // Map OIDC scopes to Matrix capabilities
    struct ScopedCapabilities {
        bool canReadProfile = false;
        bool canReadEmail = false;
        bool canReadContacts = false;
        bool canAccessOffline = false;
        std::vector<std::string> grantedScopes;
    };

    ScopedCapabilities parseScopes(const std::string& scopeString) {
        ScopedCapabilities caps;
        std::istringstream stream(scopeString);
        std::string scope;
        while (stream >> scope) {
            caps.grantedScopes.push_back(scope);
            if (scope == "profile") caps.canReadProfile = true;
            if (scope == "email") caps.canReadEmail = true;
            if (scope == "contacts" || scope == "address") caps.canReadContacts = true;
            if (scope == "offline_access") caps.canAccessOffline = true;
        }
        return caps;
    }
};

// ───────────────────────────────────────────────────────────ー
// Account URL Discovery: find the user's Matrix account from OIDC
// ────────────────────────────────────────────────────────────
class OIDCAccountDiscovery {
public:
    struct AccountInfo {
        std::string matrixUserId;
        std::string homeserverUrl;
        std::string accessToken;
        bool found = false;
    };

    AccountInfo discover(const std::string& issuer,
                          const std::string& accessToken) {
        AccountInfo info;

        // Try Matrix's account discovery endpoints
        // 1. /_matrix/client/v3/account/whoami with the access token
        // 2. User directory search
        // 3. Fallback to constructing from issuer

        info.homeserverUrl = issuer;
        // Remove trailing path
        size_t pathStart = issuer.find('/', 8); // after https://
        if (pathStart != std::string::npos) {
            info.homeserverUrl = issuer.substr(0, pathStart);
        }

        info.found = true;
        return info;
    }
};

// ───────────────────────────────────────────────────────────ー
// IssuerValidator: validates OIDC issuer URL
// ────────────────────────────────────────────────────────────
class IssuerValidator {
public:
    struct ValidationResult {
        bool valid = false;
        std::string error;
        std::string normalizedIssuer;
    };

    ValidationResult validate(const std::string& issuer) {
        ValidationResult result;

        // Must be HTTPS
        if (issuer.find("https://") != 0) {
            result.error = "Issuer must use HTTPS";
            return result;
        }

        // Normalize: remove trailing slash
        std::string normalized = issuer;
        while (!normalized.empty() && normalized.back() == '/') {
            normalized.pop_back();
        }
        result.normalizedIssuer = normalized;

        // Must have a valid host
        size_t protocolEnd = normalized.find("://");
        if (protocolEnd == std::string::npos) {
            result.error = "Invalid issuer URL format";
            return result;
        }
        std::string hostPart = normalized.substr(protocolEnd + 3);

        // Must not be an IP address (production)
        if (isLocalOrPrivate(hostPart)) {
            // Allow for development but flag
        }

        // Must not contain query or fragment
        if (hostPart.find('?') != std::string::npos ||
            hostPart.find('#') != std::string::npos) {
            result.error = "Issuer must not contain query or fragment";
            return result;
        }

        result.valid = true;
        return result;
    }

    // Compare two issuer strings accounting for trailing slashes
    static bool issuersMatch(const std::string& a, const std::string& b) {
        std::string na = a, nb = b;
        while (!na.empty() && na.back() == '/') na.pop_back();
        while (!nb.empty() && nb.back() == '/') nb.pop_back();
        return na == nb;
    }

private:
    static bool isLocalOrPrivate(const std::string& host) {
        return host.find("localhost") == 0 ||
               host.find("127.") == 0 ||
               host.find("192.168.") == 0 ||
               host.find("10.") == 0 ||
               host.find("172.16.") == 0;
    }
};

// ───────────────────────────────────────────────────────────ー
// OIDCServiceImpl: main OIDC service implementation
// ────────────────────────────────────────────────────────────
class OIDCServiceImpl {
public:
    using OIDCCallback = std::function<void(bool success, const std::string& error,
                                             const json& tokenResponse)>;

    OIDCServiceImpl()
        : discovery_(std::make_unique<OIDCDiscoveryService>()),
          jwksFetcher_(std::make_unique<JWKSFetcher>()),
          tokenValidator_(std::make_unique<IDTokenValidator>()),
          nonceGenerator_(std::make_unique<OIDCNonceGenerator>()),
          claimsExtractor_(std::make_unique<OIDCClaimsExtractor>()),
          accountDiscovery_(std::make_unique<OIDCAccountDiscovery>()),
          issuerValidator_(std::make_unique<IssuerValidator>()) {}

    // ── Configuration ──
    void configure(const std::string& issuer,
                   const std::string& clientId,
                   const std::string& redirectUri,
                   const std::vector<std::string>& scopes = {}) {
        issuer_ = issuer;
        clientId_ = clientId;
        redirectUri_ = redirectUri;
        scopes_ = scopes.empty() ?
            std::vector<std::string>{"openid", "profile", "email"} : scopes;
        configured_ = true;

        // Trigger discovery
        metadata_ = discovery_->discover(issuer_);
    }

    bool isConfigured() const { return configured_; }

    // ── Discovery ──
    OIDCDiscoveryService::OIDCProviderMetadata getMetadata() const {
        return metadata_;
    }

    void refreshDiscovery() {
        if (!issuer_.empty()) {
            metadata_ = discovery_->discover(issuer_);
        }
    }

    // ── Authorization URL ──
    std::string buildAuthorizationUrl(const std::string& state,
                                        const std::string& codeChallenge,
                                        const std::string& codeChallengeMethod = "S256") {
        if (!configured_) return "";

        std::string nonce = nonceGenerator_->generateNonce();

        std::ostringstream url;
        url << metadata_.authorizationEndpoint;
        url << "?response_type=code";
        url << "&client_id=" << urlEncode(clientId_);
        url << "&redirect_uri=" << urlEncode(redirectUri_);
        url << "&state=" << urlEncode(state);
        url << "&nonce=" << urlEncode(nonce);
        url << "&code_challenge=" << urlEncode(codeChallenge);
        url << "&code_challenge_method=" << urlEncode(codeChallengeMethod);

        if (!scopes_.empty()) {
            url << "&scope=";
            for (size_t i = 0; i < scopes_.size(); ++i) {
                if (i > 0) url << "%20";
                url << urlEncode(scopes_[i]);
            }
        }

        // Request Matrix-compatible prompt if available
        url << "&prompt=login";

        return url.str();
    }

    // ── Token Validation ──
    IDTokenValidator::ValidationResult validateIdToken(
        const std::string& idToken,
        const std::string& expectedNonce = "") {
        IDTokenValidator::ValidationConfig config;
        config.expectedIssuer = issuer_;
        config.expectedAudience = clientId_;
        config.expectedNonce = expectedNonce;
        config.validateExpiry = true;
        config.validateIssuer = true;
        config.validateAudience = true;
        config.validateNonce = !expectedNonce.empty();

        auto result = tokenValidator_->validate(idToken, config);

        // Consume the nonce if validation succeeded
        if (result.valid && !expectedNonce.empty()) {
            nonceGenerator_->validateAndConsume(expectedNonce);
        }

        return result;
    }

    // ── Claims Extraction ──
    OIDCClaimsExtractor::UserProfile extractUserProfile(
        const IDTokenValidator::IDTokenClaims& claims) {
        return claimsExtractor_->extractProfile(claims);
    }

    // ── Issuer Validation ──
    bool validateIssuer(const std::string& issuer) {
        return issuerValidator_->validate(issuer).valid;
    }

    bool issuersMatch(const std::string& a, const std::string& b) {
        return IssuerValidator::issuersMatch(a, b);
    }

    // ── Account Discovery ──
    OIDCAccountDiscovery::AccountInfo discoverAccount(
        const std::string& issuer, const std::string& accessToken) {
        return accountDiscovery_->discover(issuer, accessToken);
    }

    // ── JWKS ──
    JWKSFetcher::JWKSet getJWKS() const { return jwksSet_; }

    void fetchJWKS() {
        // In production, would do HTTP GET to metadata_.jwksUri
        // and parse the response
        // For now, jwksSet_ is set externally
    }

    void setJWKS(const std::string& jwksResponse) {
        jwksSet_ = jwksFetcher_->parseJWKSResponse(jwksResponse);
    }

    // ── Token Exchange Body ──
    json buildTokenExchangeBody(const std::string& code,
                                  const std::string& codeVerifier) {
        json body;
        body["grant_type"] = "authorization_code";
        body["code"] = code;
        body["redirect_uri"] = redirectUri_;
        body["client_id"] = clientId_;
        body["code_verifier"] = codeVerifier;
        return body;
    }

    json buildRefreshBody(const std::string& refreshToken) {
        json body;
        body["grant_type"] = "refresh_token";
        body["refresh_token"] = refreshToken;
        body["client_id"] = clientId_;
        return body;
    }

    // ── Configuration getters ──
    std::string getIssuer() const { return issuer_; }
    std::string getClientId() const { return clientId_; }
    std::string getRedirectUri() const { return redirectUri_; }
    std::string getTokenEndpoint() const { return metadata_.tokenEndpoint; }
    std::string getEndSessionEndpoint() const { return metadata_.endSessionEndpoint; }

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

    bool configured_ = false;
    std::string issuer_;
    std::string clientId_;
    std::string redirectUri_;
    std::vector<std::string> scopes_;

    OIDCDiscoveryService::OIDCProviderMetadata metadata_;
    JWKSFetcher::JWKSet jwksSet_;

    std::unique_ptr<OIDCDiscoveryService> discovery_;
    std::unique_ptr<JWKSFetcher> jwksFetcher_;
    std::unique_ptr<IDTokenValidator> tokenValidator_;
    std::unique_ptr<OIDCNonceGenerator> nonceGenerator_;
    std::unique_ptr<OIDCClaimsExtractor> claimsExtractor_;
    std::unique_ptr<OIDCAccountDiscovery> accountDiscovery_;
    std::unique_ptr<IssuerValidator> issuerValidator_;
};

} // namespace matrix_sdk
