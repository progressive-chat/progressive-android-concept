#include "progressive/push_rules.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <mutex>

namespace progressive {
using json = nlohmann::json;

namespace {
    std::mutex g_mutex;
    bool g_initialized = false;
} // anonymous namespace

PushRulesStatus push_rules_getStatus() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_status;
}

} // namespace progressive
