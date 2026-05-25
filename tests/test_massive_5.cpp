#include "test_framework.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <random>
#include <algorithm>

using json = nlohmann::json;

TEST_CASE(MassiveSuite5, test_case_1) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_1";
    ASSERT_FALSE(data.empty());
    int val = 1;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 1;
    j["name"] = "test_5_1";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 1 * 100;
    ASSERT_EQ(j["id"].get<int>(), 1);
    std::vector<int> vec;
    for (int k = 0; k < 1 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 1;
    ASSERT_EQ(m["key"], 1);
}

TEST_CASE(MassiveSuite5, test_case_2) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_2";
    ASSERT_FALSE(data.empty());
    int val = 2;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 2;
    j["name"] = "test_5_2";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 2 * 100;
    ASSERT_EQ(j["id"].get<int>(), 2);
    std::vector<int> vec;
    for (int k = 0; k < 2 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 2;
    ASSERT_EQ(m["key"], 2);
}

TEST_CASE(MassiveSuite5, test_case_3) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_3";
    ASSERT_FALSE(data.empty());
    int val = 3;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 3;
    j["name"] = "test_5_3";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 3 * 100;
    ASSERT_EQ(j["id"].get<int>(), 3);
    std::vector<int> vec;
    for (int k = 0; k < 3 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 3;
    ASSERT_EQ(m["key"], 3);
}

TEST_CASE(MassiveSuite5, test_case_4) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_4";
    ASSERT_FALSE(data.empty());
    int val = 4;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 4;
    j["name"] = "test_5_4";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 4 * 100;
    ASSERT_EQ(j["id"].get<int>(), 4);
    std::vector<int> vec;
    for (int k = 0; k < 4 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 4;
    ASSERT_EQ(m["key"], 4);
}

TEST_CASE(MassiveSuite5, test_case_5) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_5";
    ASSERT_FALSE(data.empty());
    int val = 5;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 5;
    j["name"] = "test_5_5";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 5 * 100;
    ASSERT_EQ(j["id"].get<int>(), 5);
    std::vector<int> vec;
    for (int k = 0; k < 5 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 5;
    ASSERT_EQ(m["key"], 5);
}

TEST_CASE(MassiveSuite5, test_case_6) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_6";
    ASSERT_FALSE(data.empty());
    int val = 6;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 6;
    j["name"] = "test_5_6";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 6 * 100;
    ASSERT_EQ(j["id"].get<int>(), 6);
    std::vector<int> vec;
    for (int k = 0; k < 6 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 6;
    ASSERT_EQ(m["key"], 6);
}

TEST_CASE(MassiveSuite5, test_case_7) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_7";
    ASSERT_FALSE(data.empty());
    int val = 7;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 7;
    j["name"] = "test_5_7";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 7 * 100;
    ASSERT_EQ(j["id"].get<int>(), 7);
    std::vector<int> vec;
    for (int k = 0; k < 7 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 7;
    ASSERT_EQ(m["key"], 7);
}

TEST_CASE(MassiveSuite5, test_case_8) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_8";
    ASSERT_FALSE(data.empty());
    int val = 8;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 8;
    j["name"] = "test_5_8";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 8 * 100;
    ASSERT_EQ(j["id"].get<int>(), 8);
    std::vector<int> vec;
    for (int k = 0; k < 8 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 8;
    ASSERT_EQ(m["key"], 8);
}

TEST_CASE(MassiveSuite5, test_case_9) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_9";
    ASSERT_FALSE(data.empty());
    int val = 9;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 9;
    j["name"] = "test_5_9";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 9 * 100;
    ASSERT_EQ(j["id"].get<int>(), 9);
    std::vector<int> vec;
    for (int k = 0; k < 9 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 9;
    ASSERT_EQ(m["key"], 9);
}

TEST_CASE(MassiveSuite5, test_case_10) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_10";
    ASSERT_FALSE(data.empty());
    int val = 10;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 10;
    j["name"] = "test_5_10";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 10 * 100;
    ASSERT_EQ(j["id"].get<int>(), 10);
    std::vector<int> vec;
    for (int k = 0; k < 10 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 10;
    ASSERT_EQ(m["key"], 10);
}

TEST_CASE(MassiveSuite5, test_case_11) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_11";
    ASSERT_FALSE(data.empty());
    int val = 11;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 11;
    j["name"] = "test_5_11";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 11 * 100;
    ASSERT_EQ(j["id"].get<int>(), 11);
    std::vector<int> vec;
    for (int k = 0; k < 11 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 11;
    ASSERT_EQ(m["key"], 11);
}

TEST_CASE(MassiveSuite5, test_case_12) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_12";
    ASSERT_FALSE(data.empty());
    int val = 12;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 12;
    j["name"] = "test_5_12";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 12 * 100;
    ASSERT_EQ(j["id"].get<int>(), 12);
    std::vector<int> vec;
    for (int k = 0; k < 12 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 12;
    ASSERT_EQ(m["key"], 12);
}

TEST_CASE(MassiveSuite5, test_case_13) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_13";
    ASSERT_FALSE(data.empty());
    int val = 13;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 13;
    j["name"] = "test_5_13";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 13 * 100;
    ASSERT_EQ(j["id"].get<int>(), 13);
    std::vector<int> vec;
    for (int k = 0; k < 13 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 13;
    ASSERT_EQ(m["key"], 13);
}

TEST_CASE(MassiveSuite5, test_case_14) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_14";
    ASSERT_FALSE(data.empty());
    int val = 14;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 14;
    j["name"] = "test_5_14";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 14 * 100;
    ASSERT_EQ(j["id"].get<int>(), 14);
    std::vector<int> vec;
    for (int k = 0; k < 14 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 14;
    ASSERT_EQ(m["key"], 14);
}

TEST_CASE(MassiveSuite5, test_case_15) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_15";
    ASSERT_FALSE(data.empty());
    int val = 15;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 15;
    j["name"] = "test_5_15";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 15 * 100;
    ASSERT_EQ(j["id"].get<int>(), 15);
    std::vector<int> vec;
    for (int k = 0; k < 15 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 15;
    ASSERT_EQ(m["key"], 15);
}

TEST_CASE(MassiveSuite5, test_case_16) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_16";
    ASSERT_FALSE(data.empty());
    int val = 16;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 16;
    j["name"] = "test_5_16";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 16 * 100;
    ASSERT_EQ(j["id"].get<int>(), 16);
    std::vector<int> vec;
    for (int k = 0; k < 16 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 16;
    ASSERT_EQ(m["key"], 16);
}

TEST_CASE(MassiveSuite5, test_case_17) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_17";
    ASSERT_FALSE(data.empty());
    int val = 17;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 17;
    j["name"] = "test_5_17";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 17 * 100;
    ASSERT_EQ(j["id"].get<int>(), 17);
    std::vector<int> vec;
    for (int k = 0; k < 17 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 17;
    ASSERT_EQ(m["key"], 17);
}

TEST_CASE(MassiveSuite5, test_case_18) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_18";
    ASSERT_FALSE(data.empty());
    int val = 18;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 18;
    j["name"] = "test_5_18";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 18 * 100;
    ASSERT_EQ(j["id"].get<int>(), 18);
    std::vector<int> vec;
    for (int k = 0; k < 18 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 18;
    ASSERT_EQ(m["key"], 18);
}

TEST_CASE(MassiveSuite5, test_case_19) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_19";
    ASSERT_FALSE(data.empty());
    int val = 19;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 19;
    j["name"] = "test_5_19";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 19 * 100;
    ASSERT_EQ(j["id"].get<int>(), 19);
    std::vector<int> vec;
    for (int k = 0; k < 19 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 19;
    ASSERT_EQ(m["key"], 19);
}

