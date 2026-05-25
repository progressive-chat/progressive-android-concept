#include "progressive/event_encryption_helper.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <mutex>

namespace progressive {
using json = nlohmann::json;

namespace {
    std::mutex g_mutex;
    EventEncryptionHelperStatus g_status;
    bool g_initialized = false;
} // anonymous namespace

EventEncryptionHelperStatus event_encryption_helper_getStatus() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_status;
}

} // namespace progressive
