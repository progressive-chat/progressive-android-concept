#include "matrix_sdk/auth/matrix_sdk_auth.hpp"
#include "matrix_sdk/matrix_sdk_types.hpp"
#include "matrix_sdk/network/matrix_sdk_network.hpp"
#include <nlohmann/json.hpp>
#include <sstream>
#include <algorithm>
#include <regex>

namespace matrix_sdk {

using json = nlohmann::json;

// ── AuthenticationServiceImpl ──
class AuthenticationServiceImpl : public AuthenticationService {
public:
    explicit AuthenticationServiceImpl(std::shared_ptr<HttpClient> http)
        : httpClient_(std::move(http)) {}

    void getLoginWizard(const std::string& homeServerUrl, WizardCallback callback) override {
        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = homeServerUrl + "/_matrix/client/r0/login",
            .timeoutMs = 15000
        };

        httpClient_->execute(request, [callback](const HttpResponse& response) {
            if (!response.isSuccess()) {
                LoginWizardData empty;
                callback(false, "HTTP " + std::to_string(response.httpCode), empty);
                return;
            }
            try {
                auto j = json::parse(response.body);
                LoginWizardData wizard;
                wizard.homeServerUrl = j.value("homeserver", "");

                if (j.contains("flows")) {
                    for (const auto& flow : j["flows"]) {
                        LoginFlow lf;
                        lf.type = flow.value("type", "");
                        if (flow.contains("identity_providers")) {
                            for (const auto& idp : flow["identity_providers"]) {
                                lf.identityProviders.push_back(idp.value("id", ""));
                            }
                            lf.isSso = true;
                        }
                        wizard.supportedFlows.push_back(lf);
                    }
                }
                callback(true, "", wizard);
            } catch (const std::exception& e) {
                LoginWizardData empty;
                callback(false, std::string("Parse error: ") + e.what(), empty);
            }
        });
    }

    void getRegistrationWizard(const std::string& homeServerUrl, RegistrationCallback callback) override {
        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = homeServerUrl + "/_matrix/client/r0/register",
            .timeoutMs = 15000
        };