TEST_CASE(MassiveSuite5, test_case_20) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_20";
    ASSERT_FALSE(data.empty());
    int val = 20;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 20;
    j["name"] = "test_5_20";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 20 * 100;
    ASSERT_EQ(j["id"].get<int>(), 20);
    std::vector<int> vec;
    for (int k = 0; k < 20 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 20;
    ASSERT_EQ(m["key"], 20);
}

TEST_CASE(MassiveSuite5, test_case_21) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_21";
    ASSERT_FALSE(data.empty());
    int val = 21;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 21;
    j["name"] = "test_5_21";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 21 * 100;
    ASSERT_EQ(j["id"].get<int>(), 21);
    std::vector<int> vec;
    for (int k = 0; k < 21 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 21;
    ASSERT_EQ(m["key"], 21);
}

TEST_CASE(MassiveSuite5, test_case_22) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_22";
    ASSERT_FALSE(data.empty());
    int val = 22;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 22;
    j["name"] = "test_5_22";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 22 * 100;
    ASSERT_EQ(j["id"].get<int>(), 22);
    std::vector<int> vec;
    for (int k = 0; k < 22 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 22;
    ASSERT_EQ(m["key"], 22);
}

TEST_CASE(MassiveSuite5, test_case_23) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_23";
    ASSERT_FALSE(data.empty());
    int val = 23;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 23;
    j["name"] = "test_5_23";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 23 * 100;
    ASSERT_EQ(j["id"].get<int>(), 23);
    std::vector<int> vec;
    for (int k = 0; k < 23 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 23;
    ASSERT_EQ(m["key"], 23);
}

TEST_CASE(MassiveSuite5, test_case_24) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_24";
    ASSERT_FALSE(data.empty());
    int val = 24;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 24;
    j["name"] = "test_5_24";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 24 * 100;
    ASSERT_EQ(j["id"].get<int>(), 24);
    std::vector<int> vec;
    for (int k = 0; k < 24 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 24;
    ASSERT_EQ(m["key"], 24);
}

TEST_CASE(MassiveSuite5, test_case_25) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_25";
    ASSERT_FALSE(data.empty());
    int val = 25;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 25;
    j["name"] = "test_5_25";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 25 * 100;
    ASSERT_EQ(j["id"].get<int>(), 25);
    std::vector<int> vec;
    for (int k = 0; k < 25 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 25;
    ASSERT_EQ(m["key"], 25);
}

TEST_CASE(MassiveSuite5, test_case_26) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_26";
    ASSERT_FALSE(data.empty());
    int val = 26;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 26;
    j["name"] = "test_5_26";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 26 * 100;
    ASSERT_EQ(j["id"].get<int>(), 26);
    std::vector<int> vec;
    for (int k = 0; k < 26 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 26;
    ASSERT_EQ(m["key"], 26);
}

TEST_CASE(MassiveSuite5, test_case_27) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_27";
    ASSERT_FALSE(data.empty());
    int val = 27;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 27;
    j["name"] = "test_5_27";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 27 * 100;
    ASSERT_EQ(j["id"].get<int>(), 27);
    std::vector<int> vec;
    for (int k = 0; k < 27 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 27;
    ASSERT_EQ(m["key"], 27);
}

TEST_CASE(MassiveSuite5, test_case_28) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_28";
    ASSERT_FALSE(data.empty());
    int val = 28;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 28;
    j["name"] = "test_5_28";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 28 * 100;
    ASSERT_EQ(j["id"].get<int>(), 28);
    std::vector<int> vec;
    for (int k = 0; k < 28 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 28;
    ASSERT_EQ(m["key"], 28);
}

TEST_CASE(MassiveSuite5, test_case_29) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_29";
    ASSERT_FALSE(data.empty());
    int val = 29;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 29;
    j["name"] = "test_5_29";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 29 * 100;
    ASSERT_EQ(j["id"].get<int>(), 29);
    std::vector<int> vec;
    for (int k = 0; k < 29 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 29;
    ASSERT_EQ(m["key"], 29);
}

TEST_CASE(MassiveSuite5, test_case_30) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_30";
    ASSERT_FALSE(data.empty());
    int val = 30;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 30;
    j["name"] = "test_5_30";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 30 * 100;
    ASSERT_EQ(j["id"].get<int>(), 30);
    std::vector<int> vec;
    for (int k = 0; k < 30 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 30;
    ASSERT_EQ(m["key"], 30);
}

TEST_CASE(MassiveSuite5, test_case_31) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_31";
    ASSERT_FALSE(data.empty());
    int val = 31;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 31;
    j["name"] = "test_5_31";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 31 * 100;
    ASSERT_EQ(j["id"].get<int>(), 31);
    std::vector<int> vec;
    for (int k = 0; k < 31 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 31;
    ASSERT_EQ(m["key"], 31);
}

TEST_CASE(MassiveSuite5, test_case_32) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_32";
    ASSERT_FALSE(data.empty());
    int val = 32;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 32;
    j["name"] = "test_5_32";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 32 * 100;
    ASSERT_EQ(j["id"].get<int>(), 32);
    std::vector<int> vec;
    for (int k = 0; k < 32 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 32;
    ASSERT_EQ(m["key"], 32);
}

TEST_CASE(MassiveSuite5, test_case_33) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_33";
    ASSERT_FALSE(data.empty());
    int val = 33;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 33;
    j["name"] = "test_5_33";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 33 * 100;
    ASSERT_EQ(j["id"].get<int>(), 33);
    std::vector<int> vec;
    for (int k = 0; k < 33 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 33;
    ASSERT_EQ(m["key"], 33);
}

TEST_CASE(MassiveSuite5, test_case_34) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_34";
    ASSERT_FALSE(data.empty());
    int val = 34;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 34;
    j["name"] = "test_5_34";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 34 * 100;
    ASSERT_EQ(j["id"].get<int>(), 34);
    std::vector<int> vec;
    for (int k = 0; k < 34 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 34;
    ASSERT_EQ(m["key"], 34);
}

TEST_CASE(MassiveSuite5, test_case_35) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_35";
    ASSERT_FALSE(data.empty());
    int val = 35;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 35;
    j["name"] = "test_5_35";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 35 * 100;
    ASSERT_EQ(j["id"].get<int>(), 35);
    std::vector<int> vec;
    for (int k = 0; k < 35 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 35;
    ASSERT_EQ(m["key"], 35);
}

TEST_CASE(MassiveSuite5, test_case_36) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_36";
    ASSERT_FALSE(data.empty());
    int val = 36;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 36;
    j["name"] = "test_5_36";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 36 * 100;
    ASSERT_EQ(j["id"].get<int>(), 36);
    std::vector<int> vec;
    for (int k = 0; k < 36 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 36;
    ASSERT_EQ(m["key"], 36);
}

TEST_CASE(MassiveSuite5, test_case_37) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_37";
    ASSERT_FALSE(data.empty());
    int val = 37;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 37;
    j["name"] = "test_5_37";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 37 * 100;
    ASSERT_EQ(j["id"].get<int>(), 37);
    std::vector<int> vec;
    for (int k = 0; k < 37 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 37;
    ASSERT_EQ(m["key"], 37);
}

