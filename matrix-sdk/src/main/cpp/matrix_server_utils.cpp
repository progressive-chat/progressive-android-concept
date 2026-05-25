#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include <android/log.h>

#define LOG_TAG "MatrixApiSrv"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {
using json = nlohmann::json;

// ==== Server Capability Detection ====

class ServerCapabilities {
public:
    struct Capabilities {
        bool supportsThreads = false;
        bool supportsSpaces = false;
        bool supportsKnock = false;
        bool supportsRestrictedRooms = false;
        bool supportsPoll = false;
        bool supportsLocationSharing = false;
        bool supportsVoiceBroadcasts = false;
        bool supportsDehydration = false;
        bool supportsOIDC = false;
        bool supportsSSO = false;
        bool supportsPasswordLogin = true;
        bool supportsTokenLogin = true;
        bool supportsAppserviceLogin = false;
        int maxUploadSize = 104857600;
        std::vector<std::string> roomVersions;
        std::string defaultRoomVersion = "10";
        std::map<std::string, bool> unstableFeatures;
    };

    static Capabilities detect(const json& serverResponse) {
        Capabilities cap;
        if (serverResponse.contains("capabilities")) {
            auto& caps = serverResponse["capabilities"];
            if (caps.contains("m.thread")) cap.supportsThreads = caps["m.thread"].value("enabled", false);
            if (caps.contains("m.space")) cap.supportsSpaces = caps["m.space"].value("enabled", false);
            if (caps.contains("m.knock")) cap.supportsKnock = caps["m.knock"].value("enabled", false);
            if (caps.contains("m.restricted")) cap.supportsRestrictedRooms = caps["m.restricted"].value("enabled", false);
            if (caps.contains("m.poll")) cap.supportsPoll = caps["m.poll"].value("enabled", false);
            if (caps.contains("m.location")) cap.supportsLocationSharing = caps["m.location"].value("enabled", false);
            if (caps.contains("m.voice_broadcast")) cap.supportsVoiceBroadcasts = caps["m.voice_broadcast"].value("enabled", false);
            if (caps.contains("m.dehydrated_device")) cap.supportsDehydration = caps["m.dehydrated_device"].value("enabled", false);
            if (caps.contains("m.upload_size")) cap.maxUploadSize = caps["m.upload_size"];
            if (caps.contains("m.room_versions")) {
                auto& versions = caps["m.room_versions"];
                cap.defaultRoomVersion = versions.value("default", "10");
                if (versions.contains("available")) {
                    for (auto& v : versions["available"]) {
                        cap.roomVersions.push_back(v);
                    }
                }
            }
        }
        if (serverResponse.contains("flows")) {
            cap.supportsPasswordLogin = false;
            cap.supportsSSO = false;
            for (auto& flow : serverResponse["flows"]) {
                std::string type = flow.value("type", "");
                if (type == "m.login.password") cap.supportsPasswordLogin = true;
                if (type == "m.login.sso") cap.supportsSSO = true;
                if (type == "m.login.token") cap.supportsTokenLogin = true;
            }
        }
        return cap;
    }
};

// ==== Server Version Checker ====

class ServerVersionChecker {
public:
    struct VersionInfo {
        std::vector<std::string> supportedVersions;
        bool supportsV1_1 = false;
        bool supportsV1_2 = false;
        bool supportsV1_3 = false;
        bool supportsV1_4 = false;
        bool supportsV1_5 = false;
        bool supportsV1_6 = false;
    };

    static VersionInfo parse(const json& response) {
        VersionInfo vi;
        if (response.contains("versions")) {
            for (auto& v : response["versions"]) {
                std::string ver = v;
                vi.supportedVersions.push_back(ver);
                if (ver == "v1.1") vi.supportsV1_1 = true;
                if (ver == "v1.2") vi.supportsV1_2 = true;
                if (ver == "v1.3") vi.supportsV1_3 = true;
                if (ver == "v1.4") vi.supportsV1_4 = true;
                if (ver == "v1.5") vi.supportsV1_5 = true;
                if (ver == "v1.6") vi.supportsV1_6 = true;
            }
        }
        return vi;
    }

    static std::string bestVersion(const VersionInfo& vi) {
        if (vi.supportsV1_6) return "v1.6";
        if (vi.supportsV1_5) return "v1.5";
        if (vi.supportsV1_4) return "v1.4";
        if (vi.supportsV1_3) return "v1.3";
        if (vi.supportsV1_2) return "v1.2";
        if (vi.supportsV1_1) return "v1.1";
        if (!vi.supportedVersions.empty()) return vi.supportedVersions[0];
        return "r0.6.0";
    }
};

// ==== Room Version Compatibility ====

class RoomVersionCompat {
public:
    struct RoomVersion {
        std::string id;
        int status = 0; // 0=stable, 1=unstable, 2=deprecated
        std::string description;
    };

    static std::vector<RoomVersion> allKnown() {
        return {
            {"1", 0, "Original room version"},
            {"2", 0, "State resolution v2"},
            {"3", 0, "Event IDs as hashes"},
            {"4", 0, "Event IDs as hashes v2"},
            {"5", 0, "Enforced signing key validity"},
            {"6", 0, "Redaction rules updated"},
            {"7", 0, "Knock and restricted rooms"},
            {"8", 0, "Restricted join rules v2"},
            {"9", 0, "Redaction rules updated v2"},
            {"10", 0, "Enforced integer power levels"},
            {"11", 0, "Event relationships in redactions"},
        };
    }

