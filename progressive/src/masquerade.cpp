#include "progressive/masquerade.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <mutex>

namespace progressive {
using json = nlohmann::json;

namespace {
    MasqueradeConfig g_config;
    std::mutex g_mutex;
    bool g_initialized = false;
} // anonymous namespace

} // namespace progressive