TEST_CASE(MassiveSuite5, test_case_38) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_38";
    ASSERT_FALSE(data.empty());
    int val = 38;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 38;
    j["name"] = "test_5_38";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 38 * 100;
    ASSERT_EQ(j["id"].get<int>(), 38);
    std::vector<int> vec;
    for (int k = 0; k < 38 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 38;
    ASSERT_EQ(m["key"], 38);
}

TEST_CASE(MassiveSuite5, test_case_39) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_39";
    ASSERT_FALSE(data.empty());
    int val = 39;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 39;
    j["name"] = "test_5_39";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 39 * 100;
    ASSERT_EQ(j["id"].get<int>(), 39);
    std::vector<int> vec;
    for (int k = 0; k < 39 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 39;
    ASSERT_EQ(m["key"], 39);
}

TEST_CASE(MassiveSuite5, test_case_40) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_40";
    ASSERT_FALSE(data.empty());
    int val = 40;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 40;
    j["name"] = "test_5_40";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 40 * 100;
    ASSERT_EQ(j["id"].get<int>(), 40);
    std::vector<int> vec;
    for (int k = 0; k < 40 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 40;
    ASSERT_EQ(m["key"], 40);
}

TEST_CASE(MassiveSuite5, test_case_41) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_41";
    ASSERT_FALSE(data.empty());
    int val = 41;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 41;
    j["name"] = "test_5_41";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 41 * 100;
    ASSERT_EQ(j["id"].get<int>(), 41);
    std::vector<int> vec;
    for (int k = 0; k < 41 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 41;
    ASSERT_EQ(m["key"], 41);
}

TEST_CASE(MassiveSuite5, test_case_42) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_42";
    ASSERT_FALSE(data.empty());
    int val = 42;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 42;
    j["name"] = "test_5_42";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 42 * 100;
    ASSERT_EQ(j["id"].get<int>(), 42);
    std::vector<int> vec;
    for (int k = 0; k < 42 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 42;
    ASSERT_EQ(m["key"], 42);
}

TEST_CASE(MassiveSuite5, test_case_43) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_43";
    ASSERT_FALSE(data.empty());
    int val = 43;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 43;
    j["name"] = "test_5_43";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 43 * 100;
    ASSERT_EQ(j["id"].get<int>(), 43);
    std::vector<int> vec;
    for (int k = 0; k < 43 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 43;
    ASSERT_EQ(m["key"], 43);
}

TEST_CASE(MassiveSuite5, test_case_44) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_44";
    ASSERT_FALSE(data.empty());
    int val = 44;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 44;
    j["name"] = "test_5_44";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 44 * 100;
    ASSERT_EQ(j["id"].get<int>(), 44);
    std::vector<int> vec;
    for (int k = 0; k < 44 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 44;
    ASSERT_EQ(m["key"], 44);
}

TEST_CASE(MassiveSuite5, test_case_45) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_45";
    ASSERT_FALSE(data.empty());
    int val = 45;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 45;
    j["name"] = "test_5_45";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 45 * 100;
    ASSERT_EQ(j["id"].get<int>(), 45);
    std::vector<int> vec;
    for (int k = 0; k < 45 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 45;
    ASSERT_EQ(m["key"], 45);
}

TEST_CASE(MassiveSuite5, test_case_46) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_46";
    ASSERT_FALSE(data.empty());
    int val = 46;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 46;
    j["name"] = "test_5_46";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 46 * 100;
    ASSERT_EQ(j["id"].get<int>(), 46);
    std::vector<int> vec;
    for (int k = 0; k < 46 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 46;
    ASSERT_EQ(m["key"], 46);
}

TEST_CASE(MassiveSuite5, test_case_47) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_47";
    ASSERT_FALSE(data.empty());
    int val = 47;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 47;
    j["name"] = "test_5_47";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 47 * 100;
    ASSERT_EQ(j["id"].get<int>(), 47);
    std::vector<int> vec;
    for (int k = 0; k < 47 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 47;
    ASSERT_EQ(m["key"], 47);
}

TEST_CASE(MassiveSuite5, test_case_48) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_48";
    ASSERT_FALSE(data.empty());
    int val = 48;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 48;
    j["name"] = "test_5_48";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 48 * 100;
    ASSERT_EQ(j["id"].get<int>(), 48);
    std::vector<int> vec;
    for (int k = 0; k < 48 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 48;
    ASSERT_EQ(m["key"], 48);
}

TEST_CASE(MassiveSuite5, test_case_49) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_49";
    ASSERT_FALSE(data.empty());
    int val = 49;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 49;
    j["name"] = "test_5_49";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 49 * 100;
    ASSERT_EQ(j["id"].get<int>(), 49);
    std::vector<int> vec;
    for (int k = 0; k < 49 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 49;
    ASSERT_EQ(m["key"], 49);
}

TEST_CASE(MassiveSuite5, test_case_50) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_50";
    ASSERT_FALSE(data.empty());
    int val = 50;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 50;
    j["name"] = "test_5_50";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 50 * 100;
    ASSERT_EQ(j["id"].get<int>(), 50);
    std::vector<int> vec;
    for (int k = 0; k < 50 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 50;
    ASSERT_EQ(m["key"], 50);
}

TEST_CASE(MassiveSuite5, test_case_51) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_51";
    ASSERT_FALSE(data.empty());
    int val = 51;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 51;
    j["name"] = "test_5_51";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 51 * 100;
    ASSERT_EQ(j["id"].get<int>(), 51);
    std::vector<int> vec;
    for (int k = 0; k < 51 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 51;
    ASSERT_EQ(m["key"], 51);
}

TEST_CASE(MassiveSuite5, test_case_52) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_52";
    ASSERT_FALSE(data.empty());
    int val = 52;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 52;
    j["name"] = "test_5_52";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 52 * 100;
    ASSERT_EQ(j["id"].get<int>(), 52);
    std::vector<int> vec;
    for (int k = 0; k < 52 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 52;
    ASSERT_EQ(m["key"], 52);
}

TEST_CASE(MassiveSuite5, test_case_53) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_53";
    ASSERT_FALSE(data.empty());
    int val = 53;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 53;
    j["name"] = "test_5_53";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 53 * 100;
    ASSERT_EQ(j["id"].get<int>(), 53);
    std::vector<int> vec;
    for (int k = 0; k < 53 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 53;
    ASSERT_EQ(m["key"], 53);
}

TEST_CASE(MassiveSuite5, test_case_54) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_54";
    ASSERT_FALSE(data.empty());
    int val = 54;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 54;
    j["name"] = "test_5_54";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 54 * 100;
    ASSERT_EQ(j["id"].get<int>(), 54);
    std::vector<int> vec;
    for (int k = 0; k < 54 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 54;
    ASSERT_EQ(m["key"], 54);
}

TEST_CASE(MassiveSuite5, test_case_55) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_55";
    ASSERT_FALSE(data.empty());
    int val = 55;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 55;
    j["name"] = "test_5_55";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 55 * 100;
    ASSERT_EQ(j["id"].get<int>(), 55);
    std::vector<int> vec;
    for (int k = 0; k < 55 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 55;
    ASSERT_EQ(m["key"], 55);
}

TEST_CASE(MassiveSuite5, test_case_56) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_56";
    ASSERT_FALSE(data.empty());
    int val = 56;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 56;
    j["name"] = "test_5_56";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 56 * 100;
    ASSERT_EQ(j["id"].get<int>(), 56);
    std::vector<int> vec;
    for (int k = 0; k < 56 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 56;
    ASSERT_EQ(m["key"], 56);
}

