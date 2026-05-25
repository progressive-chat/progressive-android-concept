#include "progressive/auth_models.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <mutex>

namespace progressive {
using json = nlohmann::json;

namespace {
    AuthModelsConfig g_config;
    std::mutex g_mutex;
    AuthModelsStatus g_status;
    bool g_initialized = false;
} // anonymous namespace

} // namespace progressive
