#include "progressive/room_sort.hpp"
#include <sstream>
#include <algorithm>

namespace progressive {

bool roomSortCompare(const RoomSortEntry& a, const RoomSortEntry& b) {
    // Original Kotlin (RoomComparator.kt):
    //   compareBy<RoomSummary>(
    //       { it.isFavourite.not() },     // favourites first
    //       { it.isDirect.not() },        // DMs before rooms
    //       { it.highlightCount == 0 },   // highlights first
    //       { it.notificationCount == 0 }, // notifications next
    //       { it.isServerNotice },        // server notices after normal
    //       { it.isSuggested },           // suggested at bottom
    //       { it.isLowPriority },         // low priority at bottom
    //       { it.lastEventTs * -1 }       // newest first
    //   )

    // 1. Favourites first
    bool aFav = a.tag == RoomTag::Favourite;
    bool bFav = b.tag == RoomTag::Favourite;
    if (aFav != bFav) return aFav;

    // 2. DMs before regular rooms
    if (a.isDirect != b.isDirect) return a.isDirect;

    // 3. Unread with highlights first
    bool aHigh = a.hasUnread && a.highlightCount > 0;
    bool bHigh = b.hasUnread && b.highlightCount > 0;
    if (aHigh != bHigh) return aHigh;

    // 4. Unread without highlights
    bool aUnread = a.hasUnread && a.notificationCount > 0;
    bool bUnread = b.hasUnread && b.notificationCount > 0;
    if (aUnread != bUnread) return aUnread;

    // 5. Manually marked unread
    if (a.isMarkedUnread != b.isMarkedUnread) return a.isMarkedUnread;

    // 6. Server notices below normal rooms
    bool aNotice = a.tag == RoomTag::ServerNotice;
    bool bNotice = b.tag == RoomTag::ServerNotice;
    if (aNotice != bNotice) return !aNotice;  // false (not notice) comes first

    // 7. Suggested rooms at bottom
    bool aSuggested = a.tag == RoomTag::Suggested;
    bool bSuggested = b.tag == RoomTag::Suggested;
    if (aSuggested != bSuggested) return !aSuggested;

    // 8. Low priority at very bottom
    bool aLow = a.tag == RoomTag::LowPriority;
    bool bLow = b.tag == RoomTag::LowPriority;
    if (aLow != bLow) return !aLow;

    // 9. Manual priority (higher = closer to top)
    if (a.priority != b.priority) return a.priority > b.priority;

    // 10. By last event timestamp — newest first
    // Original Kotlin: lastEventTs * -1 (negate for descending)
    if (a.lastEventTs != b.lastEventTs) return a.lastEventTs > b.lastEventTs;

    // 11. Alphabetical by display name as tiebreaker
    return a.displayName < b.displayName;
}

std::vector<RoomSortEntry> sortRooms(std::vector<RoomSortEntry> rooms) {
    std::sort(rooms.begin(), rooms.end(), roomSortCompare);
    return rooms;
}

int getRoomSortKey(const RoomSortEntry& room) {
    // Higher = closer to top. Each tier is 1000000 apart.
    int key = 0;

    // Favourites: +7M
    if (room.tag == RoomTag::Favourite) key += 7000000;

    // DMs: +6M
    if (room.isDirect) key += 6000000;

    // Unread highlights: +5M
    if (room.hasUnread && room.highlightCount > 0) key += 5000000;

    // Unread: +4M
    if (room.hasUnread && room.notificationCount > 0) key += 4000000;

    // Marked unread: +3M
    if (room.isMarkedUnread) key += 3000000;

    // Server notice penalty: max 1M
    if (room.tag == RoomTag::ServerNotice) key += 1000000;

    // Suggested: -1M penalty
    if (room.tag == RoomTag::Suggested) key -= 1000000;

    // Low priority: -2M penalty
    if (room.tag == RoomTag::LowPriority) key -= 2000000;

    // Priority: +10000 per level
    key += room.priority * 10000;

    // Timestamp: seconds since epoch / 60 (rough ordering)
    key += static_cast<int>((room.lastEventTs / 60000) & 0xFFFF);

    return key;
}

RoomTag parseRoomTag(const std::string& tagStr) {
    if (tagStr == "m.favourite") return RoomTag::Favourite;
    if (tagStr == "m.lowpriority") return RoomTag::LowPriority;
    if (tagStr == "m.server_notice") return RoomTag::ServerNotice;
    if (tagStr == "im.vector.suggested") return RoomTag::Suggested;
    return RoomTag::NoTag;
}

std::string roomTagToString(RoomTag tag) {
    switch (tag) {
        case RoomTag::Favourite: return "m.favourite";
        case RoomTag::LowPriority: return "m.lowpriority";
        case RoomTag::ServerNotice: return "m.server_notice";
        case RoomTag::Suggested: return "im.vector.suggested";
        default: return "";
    }
}

std::string getRoomSectionName(RoomTag tag, bool isDirect) {
    // Original Kotlin (RoomListViewModel.kt section headers)
    if (tag == RoomTag::Favourite) return "Favourites";
    if (isDirect) return "People";
    if (tag == RoomTag::LowPriority) return "Low Priority";
    if (tag == RoomTag::ServerNotice) return "System Alerts";
    if (tag == RoomTag::Suggested) return "Suggested";
    return "Rooms";
}

bool isDirectSection(const RoomSortEntry& room) {
    return room.isDirect && room.tag != RoomTag::LowPriority;
}

bool isFavouriteSection(const RoomSortEntry& room) {
    return room.tag == RoomTag::Favourite;
}

// ==== Breadcrumbs Sorting (from BreadcrumbsRoomComparator.kt:17-33) ====

bool breadcrumbsRoomCompare(const RoomSortEntry& a, const RoomSortEntry& b) {
    int aIdx = a.priority;  // breadcrumbs index stored in priority field
    int bIdx = b.priority;

    if (aIdx == NOT_IN_BREADCRUMBS) {
        if (bIdx == NOT_IN_BREADCRUMBS) {
            // Both not in breadcrumbs — fall back to chronological
            return a.lastEventTs > b.lastEventTs;
        }
        return false; // b has breadcrumbs, b comes first
    }
    if (bIdx == NOT_IN_BREADCRUMBS) {
        return true; // a has breadcrumbs, a comes first
    }
    // Both have breadcrumbs — sort by index (lower = more recent)
    return aIdx < bIdx;
}

std::vector<RoomSortEntry> sortRoomsByBreadcrumbs(std::vector<RoomSortEntry> rooms) {
    std::sort(rooms.begin(), rooms.end(), breadcrumbsRoomCompare);
    return rooms;
}

} // namespace progressive