TEST_CASE(MassiveSuite5, test_case_57) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_57";
    ASSERT_FALSE(data.empty());
    int val = 57;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 57;
    j["name"] = "test_5_57";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 57 * 100;
    ASSERT_EQ(j["id"].get<int>(), 57);
    std::vector<int> vec;
    for (int k = 0; k < 57 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 57;
    ASSERT_EQ(m["key"], 57);
}

TEST_CASE(MassiveSuite5, test_case_58) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_58";
    ASSERT_FALSE(data.empty());
    int val = 58;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 58;
    j["name"] = "test_5_58";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 58 * 100;
    ASSERT_EQ(j["id"].get<int>(), 58);
    std::vector<int> vec;
    for (int k = 0; k < 58 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 58;
    ASSERT_EQ(m["key"], 58);
}

TEST_CASE(MassiveSuite5, test_case_59) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_59";
    ASSERT_FALSE(data.empty());
    int val = 59;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 59;
    j["name"] = "test_5_59";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 59 * 100;
    ASSERT_EQ(j["id"].get<int>(), 59);
    std::vector<int> vec;
    for (int k = 0; k < 59 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 59;
    ASSERT_EQ(m["key"], 59);
}

TEST_CASE(MassiveSuite5, test_case_60) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_60";
    ASSERT_FALSE(data.empty());
    int val = 60;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 60;
    j["name"] = "test_5_60";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 60 * 100;
    ASSERT_EQ(j["id"].get<int>(), 60);
    std::vector<int> vec;
    for (int k = 0; k < 60 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 60;
    ASSERT_EQ(m["key"], 60);
}

TEST_CASE(MassiveSuite5, test_case_61) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_61";
    ASSERT_FALSE(data.empty());
    int val = 61;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 61;
    j["name"] = "test_5_61";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 61 * 100;
    ASSERT_EQ(j["id"].get<int>(), 61);
    std::vector<int> vec;
    for (int k = 0; k < 61 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 61;
    ASSERT_EQ(m["key"], 61);
}

TEST_CASE(MassiveSuite5, test_case_62) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_62";
    ASSERT_FALSE(data.empty());
    int val = 62;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 62;
    j["name"] = "test_5_62";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 62 * 100;
    ASSERT_EQ(j["id"].get<int>(), 62);
    std::vector<int> vec;
    for (int k = 0; k < 62 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 62;
    ASSERT_EQ(m["key"], 62);
}

TEST_CASE(MassiveSuite5, test_case_63) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_63";
    ASSERT_FALSE(data.empty());
    int val = 63;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 63;
    j["name"] = "test_5_63";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 63 * 100;
    ASSERT_EQ(j["id"].get<int>(), 63);
    std::vector<int> vec;
    for (int k = 0; k < 63 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 63;
    ASSERT_EQ(m["key"], 63);
}

TEST_CASE(MassiveSuite5, test_case_64) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_64";
    ASSERT_FALSE(data.empty());
    int val = 64;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 64;
    j["name"] = "test_5_64";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 64 * 100;
    ASSERT_EQ(j["id"].get<int>(), 64);
    std::vector<int> vec;
    for (int k = 0; k < 64 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 64;
    ASSERT_EQ(m["key"], 64);
}

TEST_CASE(MassiveSuite5, test_case_65) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_65";
    ASSERT_FALSE(data.empty());
    int val = 65;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 65;
    j["name"] = "test_5_65";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 65 * 100;
    ASSERT_EQ(j["id"].get<int>(), 65);
    std::vector<int> vec;
    for (int k = 0; k < 65 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 65;
    ASSERT_EQ(m["key"], 65);
}

TEST_CASE(MassiveSuite5, test_case_66) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_66";
    ASSERT_FALSE(data.empty());
    int val = 66;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 66;
    j["name"] = "test_5_66";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 66 * 100;
    ASSERT_EQ(j["id"].get<int>(), 66);
    std::vector<int> vec;
    for (int k = 0; k < 66 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 66;
    ASSERT_EQ(m["key"], 66);
}

TEST_CASE(MassiveSuite5, test_case_67) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_67";
    ASSERT_FALSE(data.empty());
    int val = 67;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 67;
    j["name"] = "test_5_67";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 67 * 100;
    ASSERT_EQ(j["id"].get<int>(), 67);
    std::vector<int> vec;
    for (int k = 0; k < 67 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 67;
    ASSERT_EQ(m["key"], 67);
}

TEST_CASE(MassiveSuite5, test_case_68) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_68";
    ASSERT_FALSE(data.empty());
    int val = 68;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 68;
    j["name"] = "test_5_68";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 68 * 100;
    ASSERT_EQ(j["id"].get<int>(), 68);
    std::vector<int> vec;
    for (int k = 0; k < 68 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 68;
    ASSERT_EQ(m["key"], 68);
}

TEST_CASE(MassiveSuite5, test_case_69) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_69";
    ASSERT_FALSE(data.empty());
    int val = 69;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 69;
    j["name"] = "test_5_69";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 69 * 100;
    ASSERT_EQ(j["id"].get<int>(), 69);
    std::vector<int> vec;
    for (int k = 0; k < 69 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 69;
    ASSERT_EQ(m["key"], 69);
}

TEST_CASE(MassiveSuite5, test_case_70) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_70";
    ASSERT_FALSE(data.empty());
    int val = 70;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 70;
    j["name"] = "test_5_70";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 70 * 100;
    ASSERT_EQ(j["id"].get<int>(), 70);
    std::vector<int> vec;
    for (int k = 0; k < 70 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 70;
    ASSERT_EQ(m["key"], 70);
}

TEST_CASE(MassiveSuite5, test_case_71) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_71";
    ASSERT_FALSE(data.empty());
    int val = 71;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 71;
    j["name"] = "test_5_71";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 71 * 100;
    ASSERT_EQ(j["id"].get<int>(), 71);
    std::vector<int> vec;
    for (int k = 0; k < 71 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 71;
    ASSERT_EQ(m["key"], 71);
}

TEST_CASE(MassiveSuite5, test_case_72) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_72";
    ASSERT_FALSE(data.empty());
    int val = 72;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 72;
    j["name"] = "test_5_72";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 72 * 100;
    ASSERT_EQ(j["id"].get<int>(), 72);
    std::vector<int> vec;
    for (int k = 0; k < 72 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 72;
    ASSERT_EQ(m["key"], 72);
}

TEST_CASE(MassiveSuite5, test_case_73) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_73";
    ASSERT_FALSE(data.empty());
    int val = 73;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 73;
    j["name"] = "test_5_73";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 73 * 100;
    ASSERT_EQ(j["id"].get<int>(), 73);
    std::vector<int> vec;
    for (int k = 0; k < 73 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 73;
    ASSERT_EQ(m["key"], 73);
}

TEST_CASE(MassiveSuite5, test_case_74) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_74";
    ASSERT_FALSE(data.empty());
    int val = 74;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 74;
    j["name"] = "test_5_74";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 74 * 100;
    ASSERT_EQ(j["id"].get<int>(), 74);
    std::vector<int> vec;
    for (int k = 0; k < 74 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 74;
    ASSERT_EQ(m["key"], 74);
}

TEST_CASE(MassiveSuite5, test_case_75) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_75";
    ASSERT_FALSE(data.empty());
    int val = 75;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 75;
    j["name"] = "test_5_75";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 75 * 100;
    ASSERT_EQ(j["id"].get<int>(), 75);
    std::vector<int> vec;
    for (int k = 0; k < 75 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 75;
    ASSERT_EQ(m["key"], 75);
}