        httpClient_->execute(request, [callback](const HttpResponse& response) {
            if (!response.isSuccess()) {
                RegistrationWizardData empty;
                callback(false, "HTTP " + std::to_string(response.httpCode), empty);
                return;
            }
            try {
                auto j = json::parse(response.body);
                RegistrationWizardData wizard;
                wizard.homeServerUrl = j.value("homeserver", "");

                if (j.contains("flows")) {
                    for (const auto& flow : j["flows"]) {
                        RegistrationFlow rf;
                        rf.type = flow.value("type", "");
                        if (flow.contains("stages")) {
                            for (const auto& stage : flow["stages"]) {
                                rf.stages.push_back(stage.get<std::string>());
                            }
                        }
                        wizard.supportedFlows.push_back(rf);
                    }
                }
                callback(true, "", wizard);
            } catch (const std::exception& e) {
                RegistrationWizardData empty;
                callback(false, std::string("Parse error: ") + e.what(), empty);
            }
        });
    }

    void login(const LoginDescriptor& descriptor, LoginCallback callback) override {
        json body;
        body["type"] = descriptor.type;
        body["identifier"] = json::object();
        body["identifier"]["type"] = "m.id.user";
        body["identifier"]["user"] = descriptor.identifier;
        if (!descriptor.password.empty()) body["password"] = descriptor.password;
        if (!descriptor.token.empty()) body["token"] = descriptor.token;
        if (!descriptor.deviceId.empty()) body["device_id"] = descriptor.deviceId;
        if (!descriptor.initialDeviceDisplayName.empty())
            body["initial_device_display_name"] = descriptor.initialDeviceDisplayName;

        performLogin(body, descriptor.identifier, callback);
    }

    void directLogin(const std::string& userId, const std::string& password,
        const std::string& homeServerUrl, const std::string& initialDeviceName,
        DirectLoginCallback callback) override {
        auto uid = UserId::parse(userId);
        if (!uid.isValid()) {
            SessionParams empty;
            callback(false, "Invalid user ID", empty);
            return;
        }
        std::string baseUrl = homeServerUrl;
        if (baseUrl.empty()) baseUrl = "https://" + uid.server_name;

        json body;
        body["type"] = "m.login.password";
        body["identifier"] = json::object();
        body["identifier"]["type"] = "m.id.user";
        body["identifier"]["user"] = userId;
        body["password"] = password;
        if (!initialDeviceName.empty()) body["initial_device_display_name"] = initialDeviceName;

        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = baseUrl + "/_matrix/client/r0/login",
            .body = body.dump(),
            .timeoutMs = 30000
        };

        httpClient_->execute(request, [callback, userId](const HttpResponse& response) {
            if (!response.isSuccess()) {
                SessionParams empty;
                callback(false, "Login failed: HTTP " + std::to_string(response.httpCode) +
                    " - " + response.errorMessage, empty);
                return;
            }
            try {
                auto j = json::parse(response.body);
                SessionParams params;
                params.userId = j.value("user_id", userId);
                params.accessToken = j.value("access_token", "");
                params.deviceId = j.value("device_id", "");
                params.homeServerUrl = j.value("home_server", "");
                params.refreshToken = j.value("refresh_token", "");
                params.isTokenValid = true;
                callback(true, "", params);
            } catch (const std::exception& e) {
                SessionParams empty;
                callback(false, std::string("Parse error: ") + e.what(), empty);
            }
        });
    }

    void loginWithToken(const std::string& loginToken, const std::string& homeServerUrl,
        const std::string& initialDeviceName, LoginCallback callback) override {
        json body;
        body["type"] = "m.login.token";
        body["token"] = loginToken;
        if (!initialDeviceName.empty()) body["initial_device_display_name"] = initialDeviceName;

        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = homeServerUrl + "/_matrix/client/r0/login",
            .body = body.dump(),
            .timeoutMs = 30000
        };

        httpClient_->execute(request, [callback](const HttpResponse& response) {
            if (!response.isSuccess()) {
                SessionParams empty;
                callback(false, "Token login failed: HTTP " + std::to_string(response.httpCode), empty);
                return;
            }
            try {
                auto j = json::parse(response.body);
                SessionParams params;
                params.userId = j.value("user_id", "");
                params.accessToken = j.value("access_token", "");
                params.deviceId = j.value("device_id", "");
                params.homeServerUrl = j.value("home_server", "");
                params.isTokenValid = true;
                callback(true, "", params);
            } catch (const std::exception& e) {
                SessionParams empty;
                callback(false, std::string("Parse error: ") + e.what(), empty);
            }
        });
    }

    void registerAccount(const RegistrationDescriptor& descriptor,
        RegistrationResultCallback callback) override {
        json body;
        body["username"] = descriptor.username;
        body["password"] = descriptor.password;
        if (!descriptor.authType.empty()) {
            body["auth"] = json::object();
            body["auth"]["type"] = descriptor.authType;
            if (!descriptor.authSessionId.empty())
                body["auth"]["session"] = descriptor.authSessionId;
            if (!descriptor.authParams.empty())
                body["auth"]["params"] = json::parse(descriptor.authParams);
        }
        if (!descriptor.initialDeviceDisplayName.empty())
            body["initial_device_display_name"] = descriptor.initialDeviceDisplayName;
        if (descriptor.inhibitLogin) body["inhibit_login"] = true;

        performLogin(body, "@" + descriptor.username, callback);
    }

    void getWellKnown(const std::string& domain,
        std::function<void(bool, const std::string&, const WellKnownConfig&)> callback) override {
        std::string url = "https://" + domain + "/.well-known/matrix/client";

        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = url,
            .timeoutMs = 10000
        };

        httpClient_->execute(request, [callback](const HttpResponse& response) {
            WellKnownConfig config;
            if (!response.isSuccess()) {
                callback(false, "Well-known not available for this domain", config);
                return;
            }
            try {
                auto j = json::parse(response.body);
                if (j.contains("m.homeserver")) {
                    config.homeServerBaseUrl = j["m.homeserver"].value("base_url", "");
                }
                if (j.contains("m.identity_server")) {
                    config.identityServerBaseUrl = j["m.identity_server"].value("base_url", "");
                }
                config.isValid = !config.homeServerBaseUrl.empty();
                callback(true, "", config);
            } catch (const std::exception& e) {
                callback(false, std::string("Parse error: ") + e.what(), config);
            }
        });
    }

    bool isSsoUrl(const std::string& url) override {
        return url.find("/_matrix/client/r0/login/sso/redirect") != std::string::npos;
    }

    void cancel() override {
        httpClient_->cancelAll();
    }

    void getHomeServerVersion(const std::string& homeServerUrl,
        std::function<void(bool, const std::string&, const std::string&)> callback) override {
        auto request = HttpRequest{
            .method = HttpMethod::GET,
            .url = homeServerUrl + "/_matrix/client/versions",
            .timeoutMs = 10000
        };

        httpClient_->execute(request, [callback](const HttpResponse& response) {
            if (!response.isSuccess()) {
                callback(false, "Failed to get versions", "");
                return;
            }
            try {
                auto j = json::parse(response.body);
                std::string versions;
                if (j.contains("versions")) {
                    for (size_t i = 0; i < j["versions"].size(); ++i) {
                        if (i > 0) versions += ", ";
                        versions += j["versions"][i].get<std::string>();
                    }
                }
                callback(true, "", versions);
            } catch (const std::exception& e) {
                callback(false, std::string("Parse error: ") + e.what(), "");
            }
        });
    }

private:
    void performLogin(const json& body, const std::string& userId, LoginCallback callback) {
        auto uid = UserId::parse(userId);
        std::string baseUrl = "https://" + uid.server_name;

        auto request = HttpRequest{
            .method = HttpMethod::POST,
            .url = baseUrl + "/_matrix/client/r0/login",
            .body = body.dump(),
            .timeoutMs = 30000,
            .canonicalUrl = "/_matrix/client/r0/login"
        };

        httpClient_->execute(request, [callback, userId](const HttpResponse& response) {
            if (!response.isSuccess()) {
                SessionParams empty;
                std::string err = "Auth failed: HTTP " + std::to_string(response.httpCode);
                try {
                    auto j = json::parse(response.body);
                    if (j.contains("error")) err += " - " + j["error"].get<std::string>();
                } catch (...) {}
                callback(false, err, empty);
                return;
            }
            try {
                auto j = json::parse(response.body);
                SessionParams params;
                params.userId = j.value("user_id", userId);
                params.accessToken = j.value("access_token", "");
                params.deviceId = j.value("device_id", "");
                params.homeServerUrl = j.value("home_server", "");
                params.refreshToken = j.value("refresh_token", "");
                params.expiresInMs = j.value("expires_in_ms", 0);
                params.isTokenValid = true;
                callback(true, "", params);
            } catch (const std::exception& e) {
                SessionParams empty;
                callback(false, std::string("Parse error: ") + e.what(), empty);
            }
        });
    }

    std::shared_ptr<HttpClient> httpClient_;
};

// Factory function
std::shared_ptr<AuthenticationService> createAuthenticationService(
    std::shared_ptr<HttpClient> httpClient) {
    return std::make_shared<AuthenticationServiceImpl>(std::move(httpClient));
}

} // namespace matrix_sdk