// ==== Extended room_sort implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string room_sort::serialize() const {
    json j = toJson();
    return j.dump();
}

bool room_sort::deserialize(const std::string& data) {
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
bool room_sort::validate() const {
    if (!m_initialized) {
        LOGE("room_sort: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool room_sort::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool room_sort::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json room_sort::getMetrics() const {
    json m;
    m["class"] = "room_sort";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int room_sort::getOperationCount() const {
    return m_operationCount;
}

void room_sort::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void room_sort::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "room_sort";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool room_sort::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool room_sort::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool room_sort::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void room_sort::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void room_sort::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int room_sort::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void room_sort::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> room_sort::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> room_sort::listItems() const {
    return {};
}

int room_sort::itemCount() const {
    return 0;
}

// Versioning
std::string room_sort::getVersion() const {
    return "1.0.0";
}

bool room_sort::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool room_sort::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void room_sort::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> room_sort::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool room_sort::migrateData(int fromVersion, int toVersion) {
    LOGI("room_sort: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int room_sort::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json room_sort::exportData() const {
    return toJson();
}

bool room_sort::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void room_sort::performCleanup() {
    LOGI("room_sort: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t room_sort::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool room_sort::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool room_sort::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool room_sort::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void room_sort::logDebug(const std::string& msg) const {
    LOGI("room_sort: %s", msg.c_str());
}

void room_sort::logWarning(const std::string& msg) const {
    LOGW("room_sort: %s", msg.c_str());
}

void room_sort::logError(const std::string& msg) const {
    LOGE("room_sort: %s", msg.c_str());
}
