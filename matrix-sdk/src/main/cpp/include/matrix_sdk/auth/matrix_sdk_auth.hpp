#ifndef MATRIX_SDK_AUTH_HPP
#define MATRIX_SDK_AUTH_HPP

#include "matrix_sdk_types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace matrix_sdk {

// ── Login Wizard ──
struct LoginWizardData {
    std::string homeServerUrl;
    std::string identityServerUrl;
    bool isOidc = false;
    std::vector<LoginFlow> supportedFlows;
};

// ── Registration Wizard ──
struct RegistrationWizardData {
    std::string homeServerUrl;
    std::string identityServerUrl;
    std::vector<RegistrationFlow> supportedFlows;
    bool registrationEnabled = true;
    std::string sessionId;
};

// ── Login Descriptor ──
struct LoginDescriptor {
    std::string type;           // "password", "token", "sso", "oidc"
    std::string identifier;     // user or third party ID
    std::string password;
    std::string token;
    std::string deviceId;
    std::string initialDeviceDisplayName;
};

// ── Registration Descriptor ──
struct RegistrationDescriptor {
    std::string username;
    std::string password;
    std::string authType;       // "m.login.dummy", "m.login.recaptcha", etc.
    std::string authSessionId;
    std::string authParams;
    std::string initialDeviceDisplayName;
    bool inhibitLogin = false;
};

// ── Authentication Service ──
class AuthenticationService {
public:
    using WizardCallback = std::function<void(bool success, const std::string& error, const LoginWizardData& data)>;
    using RegistrationCallback = std::function<void(bool success, const std::string& error, const RegistrationWizardData& data)>;
    using LoginCallback = std::function<void(bool success, const std::string& error, const SessionParams& params)>;
    using DirectLoginCallback = std::function<void(bool success, const std::string& error, const SessionParams& params)>;
    using RegistrationResultCallback = std::function<void(bool success, const std::string& error, const SessionParams& params)>;

    virtual ~AuthenticationService() = default;

    // Get the login flows supported by the homeserver
    virtual void getLoginWizard(const std::string& homeServerUrl, WizardCallback callback) = 0;

    // Get registration flows
    virtual void getRegistrationWizard(const std::string& homeServerUrl, RegistrationCallback callback) = 0;

    // Perform login with descriptor
    virtual void login(const LoginDescriptor& descriptor, LoginCallback callback) = 0;

    // Direct login (username + password + homeserver)
    virtual void directLogin(const std::string& userId, const std::string& password,
        const std::string& homeServerUrl, const std::string& initialDeviceName,
        DirectLoginCallback callback) = 0;

    // Login with SSO token
    virtual void loginWithToken(const std::string& loginToken, const std::string& homeServerUrl,
        const std::string& initialDeviceName, LoginCallback callback) = 0;

    // Register a new account
    virtual void registerAccount(const RegistrationDescriptor& descriptor,
        RegistrationResultCallback callback) = 0;

    // Get well-known config for a domain
    virtual void getWellKnown(const std::string& domain,
        std::function<void(bool, const std::string&, const WellKnownConfig&)> callback) = 0;

    // Check if SSO redirect URL is supported
    virtual bool isSsoUrl(const std::string& url) = 0;

    // Cancel any pending authentication
    virtual void cancel() = 0;

    // Get the homeserver version
    virtual void getHomeServerVersion(const std::string& homeServerUrl,
        std::function<void(bool, const std::string&, const std::string&)> callback) = 0;
};

} // namespace matrix_sdk

#endif