TEST_CASE(MassiveSuite5, test_case_76) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_76";
    ASSERT_FALSE(data.empty());
    int val = 76;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 76;
    j["name"] = "test_5_76";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 76 * 100;
    ASSERT_EQ(j["id"].get<int>(), 76);
    std::vector<int> vec;
    for (int k = 0; k < 76 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 76;
    ASSERT_EQ(m["key"], 76);
}

TEST_CASE(MassiveSuite5, test_case_77) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_77";
    ASSERT_FALSE(data.empty());
    int val = 77;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 77;
    j["name"] = "test_5_77";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 77 * 100;
    ASSERT_EQ(j["id"].get<int>(), 77);
    std::vector<int> vec;
    for (int k = 0; k < 77 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 77;
    ASSERT_EQ(m["key"], 77);
}

TEST_CASE(MassiveSuite5, test_case_78) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_78";
    ASSERT_FALSE(data.empty());
    int val = 78;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 78;
    j["name"] = "test_5_78";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 78 * 100;
    ASSERT_EQ(j["id"].get<int>(), 78);
    std::vector<int> vec;
    for (int k = 0; k < 78 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 78;
    ASSERT_EQ(m["key"], 78);
}

TEST_CASE(MassiveSuite5, test_case_79) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_79";
    ASSERT_FALSE(data.empty());
    int val = 79;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 79;
    j["name"] = "test_5_79";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 79 * 100;
    ASSERT_EQ(j["id"].get<int>(), 79);
    std::vector<int> vec;
    for (int k = 0; k < 79 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 79;
    ASSERT_EQ(m["key"], 79);
}

TEST_CASE(MassiveSuite5, test_case_80) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_80";
    ASSERT_FALSE(data.empty());
    int val = 80;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 80;
    j["name"] = "test_5_80";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 80 * 100;
    ASSERT_EQ(j["id"].get<int>(), 80);
    std::vector<int> vec;
    for (int k = 0; k < 80 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 80;
    ASSERT_EQ(m["key"], 80);
}

TEST_CASE(MassiveSuite5, test_case_81) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_81";
    ASSERT_FALSE(data.empty());
    int val = 81;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 81;
    j["name"] = "test_5_81";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 81 * 100;
    ASSERT_EQ(j["id"].get<int>(), 81);
    std::vector<int> vec;
    for (int k = 0; k < 81 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 81;
    ASSERT_EQ(m["key"], 81);
}

TEST_CASE(MassiveSuite5, test_case_82) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_82";
    ASSERT_FALSE(data.empty());
    int val = 82;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 82;
    j["name"] = "test_5_82";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 82 * 100;
    ASSERT_EQ(j["id"].get<int>(), 82);
    std::vector<int> vec;
    for (int k = 0; k < 82 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 82;
    ASSERT_EQ(m["key"], 82);
}

TEST_CASE(MassiveSuite5, test_case_83) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_83";
    ASSERT_FALSE(data.empty());
    int val = 83;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 83;
    j["name"] = "test_5_83";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 83 * 100;
    ASSERT_EQ(j["id"].get<int>(), 83);
    std::vector<int> vec;
    for (int k = 0; k < 83 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 83;
    ASSERT_EQ(m["key"], 83);
}

TEST_CASE(MassiveSuite5, test_case_84) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_84";
    ASSERT_FALSE(data.empty());
    int val = 84;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 84;
    j["name"] = "test_5_84";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 84 * 100;
    ASSERT_EQ(j["id"].get<int>(), 84);
    std::vector<int> vec;
    for (int k = 0; k < 84 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 84;
    ASSERT_EQ(m["key"], 84);
}

TEST_CASE(MassiveSuite5, test_case_85) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_85";
    ASSERT_FALSE(data.empty());
    int val = 85;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 85;
    j["name"] = "test_5_85";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 85 * 100;
    ASSERT_EQ(j["id"].get<int>(), 85);
    std::vector<int> vec;
    for (int k = 0; k < 85 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 85;
    ASSERT_EQ(m["key"], 85);
}

TEST_CASE(MassiveSuite5, test_case_86) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_86";
    ASSERT_FALSE(data.empty());
    int val = 86;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 86;
    j["name"] = "test_5_86";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 86 * 100;
    ASSERT_EQ(j["id"].get<int>(), 86);
    std::vector<int> vec;
    for (int k = 0; k < 86 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 86;
    ASSERT_EQ(m["key"], 86);
}

TEST_CASE(MassiveSuite5, test_case_87) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_87";
    ASSERT_FALSE(data.empty());
    int val = 87;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 87;
    j["name"] = "test_5_87";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 87 * 100;
    ASSERT_EQ(j["id"].get<int>(), 87);
    std::vector<int> vec;
    for (int k = 0; k < 87 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 87;
    ASSERT_EQ(m["key"], 87);
}

TEST_CASE(MassiveSuite5, test_case_88) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_88";
    ASSERT_FALSE(data.empty());
    int val = 88;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 88;
    j["name"] = "test_5_88";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 88 * 100;
    ASSERT_EQ(j["id"].get<int>(), 88);
    std::vector<int> vec;
    for (int k = 0; k < 88 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 88;
    ASSERT_EQ(m["key"], 88);
}

TEST_CASE(MassiveSuite5, test_case_89) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_89";
    ASSERT_FALSE(data.empty());
    int val = 89;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 89;
    j["name"] = "test_5_89";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 89 * 100;
    ASSERT_EQ(j["id"].get<int>(), 89);
    std::vector<int> vec;
    for (int k = 0; k < 89 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 89;
    ASSERT_EQ(m["key"], 89);
}

TEST_CASE(MassiveSuite5, test_case_90) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_90";
    ASSERT_FALSE(data.empty());
    int val = 90;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 90;
    j["name"] = "test_5_90";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 90 * 100;
    ASSERT_EQ(j["id"].get<int>(), 90);
    std::vector<int> vec;
    for (int k = 0; k < 90 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 90;
    ASSERT_EQ(m["key"], 90);
}

TEST_CASE(MassiveSuite5, test_case_91) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_91";
    ASSERT_FALSE(data.empty());
    int val = 91;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 91;
    j["name"] = "test_5_91";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 91 * 100;
    ASSERT_EQ(j["id"].get<int>(), 91);
    std::vector<int> vec;
    for (int k = 0; k < 91 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 91;
    ASSERT_EQ(m["key"], 91);
}

TEST_CASE(MassiveSuite5, test_case_92) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_92";
    ASSERT_FALSE(data.empty());
    int val = 92;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 92;
    j["name"] = "test_5_92";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 92 * 100;
    ASSERT_EQ(j["id"].get<int>(), 92);
    std::vector<int> vec;
    for (int k = 0; k < 92 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 92;
    ASSERT_EQ(m["key"], 92);
}

TEST_CASE(MassiveSuite5, test_case_93) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_93";
    ASSERT_FALSE(data.empty());
    int val = 93;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 93;
    j["name"] = "test_5_93";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 93 * 100;
    ASSERT_EQ(j["id"].get<int>(), 93);
    std::vector<int> vec;
    for (int k = 0; k < 93 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 93;
    ASSERT_EQ(m["key"], 93);
}

TEST_CASE(MassiveSuite5, test_case_94) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_94";
    ASSERT_FALSE(data.empty());
    int val = 94;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 94;
    j["name"] = "test_5_94";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 94 * 100;
    ASSERT_EQ(j["id"].get<int>(), 94);
    std::vector<int> vec;
    for (int k = 0; k < 94 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 94;
    ASSERT_EQ(m["key"], 94);
}

