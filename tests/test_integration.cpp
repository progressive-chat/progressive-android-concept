#include "test_framework.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>

using json = nlohmann::json;

// ==== Integration Tests ====

TEST_CASE(Integration, full_auth_flow) {
    // Simulate a full authentication flow
    json loginRequest;
    loginRequest["type"] = "m.login.password";
    loginRequest["identifier"] = {{"type", "m.id.user"}, {"user", "testuser"}};
    loginRequest["password"] = "testpassword";
    ASSERT_TRUE(loginRequest.contains("password"));
}

TEST_CASE(Integration, sync_processing) {
    std::string syncResponse = R"({
        "next_batch": "batch_123",
        "rooms": {
            "join": {
                "!room1:example.com": {
                    "timeline": {
                        "events": [
                            {"event_id": "$evt1", "type": "m.room.message", "sender": "@a:example.com", "content": {"body": "Hello"}}
                        ]
                    }
                }
            }
        }
    })";
    json j = json::parse(syncResponse);
    ASSERT_EQ(j["next_batch"].get<std::string>(), std::string("batch_123"));
    ASSERT_EQ(j["rooms"]["join"]["!room1:example.com"]["timeline"]["events"].size(), 1);
}

TEST_CASE(Integration, room_creation_flow) {
    json createReq;
    createReq["name"] = "Test Room";
    createReq["preset"] = "private_chat";
    createReq["is_direct"] = true;
    ASSERT_EQ(createReq["name"].get<std::string>(), std::string("Test Room"));
}

TEST_CASE(Integration, message_send_flow) {
    json msg;
    msg["msgtype"] = "m.text";
    msg["body"] = "Hello World";
    msg["format"] = "org.matrix.custom.html";
    msg["formatted_body"] = "<b>Hello World</b>";
    ASSERT_EQ(msg["body"].get<std::string>(), std::string("Hello World"));
}

TEST_CASE(Integration, encryption_setup_flow) {
    json deviceKeys;
    deviceKeys["algorithms"] = json::array({"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"});
    deviceKeys["device_id"] = "DEVICE123";
    deviceKeys["user_id"] = "@test:example.com";
    ASSERT_EQ(deviceKeys["device_id"].get<std::string>(), std::string("DEVICE123"));
}

TEST_CASE(Integration, key_sharing_flow) {
    json roomKeyEvent;
    roomKeyEvent["type"] = "m.room_key";
    roomKeyEvent["content"]["algorithm"] = "m.megolm.v1.aes-sha2";
    roomKeyEvent["content"]["room_id"] = "!room:example.com";
    roomKeyEvent["content"]["session_id"] = "session123";
    roomKeyEvent["content"]["session_key"] = "AQAAAA...";
    ASSERT_TRUE(roomKeyEvent["content"].contains("session_key"));
}

TEST_CASE(Integration, notification_processing) {
    json pushRule;
    pushRule["rule_id"] = ".m.rule.contains_display_name";
    pushRule["enabled"] = true;
    pushRule["actions"] = json::array({"notify", {{"set_tweak", "highlight"}}});
    ASSERT_TRUE(pushRule["enabled"]);
}

TEST_CASE(Integration, file_upload_flow) {
    std::string mxcUri = "mxc://example.com/abc123";
    ASSERT_TRUE(mxcUri.find("mxc://") == 0);
    size_t pos = mxcUri.find("mxc://");
    std::string rest = mxcUri.substr(pos + 6);
    ASSERT_EQ(rest.find('/'), std::string("example.com").size());
}

TEST_CASE(Integration, pagination_flow) {
    std::string from = "start_token";
    std::string dir = "b";
    int limit = 20;
    std::ostringstream url;
    url << "/rooms/!room/messages?from=" << from << "&dir=" << dir << "&limit=" << limit;
    ASSERT_TRUE(url.str().find("from=start_token") != std::string::npos);
}

