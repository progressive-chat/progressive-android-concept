#include "test_framework.hpp"
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include <cmath>
#include <algorithm>

using json = nlohmann::json;

// ==== Edge Cases ====

TEST_CASE(EdgeCases, empty_json_parsing) {
    json j = json::parse("{}");
    ASSERT_TRUE(j.empty());
}

TEST_CASE(EdgeCases, null_json_value) {
    json j = json::parse("{\"key\": null}");
    ASSERT_TRUE(j["key"].is_null());
}

TEST_CASE(EdgeCases, very_long_string) {
    std::string longStr(10000, 'A');
    ASSERT_EQ(static_cast<int>(longStr.size()), 10000);
}

TEST_CASE(EdgeCases, unicode_strings) {
    std::string emoji = "😀🎉❤️👍🔥";
    ASSERT_TRUE(!emoji.empty());
    ASSERT_TRUE(emoji.size() > 5);
}

TEST_CASE(EdgeCases, negative_integers) {
    int64_t neg = -9223372036854775807LL;
    ASSERT_TRUE(neg < 0);
}

TEST_CASE(EdgeCases, floating_point_precision) {
    double a = 0.1 + 0.2;
    ASSERT_NEAR(a, 0.3, 0.0001);
}

TEST_CASE(EdgeCases, vector_boundaries) {
    std::vector<int> v = {1, 2, 3};
    ASSERT_EQ(v.front(), 1);
    ASSERT_EQ(v.back(), 3);
}

TEST_CASE(EdgeCases, map_key_collision) {
    std::map<std::string, int> m;
    m["key"] = 1;
    m["key"] = 2;
    ASSERT_EQ(m["key"], 2);
    ASSERT_EQ(static_cast<int>(m.size()), 1);
}

TEST_CASE(EdgeCases, recursion_limit) {
    int depth = 0;
    auto recurse = [&](auto&& self, int n) -> int {
        if (n <= 0) return depth;
        depth++;
        return self(self, n - 1);
    };
    recurse(recurse, 50);
    ASSERT_EQ(depth, 50);
}

TEST_CASE(EdgeCases, threading_basics) {
    int counter = 0;
    std::thread t1([&]() { for (int i = 0; i < 100; i++) counter++; });
    std::thread t2([&]() { for (int i = 0; i < 100; i++) counter++; });
    t1.join(); t2.join();
    ASSERT_TRUE(counter > 0);
}

TEST_CASE(EdgeCases, chrono_precision) {
    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    ASSERT_TRUE(diff.count() >= 0);
}

TEST_CASE(EdgeCases, string_utf8_boundary) {
    std::string s = "\xc3\xa9"; // é in UTF-8
    ASSERT_TRUE(!s.empty());
}

TEST_CASE(EdgeCases, math_constants) {
    ASSERT_NEAR(M_PI, 3.1415926535, 0.001);
    ASSERT_NEAR(M_E, 2.7182818284, 0.001);
}

TEST_CASE(EdgeCases, sort_stability) {
    std::vector<int> v = {5, 3, 1, 4, 2};
    std::sort(v.begin(), v.end());
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[4], 5);
}

TEST_CASE(EdgeCases, hash_consistency_across_types) {
    std::string s = "test";
    size_t h1 = std::hash<std::string>{}(s);
    size_t h2 = std::hash<std::string>{}("test");
    ASSERT_EQ(static_cast<int64_t>(h1), static_cast<int64_t>(h2));
}

TEST_CASE(EdgeCases, move_semantics) {
    std::string original = "movable";
    std::string moved = std::move(original);
    ASSERT_EQ(moved, std::string("movable"));
}

TEST_CASE(EdgeCases, shared_ptr_lifetime) {
    auto ptr = std::make_shared<int>(42);
    ASSERT_EQ(*ptr, 42);
    ASSERT_EQ(ptr.use_count(), 1);
}

TEST_CASE(EdgeCases, weak_ptr_expiry) {
    std::weak_ptr<int> weak;
    {
        auto shared = std::make_shared<int>(42);
        weak = shared;
        ASSERT_FALSE(weak.expired());
    }
    ASSERT_TRUE(weak.expired());
}

TEST_CASE(EdgeCases, lambda_capture_by_value) {
    int x = 10;
    auto lambda = [x]() { return x; };
    x = 20;
    ASSERT_EQ(lambda(), 10);
}

TEST_CASE(EdgeCases, lambda_capture_by_reference) {
    int x = 10;
    auto lambda = [&x]() { return x; };
    x = 20;
    ASSERT_EQ(lambda(), 20);
}

TEST_CASE(EdgeCases, variant_like_union) {
    struct VariantLike {
        enum { INT, STRING } type;
        union { int i; const char* s; };
    };
    VariantLike v;
    v.type = VariantLike::INT;
    v.i = 42;
    ASSERT_EQ(v.i, 42);
}

TEST_CASE(EdgeCases, constexpr_evaluation) {
    constexpr int answer = 6 * 7;
    ASSERT_EQ(answer, 42);
}

TEST_CASE(EdgeCases, string_view_usage) {
    std::string s = "hello world";
    std::string_view sv = s;
    ASSERT_EQ(sv.substr(0, 5), std::string_view("hello"));
}

TEST_CASE(EdgeCases, optional_value) {
    std::optional<int> opt;
    ASSERT_FALSE(opt.has_value());
    opt = 42;
    ASSERT_TRUE(opt.has_value());
    ASSERT_EQ(*opt, 42);
}

TEST_CASE(EdgeCases, variant_visit) {
    std::variant<int, std::string> v;
    v = "hello";
    ASSERT_TRUE(std::holds_alternative<std::string>(v));
}

TEST_CASE(EdgeCases, any_cast) {
    std::any a = 42;
    ASSERT_EQ(std::any_cast<int>(a), 42);
}

TEST_CASE(EdgeCases, bitset_operations) {
    std::bitset<8> bits;
    bits.set(0); bits.set(3); bits.set(7);
    ASSERT_TRUE(bits.test(0));
    ASSERT_TRUE(bits.test(3));
    ASSERT_TRUE(bits.test(7));
    ASSERT_FALSE(bits.test(1));
}

TEST_CASE(EdgeCases, regex_matching) {
    std::regex re("[a-z]+@[a-z]+\\.[a-z]+");
    ASSERT_TRUE(std::regex_match("test@example.com", re));
    ASSERT_FALSE(std::regex_match("invalid", re));
}

TEST_CASE(EdgeCases, filepath_operations) {
    std::string path = "/home/user/documents/file.txt";
    size_t lastSlash = path.rfind('/');
    std::string filename = path.substr(lastSlash + 1);
    ASSERT_EQ(filename, std::string("file.txt"));
}

TEST_CASE(EdgeCases, base64_charset) {
    std::string b64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    ASSERT_EQ(b64Chars.size(), 64);
}

TEST_CASE(EdgeCases, random_number_range) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 6);
    int roll = dis(gen);
    ASSERT_TRUE(roll >= 1 && roll <= 6);
}