TEST_CASE(MassiveSuite5, test_case_95) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_95";
    ASSERT_FALSE(data.empty());
    int val = 95;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 95;
    j["name"] = "test_5_95";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 95 * 100;
    ASSERT_EQ(j["id"].get<int>(), 95);
    std::vector<int> vec;
    for (int k = 0; k < 95 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 95;
    ASSERT_EQ(m["key"], 95);
}

TEST_CASE(MassiveSuite5, test_case_96) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_96";
    ASSERT_FALSE(data.empty());
    int val = 96;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 96;
    j["name"] = "test_5_96";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 96 * 100;
    ASSERT_EQ(j["id"].get<int>(), 96);
    std::vector<int> vec;
    for (int k = 0; k < 96 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 96;
    ASSERT_EQ(m["key"], 96);
}

TEST_CASE(MassiveSuite5, test_case_97) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_97";
    ASSERT_FALSE(data.empty());
    int val = 97;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 97;
    j["name"] = "test_5_97";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 97 * 100;
    ASSERT_EQ(j["id"].get<int>(), 97);
    std::vector<int> vec;
    for (int k = 0; k < 97 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 97;
    ASSERT_EQ(m["key"], 97);
}

TEST_CASE(MassiveSuite5, test_case_98) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_98";
    ASSERT_FALSE(data.empty());
    int val = 98;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 98;
    j["name"] = "test_5_98";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 98 * 100;
    ASSERT_EQ(j["id"].get<int>(), 98);
    std::vector<int> vec;
    for (int k = 0; k < 98 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 98;
    ASSERT_EQ(m["key"], 98);
}

TEST_CASE(MassiveSuite5, test_case_99) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_99";
    ASSERT_FALSE(data.empty());
    int val = 99;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 99;
    j["name"] = "test_5_99";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 99 * 100;
    ASSERT_EQ(j["id"].get<int>(), 99);
    std::vector<int> vec;
    for (int k = 0; k < 99 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 99;
    ASSERT_EQ(m["key"], 99);
}

TEST_CASE(MassiveSuite5, test_case_100) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_100";
    ASSERT_FALSE(data.empty());
    int val = 100;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 100;
    j["name"] = "test_5_100";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 100 * 100;
    ASSERT_EQ(j["id"].get<int>(), 100);
    std::vector<int> vec;
    for (int k = 0; k < 100 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 100;
    ASSERT_EQ(m["key"], 100);
}

TEST_CASE(MassiveSuite5, test_case_101) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_101";
    ASSERT_FALSE(data.empty());
    int val = 101;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 101;
    j["name"] = "test_5_101";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 101 * 100;
    ASSERT_EQ(j["id"].get<int>(), 101);
    std::vector<int> vec;
    for (int k = 0; k < 101 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 101;
    ASSERT_EQ(m["key"], 101);
}

TEST_CASE(MassiveSuite5, test_case_102) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_102";
    ASSERT_FALSE(data.empty());
    int val = 102;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 102;
    j["name"] = "test_5_102";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 102 * 100;
    ASSERT_EQ(j["id"].get<int>(), 102);
    std::vector<int> vec;
    for (int k = 0; k < 102 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 102;
    ASSERT_EQ(m["key"], 102);
}

TEST_CASE(MassiveSuite5, test_case_103) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_103";
    ASSERT_FALSE(data.empty());
    int val = 103;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 103;
    j["name"] = "test_5_103";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 103 * 100;
    ASSERT_EQ(j["id"].get<int>(), 103);
    std::vector<int> vec;
    for (int k = 0; k < 103 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 103;
    ASSERT_EQ(m["key"], 103);
}

TEST_CASE(MassiveSuite5, test_case_104) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_104";
    ASSERT_FALSE(data.empty());
    int val = 104;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 104;
    j["name"] = "test_5_104";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 104 * 100;
    ASSERT_EQ(j["id"].get<int>(), 104);
    std::vector<int> vec;
    for (int k = 0; k < 104 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 104;
    ASSERT_EQ(m["key"], 104);
}

TEST_CASE(MassiveSuite5, test_case_105) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_105";
    ASSERT_FALSE(data.empty());
    int val = 105;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 105;
    j["name"] = "test_5_105";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 105 * 100;
    ASSERT_EQ(j["id"].get<int>(), 105);
    std::vector<int> vec;
    for (int k = 0; k < 105 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 105;
    ASSERT_EQ(m["key"], 105);
}

TEST_CASE(MassiveSuite5, test_case_106) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_106";
    ASSERT_FALSE(data.empty());
    int val = 106;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 106;
    j["name"] = "test_5_106";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 106 * 100;
    ASSERT_EQ(j["id"].get<int>(), 106);
    std::vector<int> vec;
    for (int k = 0; k < 106 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 106;
    ASSERT_EQ(m["key"], 106);
}

TEST_CASE(MassiveSuite5, test_case_107) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_107";
    ASSERT_FALSE(data.empty());
    int val = 107;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 107;
    j["name"] = "test_5_107";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 107 * 100;
    ASSERT_EQ(j["id"].get<int>(), 107);
    std::vector<int> vec;
    for (int k = 0; k < 107 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 107;
    ASSERT_EQ(m["key"], 107);
}

TEST_CASE(MassiveSuite5, test_case_108) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_108";
    ASSERT_FALSE(data.empty());
    int val = 108;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 108;
    j["name"] = "test_5_108";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 108 * 100;
    ASSERT_EQ(j["id"].get<int>(), 108);
    std::vector<int> vec;
    for (int k = 0; k < 108 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 108;
    ASSERT_EQ(m["key"], 108);
}

TEST_CASE(MassiveSuite5, test_case_109) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_109";
    ASSERT_FALSE(data.empty());
    int val = 109;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 109;
    j["name"] = "test_5_109";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 109 * 100;
    ASSERT_EQ(j["id"].get<int>(), 109);
    std::vector<int> vec;
    for (int k = 0; k < 109 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 109;
    ASSERT_EQ(m["key"], 109);
}

TEST_CASE(MassiveSuite5, test_case_110) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_110";
    ASSERT_FALSE(data.empty());
    int val = 110;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 110;
    j["name"] = "test_5_110";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 110 * 100;
    ASSERT_EQ(j["id"].get<int>(), 110);
    std::vector<int> vec;
    for (int k = 0; k < 110 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 110;
    ASSERT_EQ(m["key"], 110);
}

TEST_CASE(MassiveSuite5, test_case_111) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_111";
    ASSERT_FALSE(data.empty());
    int val = 111;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 111;
    j["name"] = "test_5_111";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 111 * 100;
    ASSERT_EQ(j["id"].get<int>(), 111);
    std::vector<int> vec;
    for (int k = 0; k < 111 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 111;
    ASSERT_EQ(m["key"], 111);
}

TEST_CASE(MassiveSuite5, test_case_112) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_112";
    ASSERT_FALSE(data.empty());
    int val = 112;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 112;
    j["name"] = "test_5_112";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 112 * 100;
    ASSERT_EQ(j["id"].get<int>(), 112);
    std::vector<int> vec;
    for (int k = 0; k < 112 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 112;
    ASSERT_EQ(m["key"], 112);
}

TEST_CASE(MassiveSuite5, test_case_113) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_113";
    ASSERT_FALSE(data.empty());
    int val = 113;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 113;
    j["name"] = "test_5_113";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 113 * 100;
    ASSERT_EQ(j["id"].get<int>(), 113);
    std::vector<int> vec;
    for (int k = 0; k < 113 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 113;
    ASSERT_EQ(m["key"], 113);
}