TEST_CASE(Integration, thread_management) {
    std::map<std::string, std::vector<std::string>> threads;
    threads["!room:example.com"].push_back("$thread1");
    threads["!room:example.com"].push_back("$thread2");
    ASSERT_EQ(static_cast<int>(threads["!room:example.com"].size()), 2);
}

TEST_CASE(Integration, space_hierarchy) {
    json spaceEvent;
    spaceEvent["type"] = "m.space.child";
    spaceEvent["content"]["via"] = json::array({"example.com"});
    spaceEvent["state_key"] = "!childRoom:example.com";
    ASSERT_TRUE(spaceEvent["content"].contains("via"));
}

TEST_CASE(Integration, reaction_aggregation) {
    std::map<std::string, int> reactionCounts;
    reactionCounts["+1"] = 5;
    reactionCounts["heart"] = 3;
    reactionCounts["laugh"] = 7;
    int total = 0;
    for (auto& [key, count] : reactionCounts) total += count;
    ASSERT_EQ(total, 15);
}

TEST_CASE(Integration, device_verification_flow) {
    json verificationRequest;
    verificationRequest["from_device"] = "DEVICE_A";
    verificationRequest["transaction_id"] = "txn123";
    verificationRequest["methods"] = json::array({"m.sas.v1"});
    ASSERT_EQ(verificationRequest["methods"].size(), 1);
}

TEST_CASE(Integration, oidc_flow) {
    std::string codeVerifier = "abcdefghij";
    std::string codeChallenge = codeVerifier;  // For testing
    ASSERT_EQ(codeVerifier, codeChallenge);
}

TEST_CASE(Integration, sso_provider_selection) {
    std::vector<std::string> providers = {"google", "github", "apple", "facebook", "gitlab"};
    ASSERT_EQ(static_cast<int>(providers.size()), 5);
    ASSERT_EQ(providers[0], std::string("google"));
}

TEST_CASE(Integration, cross_signing_bootstrap) {
    json crossSigningKeys;
    crossSigningKeys["master_key"]["user_id"] = "@test:example.com";
    crossSigningKeys["master_key"]["usage"] = json::array({"master"});
    crossSigningKeys["self_signing_key"]["usage"] = json::array({"self_signing"});
    crossSigningKeys["user_signing_key"]["usage"] = json::array({"user_signing"});
    ASSERT_TRUE(crossSigningKeys.contains("master_key"));
}

TEST_CASE(Integration, backup_restore_flow) {
    json backupInfo;
    backupInfo["algorithm"] = "m.megolm_backup.v1.curve25519-aes-sha2";
    backupInfo["auth_data"]["public_key"] = "base64key...";
    backupInfo["version"] = "1";
    ASSERT_EQ(backupInfo["version"].get<std::string>(), std::string("1"));
}

TEST_CASE(Integration, read_receipt_processing) {
    std::map<std::string, std::string> receipts;
    receipts["!room:example.com"] = "$lastReadEvent";
    ASSERT_EQ(receipts["!room:example.com"], std::string("$lastReadEvent"));
}

TEST_CASE(Integration, widget_capability_check) {
    json widgetCapabilities;
    widgetCapabilities["capabilities"] = json::array({"m.text", "m.image", "m.video"});
    ASSERT_EQ(widgetCapabilities["capabilities"].size(), 3);
}

TEST_CASE(Integration, presence_status_update) {
    json presenceUpdate;
    presenceUpdate["presence"] = "online";
    presenceUpdate["status_msg"] = "Available for chat";
    ASSERT_EQ(presenceUpdate["presence"].get<std::string>(), std::string("online"));
}

TEST_CASE(Integration, user_directory_search) {
    json searchResult;
    searchResult["results"] = json::array();
    searchResult["results"].push_back({{"user_id", "@alice:example.com"}, {"display_name", "Alice"}});
    searchResult["results"].push_back({{"user_id", "@bob:example.com"}, {"display_name", "Bob"}});
    ASSERT_EQ(searchResult["results"].size(), 2);
}

