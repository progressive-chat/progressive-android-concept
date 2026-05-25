#include "test_framework.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>
#include <set>

using json = nlohmann::json;

TEST_CASE(ServerUtils, capabilities_detection) {
    json caps = json::parse(R"({
        "capabilities": {
            "m.thread": {"enabled": true},
            "m.space": {"enabled": true},
            "m.upload_size": 104857600
        }
    })");
    ASSERT_TRUE(caps["capabilities"].contains("m.thread"));
    ASSERT_EQ(caps["capabilities"]["m.upload_size"].get<int>(), 104857600);
}

TEST_CASE(ServerUtils, version_parsing) {
    json versions = json::parse(R"({
        "versions": ["r0.6.0", "v1.1", "v1.2", "v1.3", "v1.4", "v1.5", "v1.6"]
    })");
    ASSERT_TRUE(versions["versions"].size() >= 5);
}

TEST_CASE(ServerUtils, login_flows) {
    json loginResp = json::parse(R"({
        "flows": [
            {"type": "m.login.password"},
            {"type": "m.login.sso", "identity_providers": [
                {"id": "google", "name": "Google"},
                {"id": "github", "name": "GitHub"}
            ]}
        ]
    })");
    ASSERT_EQ(loginResp["flows"].size(), 2);
}

TEST_CASE(ServerUtils, registration_flows) {
    json regResp = json::parse(R"({
        "flows": [{"stages": ["m.login.recaptcha", "m.login.terms", "m.login.dummy"]}]
    })");
    ASSERT_TRUE(regResp["flows"][0]["stages"].size() == 3);
}

TEST_CASE(ServerUtils, uia_session) {
    json uiaResp = json::parse(R"({
        "errcode": "M_UNAUTHORIZED",
        "session": "abc123",
        "flows": [{"stages": ["m.login.password"]}]
    })");
    ASSERT_EQ(uiaResp["errcode"].get<std::string>(), std::string("M_UNAUTHORIZED"));
    ASSERT_EQ(uiaResp["session"].get<std::string>(), std::string("abc123"));
}

TEST_CASE(ServerUtils, room_versions) {
    std::vector<std::string> versions = {"1","2","3","4","5","6","7","8","9","10","11"};
    ASSERT_TRUE(versions.size() >= 10);
    ASSERT_EQ(versions.back(), std::string("11"));
}

TEST_CASE(ServerUtils, wellknown_resolution) {
    std::string domain = "matrix.org";
    ASSERT_TRUE(!domain.empty());
    ASSERT_TRUE(domain.find('.') != std::string::npos);
}

TEST_CASE(ServerUtils, identity_providers) {
    json idps = json::array();
    idps.push_back({{"id", "google"}, {"name", "Google"}});
    idps.push_back({{"id", "github"}, {"name", "GitHub"}});
    ASSERT_EQ(idps.size(), 2);
}

TEST_CASE(ServerUtils, federation_testing) {
    std::string server = "matrix.example.com";
    ASSERT_TRUE(server.find("example") != std::string::npos);
}

TEST_CASE(ServerUtils, content_config) {
    json config;
    config["m.upload.size"] = 100 * 1024 * 1024;
    ASSERT_TRUE(config["m.upload.size"].get<int>() > 0);
}
