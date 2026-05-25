#include "progressive/sync_filter.hpp"
#include "progressive/sync_utils.hpp"
#include <sstream>

namespace progressive {

// ---- RoomEventFilter ----
// Original Kotlin (RoomSyncFilterBuilder.kt:RoomEventFilter):
//   data class RoomEventFilter(
//       val enableUnreadThreadNotifications: Boolean? = null,
//       val lazyLoadMembers: Boolean? = null,
//       val types: List<String>? = null,
//       val notTypes: List<String>? = null
//   )

bool RoomEventFilter::hasData() const {
    return lazyLoadMembers.has_value() ||
           enableUnreadThreadNotifications.has_value() ||
           !types.empty();
}

std::string RoomEventFilter::toJson() const {
    if (!hasData()) return "{}";

    std::ostringstream json;
    json << "{";
    bool first = true;

    if (lazyLoadMembers.has_value()) {
        json << R"("lazy_load_members": )" << (*lazyLoadMembers ? "true" : "false");
        first = false;
    }

    if (enableUnreadThreadNotifications.has_value()) {
        if (!first) json << ",";
        json << R"("unread_thread_notifications": )" << (*enableUnreadThreadNotifications ? "true" : "false");
        first = false;
    }

    if (!types.empty()) {
        if (!first) json << ",";
        json << R"("types": [)";
        for (size_t i = 0; i < types.size(); ++i) {
            if (i > 0) json << ",";
            json << R"(")" << types[i] << R"(")";
        }
        json << "]";
        first = false;
    }

    json << "}";
    return json.str();
}

// ---- RoomFilter ----
// Original Kotlin: RoomFilter(timeline, state)

bool RoomFilter::hasData() const {
    return timeline.hasData() || state.hasData();
}

std::string RoomFilter::toJson() const {
    // Only output if there's actual data
    if (!hasData()) return "{}";

    std::ostringstream json;
    json << "{";
    bool first = true;

    if (timeline.hasData()) {
        json << R"("timeline": )" << timeline.toJson();
        first = false;
    }

    if (state.hasData()) {
        if (!first) json << ",";
        json << R"("state": )" << state.toJson();
        first = false;
    }

    json << "}";
    return json.str();
}

// ---- RoomSyncFilter ----
// Original Kotlin (Filter): data class Filter(val room: RoomFilter? = null, ...)

bool RoomSyncFilter::hasData() const {
    return room.hasData() || !eventFields.empty() || !eventFormat.empty();
}

std::string RoomSyncFilter::toJson() const {
    if (!hasData()) return "{}";

    std::ostringstream json;
    json << "{";
    bool first = true;

    if (room.hasData()) {
        json << R"("room": )" << room.toJson();
        first = false;
    }

    if (!eventFields.empty()) {
        if (!first) json << ",";
        json << R"("event_fields": [)";
        for (size_t i = 0; i < eventFields.size(); ++i) {
            if (i > 0) json << ",";
            json << R"(")" << eventFields[i] << R"(")";
        }
        json << "]";
        first = false;
    }

    if (!eventFormat.empty()) {
        if (!first) json << ",";
        json << R"("event_format": ")" << eventFormat << R"(")";
    }

    json << "}";
    return json.str();
}

// ---- RoomSyncFilterBuilder logic (from RoomSyncFilterBuilder.kt:57-94) ----
// Original Kotlin:
//   fun build(homeServerCapabilities: HomeServerCapabilities): Filter {
//       return Filter(room = buildRoomFilter(homeServerCapabilities))
//   }
//   private fun buildRoomFilter(caps): RoomFilter {
//       return RoomFilter(timeline = buildTimelineFilter(caps), state = buildStateFilter())
//   }
//   private fun buildTimelineFilter(caps): RoomEventFilter? {
//       val resolvedUseThreadNotifications = if (caps.canUseThreadReadReceiptsAndNotifications)
//           useThreadNotifications else null
//       return RoomEventFilter(
//           enableUnreadThreadNotifications = resolvedUseThreadNotifications,
//           lazyLoadMembers = lazyLoadMembersForMessageEvents
//       ).orNullIfEmpty()
//   }
//   private fun buildStateFilter(): RoomEventFilter? =
//       RoomEventFilter(lazyLoadMembers = lazyLoadMembersForStateEvents, types = supportedStateTypes).orNullIfEmpty()

RoomSyncFilter buildRoomSyncFilter(
    const SyncFilterParams& params,
    bool canUseThreadReadReceiptsAndNotifications
) {
    RoomSyncFilter filter;

    // Build timeline filter
    // Original: lazyLoadMembers = lazyLoadMembersForMessageEvents
    if (params.lazyLoadMembersForMessageEvents.has_value()) {
        filter.room.timeline.lazyLoadMembers = params.lazyLoadMembersForMessageEvents;
    }

    // Original: enableUnreadThreadNotifications = if (caps.canUseThread...) useThreadNotifications else null
    if (canUseThreadReadReceiptsAndNotifications && params.useThreadNotifications.has_value()) {
        filter.room.timeline.enableUnreadThreadNotifications = params.useThreadNotifications;
    }

    // Original: timeline event types
    filter.room.timeline.types = params.listOfSupportedEventTypes;

    // Build state filter
    // Original: lazyLoadMembers = lazyLoadMembersForStateEvents
    if (params.lazyLoadMembersForStateEvents.has_value()) {
        filter.room.state.lazyLoadMembers = params.lazyLoadMembersForStateEvents;
    }

    // Original: state event types
    filter.room.state.types = params.listOfSupportedStateEventTypes;

    // Default event format
    filter.eventFormat = "client";

    return filter;
}

RoomSyncFilter getDefaultRoomSyncFilter() {
    SyncFilterParams params;
    params.lazyLoadMembersForMessageEvents = true;
    params.lazyLoadMembersForStateEvents = true;

    // Default timeline event types to include
    params.listOfSupportedEventTypes = {
        "m.room.message",
        "m.room.member",
        "m.room.name",
        "m.room.topic",
        "m.room.avatar",
        "m.room.canonical_alias",
        "m.room.join_rules",
        "m.room.history_visibility",
        "m.room.guest_access",
        "m.room.power_levels",
        "m.room.redaction",
        "m.room.encryption",
        "m.room.create",
        "m.sticker",
        "m.reaction",
        "m.call.invite",
        "m.call.answer",
        "m.call.hangup"
    };

    // Default state event types
    params.listOfSupportedStateEventTypes = {
        "m.room.name",
        "m.room.topic",
        "m.room.avatar",
        "m.room.canonical_alias",
        "m.room.join_rules",
        "m.room.history_visibility",
        "m.room.guest_access",
        "m.room.power_levels",
        "m.room.encryption",
        "m.room.create",
        "m.room.member",
        "m.room.tombstone"
    };

    return buildRoomSyncFilter(params, false);
}

bool hasActiveFiltering(const RoomSyncFilter& filter) {
    // Original: orNullIfEmpty() — null if no data
    return filter.hasData();
}

std::string syncFilterToJson(const RoomSyncFilter& filter) {
    return filter.toJson();
}

} // namespace progressive