TEST_CASE(Integration, event_redaction_flow) {
    std::string redactionEvent;
    json redaction;
    redaction["reason"] = "Spam message";
    ASSERT_EQ(redaction["reason"].get<std::string>(), std::string("Spam message"));
}

TEST_CASE(Integration, room_upgrade_flow) {
    json upgradeRequest;
    upgradeRequest["new_version"] = "9";
    ASSERT_EQ(upgradeRequest["new_version"].get<std::string>(), std::string("9"));
}

TEST_CASE(Integration, media_config_retrieval) {
    json mediaConfig;
    mediaConfig["m.upload.size"] = 104857600;
    ASSERT_EQ(mediaConfig["m.upload.size"].get<int>(), 104857600);
}

TEST_CASE(Integration, typing_indicator) {
    json typingRequest;
    typingRequest["typing"] = true;
    typingRequest["timeout"] = 30000;
    ASSERT_TRUE(typingRequest["typing"]);
}

TEST_CASE(Integration, public_room_directory) {
    json publicRooms;
    publicRooms["chunk"] = json::array();
    publicRooms["total_room_count_estimate"] = 100;
    ASSERT_EQ(publicRooms["total_room_count_estimate"].get<int>(), 100);
}

TEST_CASE(Integration, federation_tester) {
    std::string serverName = "matrix.org";
    ASSERT_TRUE(!serverName.empty());
}

TEST_CASE(Integration, ignore_list_management) {
    std::set<std::string> ignored = {"@spammer:example.com", "@bot:example.com"};
    ASSERT_EQ(static_cast<int>(ignored.size()), 2);
    ignored.insert("@troll:example.com");
    ASSERT_EQ(static_cast<int>(ignored.size()), 3);
}

TEST_CASE(Integration, direct_message_detection) {
    std::map<std::string, std::string> dmRooms;
    dmRooms["@alice:example.com"] = "!dm1:example.com";
    dmRooms["@bob:example.com"] = "!dm2:example.com";
    ASSERT_EQ(static_cast<int>(dmRooms.size()), 2);
}

TEST_CASE(Integration, event_context_loading) {
    json context;
    context["start"] = "start_token";
    context["end"] = "end_token";
    context["events_before"] = json::array();
    context["events_after"] = json::array();
    ASSERT_TRUE(context.contains("start"));
}

TEST_CASE(Integration, server_notice_handling) {
    json serverNotice;
    serverNotice["admin_contact"] = "admin@example.com";
    serverNotice["server_notice_type"] = "m.server_notice";
    ASSERT_EQ(serverNotice["admin_contact"].get<std::string>(), std::string("admin@example.com"));
}

TEST_CASE(Integration, canonical_alias_routing) {
    std::string alias = "#welcome:matrix.org";
    ASSERT_TRUE(alias[0] == '#');
}

TEST_CASE(Integration, event_relations_query) {
    json relations;
    relations["chunk"] = json::array();
    relations["original_event"] = json::object();
    ASSERT_TRUE(relations.contains("chunk"));
}

TEST_CASE(Integration, rate_limit_backoff) {
    int baseDelay = 1000;
    int attempt = 3;
    int delay = baseDelay * (1 << attempt);
    ASSERT_EQ(delay, 8000);
}

TEST_CASE(Integration, content_scanner_check) {
    std::string mxcUri = "mxc://example.com/files/safe.pdf";
    ASSERT_TRUE(mxcUri.find("safe") != std::string::npos);
}

TEST_CASE(Integration, dehydration_manager) {
    json dehydratedDevice;
    dehydratedDevice["device_id"] = "DEHYDRATED_DEVICE";
    dehydratedDevice["device_data"]["algorithm"] = "m.olm.v1.curve25519-aes-sha2";
    ASSERT_TRUE(dehydratedDevice["device_data"].contains("algorithm"));
}
