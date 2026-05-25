#include "progressive/eventcache.hpp"
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

} // namespace progressive