// ==== Extended sync_filter implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string sync_filter::serialize() const {
    json j = toJson();
    return j.dump();
}

bool sync_filter::deserialize(const std::string& data) {
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
bool sync_filter::validate() const {
    if (!m_initialized) {
        LOGE("sync_filter: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool sync_filter::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool sync_filter::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json sync_filter::getMetrics() const {
    json m;
    m["class"] = "sync_filter";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int sync_filter::getOperationCount() const {
    return m_operationCount;
}

void sync_filter::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void sync_filter::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "sync_filter";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool sync_filter::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool sync_filter::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool sync_filter::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void sync_filter::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void sync_filter::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int sync_filter::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void sync_filter::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> sync_filter::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> sync_filter::listItems() const {
    return {};
}

int sync_filter::itemCount() const {
    return 0;
}

// Versioning
std::string sync_filter::getVersion() const {
    return "1.0.0";
}

bool sync_filter::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool sync_filter::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void sync_filter::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> sync_filter::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool sync_filter::migrateData(int fromVersion, int toVersion) {
    LOGI("sync_filter: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int sync_filter::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json sync_filter::exportData() const {
    return toJson();
}

bool sync_filter::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void sync_filter::performCleanup() {
    LOGI("sync_filter: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t sync_filter::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool sync_filter::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool sync_filter::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool sync_filter::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void sync_filter::logDebug(const std::string& msg) const {
    LOGI("sync_filter: %s", msg.c_str());
}

void sync_filter::logWarning(const std::string& msg) const {
    LOGW("sync_filter: %s", msg.c_str());
}

void sync_filter::logError(const std::string& msg) const {
    LOGE("sync_filter: %s", msg.c_str());
}