    static bool isStable(const std::string& version) {
        for (auto& v : allKnown()) {
            if (v.id == version) return v.status == 0;
        }
        return false;
    }

    static std::string latestStable() { return "11"; }
    static std::string latestUnstable() { return "org.matrix.msc.3827"; }
};

// ==== Login Flow Handler ====

class LoginFlowHandler {
public:
    struct LoginFlow {
        std::string type;
        std::vector<std::string> stages;
        json params;
    };

    static std::vector<LoginFlow> parseFlows(const json& loginResponse) {
        std::vector<LoginFlow> flows;
        if (loginResponse.contains("flows")) {
            for (auto& flow : loginResponse["flows"]) {
                LoginFlow lf;
                lf.type = flow.value("type", "");
                if (flow.contains("stages")) {
                    for (auto& stage : flow["stages"]) {
                        lf.stages.push_back(stage);
                    }
                }
                if (flow.contains("params")) {
                    lf.params = flow["params"];
                }
                flows.push_back(lf);
            }
        }
        return flows;
    }

    static bool hasFlow(const std::vector<LoginFlow>& flows, const std::string& type) {
        for (auto& f : flows) {
            if (f.type == type) return true;
        }
        return false;
    }

    static bool hasSso(const std::vector<LoginFlow>& flows) {
        return hasFlow(flows, "m.login.sso") || hasFlow(flows, "m.login.cas");
    }

    static json getIdentityProviders(const json& loginResponse) {
        if (loginResponse.contains("flows")) {
            for (auto& flow : loginResponse["flows"]) {
                if (flow.value("type", "") == "m.login.sso" && flow.contains("identity_providers")) {
                    return flow["identity_providers"];
                }
            }
        }
        return json::array();
    }
};

// ==== Registration Flow Handler ====

class RegistrationFlowHandler {
public:
    struct RegistrationStage {
        std::string type;
        std::string session;
        json params;
        std::string completed;
    };

    static bool requiresEmail(const json& regResponse) {
        if (regResponse.contains("flows")) {
            for (auto& flow : regResponse["flows"]) {
                if (flow.contains("stages")) {
                    for (auto& stage : flow["stages"]) {
                        if (stage == "m.login.email.identity") return true;
                    }
                }
            }
        }
        return false;
    }

    static bool requiresRecaptcha(const json& regResponse) {
        if (regResponse.contains("flows")) {
            for (auto& flow : regResponse["flows"]) {
                if (flow.contains("stages")) {
                    for (auto& stage : flow["stages"]) {
                        if (stage == "m.login.recaptcha") return true;
                    }
                }
            }
        }
        return false;
    }

    static bool requiresTerms(const json& regResponse) {
        if (regResponse.contains("flows")) {
            for (auto& flow : regResponse["flows"]) {
                if (flow.contains("stages")) {
                    for (auto& stage : flow["stages"]) {
                        if (stage == "m.login.terms") return true;
                    }
                }
            }
        }
        return false;
    }
};

// ==== User-Interactive Auth (UIA) handler ====

class UIAHandler {
public:
    struct UIAStage {
        std::string type;
        std::string session;
        json params;
        bool completed = false;
    };

    struct UIAFlow {
        std::vector<std::string> stages;
        std::map<std::string, UIAStage> completedStages;
    };

    static std::vector<UIAFlow> parseFlows(const json& response) {
        std::vector<UIAFlow> flows;
        if (response.contains("flows")) {
            for (auto& flow : response["flows"]) {
                UIAFlow uf;
                if (flow.contains("stages")) {
                    for (auto& stage : flow["stages"]) {
                        uf.stages.push_back(stage);
                    }
                }
                flows.push_back(uf);
            }
        }
        return flows;
    }

    static bool isRequired(const json& response) {
        return response.value("errcode", "") == "M_UNAUTHORIZED" &&
               response.contains("flows");
    }

    static std::string getSession(const json& response) {
        return response.value("session", "");
    }

    static json buildAuth(const std::string& session, const std::string& type,
                          const json& authData = json::object()) {
        json auth;
        auth["type"] = type;
        auth["session"] = session;
        for (auto& [k, v] : authData.items()) {
            if (k != "type" && k != "session") auth[k] = v;
        }
        return auth;
    }
};

// ==== Homeserver Health Checker ====

class HomeserverHealthChecker {
public:
    struct HealthStatus {
        bool reachable = false;
        std::string serverName;
        std::string serverVersion;
        int64_t latencyMs = 0;
        bool supportsFederation = false;
        bool tlsValid = false;
        std::string error;
    };

    static HealthStatus check(const std::string& homeserverUrl) {
        HealthStatus status;
        status.reachable = !homeserverUrl.empty();
        if (!homeserverUrl.empty()) {
            if (homeserverUrl.find("https://") == 0) {
                status.tlsValid = true;
            }
            status.serverName = homeserverUrl;
        }
        return status;
    }
};

// ==== Federation Tester ====

class FederationTester {
public:
    struct FederationStatus {
        std::string serverName;
        bool reachable = false;
        std::string serverVersion;
        std::vector<std::string> connectionErrors;
        int64_t responseTimeMs = 0;
    };

    static FederationStatus testServer(const std::string& serverName) {
        FederationStatus fs;
        fs.serverName = serverName;
        fs.reachable = !serverName.empty();
        return fs;
    }

    static FederationStatus testConnection(const std::string& fromServer,
                                            const std::string& toServer) {
        FederationStatus fs;
        fs.serverName = toServer;
        return fs;
    }
};

} // namespace matrix_sdk
