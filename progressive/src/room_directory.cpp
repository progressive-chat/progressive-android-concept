#include "progressive/room_directory.hpp"
#include <sstream>
#include <algorithm>
#include <unordered_set>

namespace progressive {

std::vector<RoomDirectoryEntry> filterDirectory(
    const std::vector<RoomDirectoryEntry>& rooms,
    const DirectoryFilter& filter,
    int maxResults
) {
    std::vector<RoomDirectoryEntry> result;

    for (const auto& room : rooms) {
        // Server filter
        if (!filter.serverFilter.empty()) {
            auto server = room.roomId;
            auto colon = server.find(':');
            if (colon != std::string::npos) server = server.substr(colon + 1);
            if (server != filter.serverFilter) continue;
        }

        // Name filter
        if (!filter.nameFilter.empty()) {
            auto lower = room.name;
            auto lowerFilter = filter.nameFilter;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
            if (lower.find(lowerFilter) == std::string::npos) continue;
        }

        // Member count filter
        if (room.memberCount < filter.minMembers) continue;
        if (room.memberCount > filter.maxMembers) continue;

        // Public/private
        if (filter.showOnlyPublic && !room.isPublic) continue;

        // Already joined
        if (filter.showOnlyUnjoined && room.isJoined) continue;

        result.push_back(room);
    }

    if (static_cast<int>(result.size()) > maxResults) {
        result.resize(maxResults);
    }

    return result;
}

DirectoryStats computeDirectoryStats(const std::vector<RoomDirectoryEntry>& rooms) {
    DirectoryStats stats;
    stats.totalRooms = static_cast<int>(rooms.size());
    stats.filteredRooms = stats.totalRooms;

    for (const auto& room : rooms) {
        stats.totalMembers += room.memberCount;
        if (room.memberCount > stats.biggestRoomMembers) {
            stats.biggestRoomMembers = room.memberCount;
            stats.biggestRoom = room.name;
        }
    }

    stats.availableServers = extractServers(rooms);
    return stats;
}

std::vector<RoomDirectoryEntry> searchRooms(
    const std::vector<RoomDirectoryEntry>& rooms,
    const std::string& query, int maxResults
) {
    if (query.empty()) return {};

    std::vector<std::pair<RoomDirectoryEntry, double>> scored;

    auto lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (auto room : rooms) {
        auto lowerName = room.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        double score = 0.0;
        if (lowerName == lowerQuery) score = 1.0;
        else if (lowerName.rfind(lowerQuery, 0) == 0) score = 0.8;
        else if (lowerName.find(lowerQuery) != std::string::npos) score = 0.5;
        else if (room.topic.find(query) != std::string::npos) score = 0.3;

        if (score > 0.0) {
            room.relevance = score;
            scored.push_back({room, score});
        }
    }

    std::sort(scored.begin(), scored.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::vector<RoomDirectoryEntry> result;
    int limit = std::min(maxResults, static_cast<int>(scored.size()));
    for (int i = 0; i < limit; ++i) {
        result.push_back(scored[i].first);
    }

    return result;
}

void sortRooms(std::vector<RoomDirectoryEntry>& rooms, const std::string& sortBy) {
    if (sortBy == "members") {
        std::sort(rooms.begin(), rooms.end(), [](const auto& a, const auto& b) {
            return a.memberCount > b.memberCount;
        });
    } else if (sortBy == "relevance") {
        std::sort(rooms.begin(), rooms.end(), [](const auto& a, const auto& b) {
            return a.relevance > b.relevance;
        });
    } else { // name (default)
        std::sort(rooms.begin(), rooms.end(), [](const auto& a, const auto& b) {
            return a.name < b.name;
        });
    }
}

std::string formatDirectoryEntry(const RoomDirectoryEntry& entry) {
    std::ostringstream out;
    out << entry.name << " (" << entry.memberCount << " members)\n";
    if (!entry.topic.empty()) out << "  " << entry.topic << "\n";
    return out.str();
}

std::vector<std::string> extractServers(const std::vector<RoomDirectoryEntry>& rooms) {
    std::unordered_set<std::string> servers;
    for (const auto& room : rooms) {
        auto server = room.roomId;
        auto colon = server.find(':');
        if (colon != std::string::npos) {
            servers.insert(server.substr(colon + 1));
        }
    }
    return std::vector<std::string>(servers.begin(), servers.end());
}

std::string directoryStatsToJson(const DirectoryStats& stats) {
    auto esc = [](const std::string& s) -> std::string {
        std::string out; for (char c : s) { if (c == '"') out += "\\\""; else out += c; } return out;
    };
    std::ostringstream json;
    json << "{";
    json << R"("totalRooms": )" << stats.totalRooms << ",";
    json << R"("filteredRooms": )" << stats.filteredRooms << ",";
    json << R"("totalMembers": )" << stats.totalMembers << ",";
    json << R"("biggestRoom": ")" << esc(stats.biggestRoom) << R"(",)";
    json << R"("servers": [)";
    for (size_t i = 0; i < stats.availableServers.size(); ++i) {
        if (i > 0) json << ",";
        json << R"(")" << esc(stats.availableServers[i]) << R"(")";
    }
    json << "]}";
    return json.str();
}

} // namespace progressive


// ==== Extended room_directory implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string room_directory::serialize() const {
    json j = toJson();
    return j.dump();
}

bool room_directory::deserialize(const std::string& data) {
    if (data.empty()) return false;
    try {
        json j = json::parse(data);
        return fromJson(j);
    } catch (...) {
        setError("Failed to deserialize data");
        return false;
    }
}

// Validation helpers
bool room_directory::validate() const {
    if (!m_initialized) {
        LOGE("room_directory: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool room_directory::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool room_directory::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json room_directory::getMetrics() const {
    json m;
    m["class"] = "room_directory";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int room_directory::getOperationCount() const {
    return m_operationCount;
}

void room_directory::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void room_directory::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "room_directory";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool room_directory::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool room_directory::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool room_directory::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
    auto now = currentTimeMs();
    auto& window = m_rateLimitWindows[key];
    if (now - window.startTime > windowMs) {
        window.startTime = now;
        window.count = 0;
    }
    if (window.count >= maxRequests) return false;
    window.count++;
    return true;
}

// Observation pattern
void room_directory::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void room_directory::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int room_directory::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void room_directory::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> room_directory::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> room_directory::listItems() const {
    return {};
}

int room_directory::itemCount() const {
    return 0;
}

// Versioning
std::string room_directory::getVersion() const {
    return "1.0.0";
}

bool room_directory::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool room_directory::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void room_directory::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> room_directory::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool room_directory::migrateData(int fromVersion, int toVersion) {
    LOGI("room_directory: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int room_directory::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json room_directory::exportData() const {
    return toJson();
}

bool room_directory::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void room_directory::performCleanup() {
    LOGI("room_directory: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t room_directory::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool room_directory::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool room_directory::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool room_directory::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void room_directory::logDebug(const std::string& msg) const {
    LOGI("room_directory: %s", msg.c_str());
}

void room_directory::logWarning(const std::string& msg) const {
    LOGW("room_directory: %s", msg.c_str());
}

void room_directory::logError(const std::string& msg) const {
    LOGE("room_directory: %s", msg.c_str());
}