TEST_CASE(MassiveSuite5, test_case_114) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_114";
    ASSERT_FALSE(data.empty());
    int val = 114;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 114;
    j["name"] = "test_5_114";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 114 * 100;
    ASSERT_EQ(j["id"].get<int>(), 114);
    std::vector<int> vec;
    for (int k = 0; k < 114 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 114;
    ASSERT_EQ(m["key"], 114);
}

TEST_CASE(MassiveSuite5, test_case_115) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_115";
    ASSERT_FALSE(data.empty());
    int val = 115;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 115;
    j["name"] = "test_5_115";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 115 * 100;
    ASSERT_EQ(j["id"].get<int>(), 115);
    std::vector<int> vec;
    for (int k = 0; k < 115 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 115;
    ASSERT_EQ(m["key"], 115);
}

TEST_CASE(MassiveSuite5, test_case_116) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_116";
    ASSERT_FALSE(data.empty());
    int val = 116;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 116;
    j["name"] = "test_5_116";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 116 * 100;
    ASSERT_EQ(j["id"].get<int>(), 116);
    std::vector<int> vec;
    for (int k = 0; k < 116 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 116;
    ASSERT_EQ(m["key"], 116);
}

TEST_CASE(MassiveSuite5, test_case_117) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_117";
    ASSERT_FALSE(data.empty());
    int val = 117;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 117;
    j["name"] = "test_5_117";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 117 * 100;
    ASSERT_EQ(j["id"].get<int>(), 117);
    std::vector<int> vec;
    for (int k = 0; k < 117 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 117;
    ASSERT_EQ(m["key"], 117);
}

TEST_CASE(MassiveSuite5, test_case_118) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_118";
    ASSERT_FALSE(data.empty());
    int val = 118;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 118;
    j["name"] = "test_5_118";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 118 * 100;
    ASSERT_EQ(j["id"].get<int>(), 118);
    std::vector<int> vec;
    for (int k = 0; k < 118 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 118;
    ASSERT_EQ(m["key"], 118);
}

TEST_CASE(MassiveSuite5, test_case_119) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_119";
    ASSERT_FALSE(data.empty());
    int val = 119;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 119;
    j["name"] = "test_5_119";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 119 * 100;
    ASSERT_EQ(j["id"].get<int>(), 119);
    std::vector<int> vec;
    for (int k = 0; k < 119 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 119;
    ASSERT_EQ(m["key"], 119);
}

TEST_CASE(MassiveSuite5, test_case_120) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_120";
    ASSERT_FALSE(data.empty());
    int val = 120;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 120;
    j["name"] = "test_5_120";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 120 * 100;
    ASSERT_EQ(j["id"].get<int>(), 120);
    std::vector<int> vec;
    for (int k = 0; k < 120 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 120;
    ASSERT_EQ(m["key"], 120);
}

TEST_CASE(MassiveSuite5, test_case_121) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_121";
    ASSERT_FALSE(data.empty());
    int val = 121;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 121;
    j["name"] = "test_5_121";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 121 * 100;
    ASSERT_EQ(j["id"].get<int>(), 121);
    std::vector<int> vec;
    for (int k = 0; k < 121 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 121;
    ASSERT_EQ(m["key"], 121);
}

TEST_CASE(MassiveSuite5, test_case_122) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_122";
    ASSERT_FALSE(data.empty());
    int val = 122;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 122;
    j["name"] = "test_5_122";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 122 * 100;
    ASSERT_EQ(j["id"].get<int>(), 122);
    std::vector<int> vec;
    for (int k = 0; k < 122 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 122;
    ASSERT_EQ(m["key"], 122);
}

TEST_CASE(MassiveSuite5, test_case_123) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_123";
    ASSERT_FALSE(data.empty());
    int val = 123;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 123;
    j["name"] = "test_5_123";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 123 * 100;
    ASSERT_EQ(j["id"].get<int>(), 123);
    std::vector<int> vec;
    for (int k = 0; k < 123 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 123;
    ASSERT_EQ(m["key"], 123);
}

TEST_CASE(MassiveSuite5, test_case_124) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_124";
    ASSERT_FALSE(data.empty());
    int val = 124;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 124;
    j["name"] = "test_5_124";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 124 * 100;
    ASSERT_EQ(j["id"].get<int>(), 124);
    std::vector<int> vec;
    for (int k = 0; k < 124 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 124;
    ASSERT_EQ(m["key"], 124);
}

TEST_CASE(MassiveSuite5, test_case_125) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_125";
    ASSERT_FALSE(data.empty());
    int val = 125;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 125;
    j["name"] = "test_5_125";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 125 * 100;
    ASSERT_EQ(j["id"].get<int>(), 125);
    std::vector<int> vec;
    for (int k = 0; k < 125 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 125;
    ASSERT_EQ(m["key"], 125);
}

TEST_CASE(MassiveSuite5, test_case_126) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_126";
    ASSERT_FALSE(data.empty());
    int val = 126;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 126;
    j["name"] = "test_5_126";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 126 * 100;
    ASSERT_EQ(j["id"].get<int>(), 126);
    std::vector<int> vec;
    for (int k = 0; k < 126 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 126;
    ASSERT_EQ(m["key"], 126);
}

TEST_CASE(MassiveSuite5, test_case_127) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_127";
    ASSERT_FALSE(data.empty());
    int val = 127;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 127;
    j["name"] = "test_5_127";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 127 * 100;
    ASSERT_EQ(j["id"].get<int>(), 127);
    std::vector<int> vec;
    for (int k = 0; k < 127 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 127;
    ASSERT_EQ(m["key"], 127);
}

TEST_CASE(MassiveSuite5, test_case_128) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_128";
    ASSERT_FALSE(data.empty());
    int val = 128;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 128;
    j["name"] = "test_5_128";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 128 * 100;
    ASSERT_EQ(j["id"].get<int>(), 128);
    std::vector<int> vec;
    for (int k = 0; k < 128 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 128;
    ASSERT_EQ(m["key"], 128);
}

TEST_CASE(MassiveSuite5, test_case_129) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_129";
    ASSERT_FALSE(data.empty());
    int val = 129;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 129;
    j["name"] = "test_5_129";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 129 * 100;
    ASSERT_EQ(j["id"].get<int>(), 129);
    std::vector<int> vec;
    for (int k = 0; k < 129 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 129;
    ASSERT_EQ(m["key"], 129);
}

TEST_CASE(MassiveSuite5, test_case_130) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_130";
    ASSERT_FALSE(data.empty());
    int val = 130;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 130;
    j["name"] = "test_5_130";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 130 * 100;
    ASSERT_EQ(j["id"].get<int>(), 130);
    std::vector<int> vec;
    for (int k = 0; k < 130 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 130;
    ASSERT_EQ(m["key"], 130);
}

TEST_CASE(MassiveSuite5, test_case_131) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_131";
    ASSERT_FALSE(data.empty());
    int val = 131;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 131;
    j["name"] = "test_5_131";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 131 * 100;
    ASSERT_EQ(j["id"].get<int>(), 131);
    std::vector<int> vec;
    for (int k = 0; k < 131 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 131;
    ASSERT_EQ(m["key"], 131);
}

TEST_CASE(MassiveSuite5, test_case_132) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_132";
    ASSERT_FALSE(data.empty());
    int val = 132;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 132;
    j["name"] = "test_5_132";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 132 * 100;
    ASSERT_EQ(j["id"].get<int>(), 132);
    std::vector<int> vec;
    for (int k = 0; k < 132 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 132;
    ASSERT_EQ(m["key"], 132);
}

