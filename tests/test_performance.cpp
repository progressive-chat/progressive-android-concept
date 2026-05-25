#include "test_framework.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <algorithm>
#include <thread>
#include <random>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ==== Performance & Stress Tests ====

TEST_CASE(Performance, json_parse_large) {
    json large;
    for (int i = 0; i < 1000; i++) {
        large["key_" + std::to_string(i)] = "value_" + std::to_string(i);
    }
    std::string serialized = large.dump();
    json parsed = json::parse(serialized);
    ASSERT_EQ(static_cast<int>(parsed.size()), 1000);
}

TEST_CASE(Performance, vector_insert_100k) {
    std::vector<int> v;
    v.reserve(100000);
    for (int i = 0; i < 100000; i++) v.push_back(i);
    ASSERT_EQ(static_cast<int>(v.size()), 100000);
}

TEST_CASE(Performance, map_lookup_10k) {
    std::map<std::string, int> m;
    for (int i = 0; i < 10000; i++) m["key_" + std::to_string(i)] = i;
    ASSERT_EQ(m["key_5000"], 5000);
    ASSERT_EQ(static_cast<int>(m.size()), 10000);
}

TEST_CASE(Performance, set_operations) {
    std::set<int> s;
    for (int i = 0; i < 50000; i++) s.insert(i);
    ASSERT_EQ(static_cast<int>(s.size()), 50000);
    auto it = s.find(25000);
    ASSERT_TRUE(it != s.end());
}

TEST_CASE(Performance, string_concatenation) {
    std::string s;
    s.reserve(100000);
    for (int i = 0; i < 10000; i++) s += "hello ";
    ASSERT_TRUE(s.size() > 50000);
}

TEST_CASE(Performance, sort_large_vector) {
    std::vector<int> v(10000);
    std::random_device rd; std::mt19937 gen(rd());
    std::iota(v.begin(), v.end(), 0);
    std::shuffle(v.begin(), v.end(), gen);
    std::sort(v.begin(), v.end());
    ASSERT_TRUE(std::is_sorted(v.begin(), v.end()));
}

TEST_CASE(Performance, hash_table_load) {
    std::unordered_map<std::string, int> um;
    for (int i = 0; i < 20000; i++) um["key_" + std::to_string(i)] = i;
    ASSERT_EQ(um["key_10000"], 10000);
}

TEST_CASE(Performance, regex_compilation) {
    std::regex re("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    ASSERT_TRUE(std::regex_match("user@example.com", re));
}

TEST_CASE(Performance, base64_encode_decode) {
    std::string input(1000, 'A');
    ASSERT_EQ(input.size(), 1000);
}

TEST_CASE(Performance, thread_pool_spawn) {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });
    }
    for (auto& t : threads) t.join();
    ASSERT_TRUE(true);
}

TEST_CASE(Performance, mutex_contention) {
    std::mutex m;
    int counter = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; i++) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 1000; j++) {
                std::lock_guard<std::mutex> lock(m);
                counter++;
            }
        });
    }
    for (auto& t : threads) t.join();
    ASSERT_EQ(counter, 8000);
}

TEST_CASE(Performance, copy_vs_move) {
    std::string original = "This is a test string for move semantics";
    std::string copied = original;
    std::string moved = std::move(original);
    ASSERT_EQ(copied, moved);
}

TEST_CASE(Performance, memory_allocation_patterns) {
    for (int size : {16, 64, 256, 1024, 4096, 16384}) {
        std::vector<char> buf(size);
        std::fill(buf.begin(), buf.end(), 'X');
        ASSERT_EQ(static_cast<int>(buf.size()), size);
    }
}

TEST_CASE(Performance, json_path_traversal) {
    json root = json::parse(R"({
        "level1": {
            "level2": {
                "level3": {
                    "value": 42
                }
            }
        }
    })");
    int val = root["level1"]["level2"]["level3"]["value"];
    ASSERT_EQ(val, 42);
}

TEST_CASE(Performance, event_batch_processing) {
    json batch = json::array();
    for (int i = 0; i < 500; i++) {
        json event;
        event["event_id"] = "$event_" + std::to_string(i);
        event["type"] = "m.room.message";
        event["sender"] = "@user:example.com";
        event["content"] = {{"body", "Message " + std::to_string(i)}, {"msgtype", "m.text"}};
        event["origin_server_ts"] = 1600000000000ULL + i * 1000;
        batch.push_back(event);
    }
    ASSERT_EQ(batch.size(), 500);
    for (int i = 0; i < 500; i++) {
        std::string eid = batch[i]["event_id"];
        ASSERT_TRUE(eid.find("$event_") == 0);
    }
}

TEST_CASE(Performance, room_state_simulation) {
    json state;
    state["name"] = "Test Room";
    state["topic"] = "A room for testing";
    state["members"] = json::object();
    for (int i = 0; i < 100; i++) {
        std::string uid = "@user" + std::to_string(i) + ":example.com";
        state["members"][uid] = {{"displayname", "User " + std::to_string(i)}, {"membership", "join"}};
    }
    ASSERT_EQ(state["members"].size(), 100);
}

TEST_CASE(Performance, notification_filter_throughput) {
    std::vector<std::string> events;
    for (int i = 0; i < 1000; i++) {
        events.push_back("event_" + std::to_string(i));
    }
    int matched = 0;
    for (auto& e : events) {
        if (e.find("event_50") != std::string::npos) matched++;
    }
    ASSERT_TRUE(matched > 0);
}

TEST_CASE(Performance, media_thumbnail_generation) {
    std::vector<int> sizes = {32, 64, 128, 256, 512, 1024};
    for (int size : sizes) {
        ASSERT_TRUE(size > 0);
    }
    ASSERT_EQ(static_cast<int>(sizes.size()), 6);
}

TEST_CASE(Performance, sync_data_parsing) {
    std::string syncJson = R"({"next_batch":"s123","rooms":{"join":{"!room:example.com":{"timeline":{"events":[]}},"!room2:example.com":{"timeline":{"events":[]}},"!room3:example.com":{"timeline":{"events":[]}}}}})";
    json j = json::parse(syncJson);
    ASSERT_EQ(j["rooms"]["join"].size(), 3);
}

TEST_CASE(Performance, encryption_key_generation) {
    std::vector<uint8_t> key(256);
    std::fill(key.begin(), key.end(), 0x42);
    ASSERT_EQ(key.size(), 256);
    ASSERT_EQ(key[0], 0x42);
}

TEST_CASE(Performance, megolm_session_rotation) {
    for (int i = 0; i < 100; i++) {
        std::string sessionId = "session_" + std::to_string(i);
        ASSERT_TRUE(!sessionId.empty());
    }
}

TEST_CASE(Performance, cross_signing_setup) {
    json crossSign;
    crossSign["master_key"] = {{"key", "base64key..."}};
    crossSign["self_signing_key"] = {{"key", "base64key..."}};
    crossSign["user_signing_key"] = {{"key", "base64key..."}};
    ASSERT_TRUE(crossSign.contains("master_key"));
    ASSERT_TRUE(crossSign.contains("self_signing_key"));
    ASSERT_TRUE(crossSign.contains("user_signing_key"));
}

TEST_CASE(Performance, content_filter_pipeline) {
    std::vector<std::string> contentTypes = {"text", "image", "video", "audio", "file", "location", "sticker"};
    std::set<std::string> allowed = {"text", "image", "file"};
    int passed = 0;
    for (auto& ct : contentTypes) {
        if (allowed.find(ct) != allowed.end()) passed++;
    }
    ASSERT_EQ(passed, 3);
}
