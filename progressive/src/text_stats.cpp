#include "progressive/text_stats.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <mutex>

namespace progressive {
using json = nlohmann::json;

namespace {
    std::mutex g_mutex;
    TextStatsStats g_stats;
    bool g_initialized = false;
} // anonymous namespace

TextStatsStats text_stats_getStats() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_stats;
}

} // namespace progressive