TEST_CASE(MassiveSuite5, test_case_133) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_133";
    ASSERT_FALSE(data.empty());
    int val = 133;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 133;
    j["name"] = "test_5_133";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 133 * 100;
    ASSERT_EQ(j["id"].get<int>(), 133);
    std::vector<int> vec;
    for (int k = 0; k < 133 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 133;
    ASSERT_EQ(m["key"], 133);
}

TEST_CASE(MassiveSuite5, test_case_134) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_134";
    ASSERT_FALSE(data.empty());
    int val = 134;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 134;
    j["name"] = "test_5_134";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 134 * 100;
    ASSERT_EQ(j["id"].get<int>(), 134);
    std::vector<int> vec;
    for (int k = 0; k < 134 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 134;
    ASSERT_EQ(m["key"], 134);
}

TEST_CASE(MassiveSuite5, test_case_135) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_135";
    ASSERT_FALSE(data.empty());
    int val = 135;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 135;
    j["name"] = "test_5_135";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 135 * 100;
    ASSERT_EQ(j["id"].get<int>(), 135);
    std::vector<int> vec;
    for (int k = 0; k < 135 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 135;
    ASSERT_EQ(m["key"], 135);
}

TEST_CASE(MassiveSuite5, test_case_136) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_136";
    ASSERT_FALSE(data.empty());
    int val = 136;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 136;
    j["name"] = "test_5_136";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 136 * 100;
    ASSERT_EQ(j["id"].get<int>(), 136);
    std::vector<int> vec;
    for (int k = 0; k < 136 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 136;
    ASSERT_EQ(m["key"], 136);
}

TEST_CASE(MassiveSuite5, test_case_137) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_137";
    ASSERT_FALSE(data.empty());
    int val = 137;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 137;
    j["name"] = "test_5_137";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 137 * 100;
    ASSERT_EQ(j["id"].get<int>(), 137);
    std::vector<int> vec;
    for (int k = 0; k < 137 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 137;
    ASSERT_EQ(m["key"], 137);
}

TEST_CASE(MassiveSuite5, test_case_138) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_138";
    ASSERT_FALSE(data.empty());
    int val = 138;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 138;
    j["name"] = "test_5_138";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 138 * 100;
    ASSERT_EQ(j["id"].get<int>(), 138);
    std::vector<int> vec;
    for (int k = 0; k < 138 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 138;
    ASSERT_EQ(m["key"], 138);
}

TEST_CASE(MassiveSuite5, test_case_139) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_139";
    ASSERT_FALSE(data.empty());
    int val = 139;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 139;
    j["name"] = "test_5_139";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 139 * 100;
    ASSERT_EQ(j["id"].get<int>(), 139);
    std::vector<int> vec;
    for (int k = 0; k < 139 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 139;
    ASSERT_EQ(m["key"], 139);
}

TEST_CASE(MassiveSuite5, test_case_140) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_140";
    ASSERT_FALSE(data.empty());
    int val = 140;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 140;
    j["name"] = "test_5_140";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 140 * 100;
    ASSERT_EQ(j["id"].get<int>(), 140);
    std::vector<int> vec;
    for (int k = 0; k < 140 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 140;
    ASSERT_EQ(m["key"], 140);
}

TEST_CASE(MassiveSuite5, test_case_141) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_141";
    ASSERT_FALSE(data.empty());
    int val = 141;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 141;
    j["name"] = "test_5_141";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 141 * 100;
    ASSERT_EQ(j["id"].get<int>(), 141);
    std::vector<int> vec;
    for (int k = 0; k < 141 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 141;
    ASSERT_EQ(m["key"], 141);
}

TEST_CASE(MassiveSuite5, test_case_142) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_142";
    ASSERT_FALSE(data.empty());
    int val = 142;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 142;
    j["name"] = "test_5_142";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 142 * 100;
    ASSERT_EQ(j["id"].get<int>(), 142);
    std::vector<int> vec;
    for (int k = 0; k < 142 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 142;
    ASSERT_EQ(m["key"], 142);
}

TEST_CASE(MassiveSuite5, test_case_143) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_143";
    ASSERT_FALSE(data.empty());
    int val = 143;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 143;
    j["name"] = "test_5_143";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 143 * 100;
    ASSERT_EQ(j["id"].get<int>(), 143);
    std::vector<int> vec;
    for (int k = 0; k < 143 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 143;
    ASSERT_EQ(m["key"], 143);
}

TEST_CASE(MassiveSuite5, test_case_144) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_144";
    ASSERT_FALSE(data.empty());
    int val = 144;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 144;
    j["name"] = "test_5_144";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 144 * 100;
    ASSERT_EQ(j["id"].get<int>(), 144);
    std::vector<int> vec;
    for (int k = 0; k < 144 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 144;
    ASSERT_EQ(m["key"], 144);
}

TEST_CASE(MassiveSuite5, test_case_145) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_145";
    ASSERT_FALSE(data.empty());
    int val = 145;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 145;
    j["name"] = "test_5_145";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 145 * 100;
    ASSERT_EQ(j["id"].get<int>(), 145);
    std::vector<int> vec;
    for (int k = 0; k < 145 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 145;
    ASSERT_EQ(m["key"], 145);
}

TEST_CASE(MassiveSuite5, test_case_146) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_146";
    ASSERT_FALSE(data.empty());
    int val = 146;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 146;
    j["name"] = "test_5_146";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 146 * 100;
    ASSERT_EQ(j["id"].get<int>(), 146);
    std::vector<int> vec;
    for (int k = 0; k < 146 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 146;
    ASSERT_EQ(m["key"], 146);
}

TEST_CASE(MassiveSuite5, test_case_147) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_147";
    ASSERT_FALSE(data.empty());
    int val = 147;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 147;
    j["name"] = "test_5_147";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 147 * 100;
    ASSERT_EQ(j["id"].get<int>(), 147);
    std::vector<int> vec;
    for (int k = 0; k < 147 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 147;
    ASSERT_EQ(m["key"], 147);
}

TEST_CASE(MassiveSuite5, test_case_148) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_148";
    ASSERT_FALSE(data.empty());
    int val = 148;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 148;
    j["name"] = "test_5_148";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 148 * 100;
    ASSERT_EQ(j["id"].get<int>(), 148);
    std::vector<int> vec;
    for (int k = 0; k < 148 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 148;
    ASSERT_EQ(m["key"], 148);
}

TEST_CASE(MassiveSuite5, test_case_149) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_149";
    ASSERT_FALSE(data.empty());
    int val = 149;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 149;
    j["name"] = "test_5_149";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 149 * 100;
    ASSERT_EQ(j["id"].get<int>(), 149);
    std::vector<int> vec;
    for (int k = 0; k < 149 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 149;
    ASSERT_EQ(m["key"], 149);
}

TEST_CASE(MassiveSuite5, test_case_150) {
    ASSERT_TRUE(true);
    std::string data = "massive_test_5_150";
    ASSERT_FALSE(data.empty());
    int val = 150;
    ASSERT_TRUE(val > 0 || val == 0);
    json j;
    j["id"] = 150;
    j["name"] = "test_5_150";
    j["timestamp"] = 1600000000000LL + 5 * 100000 + 150 * 100;
    ASSERT_EQ(j["id"].get<int>(), 150);
    std::vector<int> vec;
    for (int k = 0; k < 150 % 10 + 1; k++) vec.push_back(k);
    ASSERT_TRUE(vec.size() > 0);
    std::map<std::string, int> m;
    m["key"] = 150;
    ASSERT_EQ(m["key"], 150);
}
}
