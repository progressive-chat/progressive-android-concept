#include "matrix_sdk/room/services/matrix_sdk_room_services.hpp"
#include "matrix_sdk/matrix_sdk_types.hpp"
#include "matrix_sdk/events/matrix_sdk_event.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <unordered_map>

namespace matrix_sdk {

// ============================================================================
// RelationServiceImpl
// ============================================================================
class RelationServiceImpl : public RelationService {
public:
    explicit RelationServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    std::map<std::string, EventAnnotation> getAnnotations(const std::string& eventId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = annotationCache_.find(eventId);
        if (it != annotationCache_.end()) {
            return it->second;
        }
        return {};
    }

    int getAnnotationCount(const std::string& eventId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = annotationCache_.find(eventId);
        if (it != annotationCache_.end()) {
            return static_cast<int>(it->second.size());
        }
        return 0;
    }

    void fetchAnnotations(const std::string& eventId, const std::string& key,
        std::function<void(bool, const std::string&, const std::map<std::string, EventAnnotation>&)> callback) override {
        if (eventId.empty()) {
            if (callback) callback(false, "Event ID is required", {});
            return;
        }

        // In a real impl this would call GET /rooms/{roomId}/relations/{eventId}
        // with optional ?key={key} filter
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = annotationCache_.find(eventId);
        std::map<std::string, EventAnnotation> result;
        if (it != annotationCache_.end()) {
            if (key.empty()) {
                result = it->second;
            } else {
                // Filter by key
                for (const auto& [annEventId, annotation] : it->second) {
                    if (annotation.key == key) {
                        result[annEventId] = annotation;
                    }
                }
            }
        }

        if (callback) {
            callback(true, "", result);
        }
    }

    // ── Internal helpers ──
    void addAnnotation(const std::string& eventId, const EventAnnotation& annotation) {
        std::lock_guard<std::mutex> lock(mutex_);
        annotationCache_[eventId][annotation.eventId] = annotation;
    }

    void removeAnnotation(const std::string& eventId, const std::string& annotationEventId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = annotationCache_.find(eventId);
        if (it != annotationCache_.end()) {
            it->second.erase(annotationEventId);
        }
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    // eventId -> (annotationEventId -> EventAnnotation)
    std::map<std::string, std::map<std::string, EventAnnotation>> annotationCache_;
};

// ============================================================================
// ThreadsServiceImpl
// ============================================================================
class ThreadsServiceImpl : public ThreadsService {
public:
    explicit ThreadsServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    void fetchThreads(ThreadCallback callback) override {
        // In a real impl this would call GET /rooms/{roomId}/threads
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<ThreadInfo> threads;
        threads.reserve(threadCache_.size());
        for (const auto& [threadId, threadInfo] : threadCache_) {
            threads.push_back(threadInfo);
        }
        // Sort by latest event (most recent first)
        std::sort(threads.begin(), threads.end(), [](const ThreadInfo& a, const ThreadInfo& b) {
            return a.numberOfReplies > b.numberOfReplies;
        });
        if (callback) {
            callback(true, "", threads);
        }
    }

    std::vector<ThreadInfo> getCachedThreads() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<ThreadInfo> result;
        result.reserve(threadCache_.size());
        for (const auto& [threadId, threadInfo] : threadCache_) {
            result.push_back(threadInfo);
        }
        std::sort(result.begin(), result.end(), [](const ThreadInfo& a, const ThreadInfo& b) {
            return a.numberOfReplies > b.numberOfReplies;
        });
        return result;
    }

    std::optional<ThreadInfo> getThread(const std::string& threadId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = threadCache_.find(threadId);
        if (it != threadCache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    int getThreadCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return static_cast<int>(threadCache_.size());
    }

    // ── Internal helpers ──
    void addThread(const ThreadInfo& thread) {
        if (thread.threadId.empty()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = threadCache_.find(thread.threadId);
        if (it != threadCache_.end()) {
            // Update existing thread
            it->second.latestEventId = thread.latestEventId;
            it->second.numberOfReplies = thread.numberOfReplies;
        } else {
            threadCache_[thread.threadId] = thread;
        }
    }

    void incrementReplyCount(const std::string& threadId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = threadCache_.find(threadId);
        if (it != threadCache_.end()) {
            it->second.numberOfReplies++;
        }
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::map<std::string, ThreadInfo> threadCache_;
};

// ============================================================================
// PollHistoryServiceImpl
// ============================================================================
class PollHistoryServiceImpl : public PollHistoryService {
public:
    explicit PollHistoryServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    std::vector<PollQuestion> getActivePolls() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<PollQuestion> result;
        for (const auto& [eventId, poll] : polls_) {
            if (!poll.isClosed) {
                result.push_back(poll);
            }
        }
        return result;
    }

    std::vector<PollQuestion> getEndedPolls() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<PollQuestion> result;
        for (const auto& [eventId, poll] : polls_) {
            if (poll.isClosed) {
                result.push_back(poll);
            }
        }
        return result;
    }

    std::optional<PollQuestion> getPoll(const std::string& eventId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = polls_.find(eventId);
        if (it != polls_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // ── Internal helpers ──
    void addPoll(const std::string& eventId, const PollQuestion& poll) {
        std::lock_guard<std::mutex> lock(mutex_);
        polls_[eventId] = poll;
    }

    void endPoll(const std::string& eventId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = polls_.find(eventId);
        if (it != polls_.end()) {
            it->second.isClosed = true;
        }
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::map<std::string, PollQuestion> polls_; // eventId -> poll
};

// ============================================================================
// LocationSharingServiceImpl
// ============================================================================
class LocationSharingServiceImpl : public LocationSharingService {
public:
    explicit LocationSharingServiceImpl(const std::string& roomId) : roomId_(roomId), isSharing_(false) {}

    void shareLocation(const LocationData& location, SimpleCallback callback) override {
        if (location.latitude == 0.0 && location.longitude == 0.0) {
            callback(false, "Invalid location data");
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        currentLocation_ = location;
        isSharing_ = true;

        // In a real impl this would send a beacon info state event
        BeaconInfo beacon;
        beacon.eventId = "beacon_" + std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        beacon.description = location.description;
        beacon.latitude = location.latitude;
        beacon.longitude = location.longitude;
        beacon.uncertainty = location.uncertainty;
        beacon.isLive = true;
        beacon.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        activeBeacons_[beacon.eventId] = beacon;

        callback(true, "");
    }

    void stopSharingLocation(SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        isSharing_ = false;
        // Mark all active beacons as not live
        for (auto& [id, beacon] : activeBeacons_) {
            beacon.isLive = false;
        }
        activeBeacons_.clear();
        callback(true, "");
    }

    bool isSharingLocation() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return isSharing_;
    }

    std::vector<BeaconInfo> getActiveBeacons() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<BeaconInfo> result;
        for (const auto& [id, beacon] : activeBeacons_) {
            if (beacon.isLive) {
                result.push_back(beacon);
            }
        }
        return result;
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    LocationData currentLocation_;
    bool isSharing_;
    std::map<std::string, BeaconInfo> activeBeacons_;
};

// ============================================================================
// BeaconInfoSummaryServiceImpl
// ============================================================================
class BeaconInfoSummaryServiceImpl : public BeaconInfoSummaryService {
public:
    explicit BeaconInfoSummaryServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    std::vector<BeaconInfo> getBeaconInfos() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<BeaconInfo> result;
        result.reserve(beacons_.size());
        for (const auto& [eventId, beacon] : beacons_) {
            result.push_back(beacon);
        }
        return result;
    }

    std::optional<BeaconInfo> getBeaconInfo(const std::string& eventId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = beacons_.find(eventId);
        if (it != beacons_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool hasActiveBeacons() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [eventId, beacon] : beacons_) {
            if (beacon.isLive) return true;
        }
        return false;
    }

    // ── Internal helpers ──
    void addBeacon(const BeaconInfo& beacon) {
        if (beacon.eventId.empty()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        beacons_[beacon.eventId] = beacon;
    }

    void removeBeacon(const std::string& eventId) {
        std::lock_guard<std::mutex> lock(mutex_);
        beacons_.erase(eventId);
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::map<std::string, BeaconInfo> beacons_;
};

// ============================================================================
// RoomCallServiceImpl
// ============================================================================
class RoomCallServiceImpl : public RoomCallService {
public:
    explicit RoomCallServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    void placeCall(const std::string& callId, SimpleCallback callback) override {
        if (callId.empty()) {
            callback(false, "Call ID is required");
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (activeCalls_.find(callId) != activeCalls_.end()) {
            callback(false, "Call already exists: " + callId);
            return;
        }

        CallInfo info;
        info.callId = callId;
        info.roomId = roomId_;
        info.state = CallState::RINGING;
        info.createdAt = std::chrono::system_clock::now();
        activeCalls_[callId] = info;

        // In a real impl this would send m.call.invite event
        callback(true, "");
    }

    void answerCall(const std::string& callId, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = activeCalls_.find(callId);
        if (it == activeCalls_.end()) {
            callback(false, "Call not found: " + callId);
            return;
        }
        it->second.state = CallState::CONNECTED;
        // In a real impl this would send m.call.answer event
        callback(true, "");
    }

    void hangupCall(const std::string& callId, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = activeCalls_.find(callId);
        if (it != activeCalls_.end()) {
            it->second.state = CallState::ENDED;
        }
        // In a real impl this would send m.call.hangup event
        callback(true, "");

        // Clean up after a delay in real impl
        activeCalls_.erase(callId);
    }

    void rejectCall(const std::string& callId, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = activeCalls_.find(callId);
        if (it != activeCalls_.end()) {
            it->second.state = CallState::REJECTED;
        }
        // In a real impl this would send m.call.reject event
        callback(true, "");
        activeCalls_.erase(callId);
    }

    bool hasActiveCall(const std::string& roomId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [callId, info] : activeCalls_) {
            if (info.roomId == roomId &&
                (info.state == CallState::CONNECTED || info.state == CallState::RINGING)) {
                return true;
            }
        }
        return false;
    }

    bool hasActiveCalls() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [callId, info] : activeCalls_) {
            if (info.state == CallState::CONNECTED || info.state == CallState::RINGING) {
                return true;
            }
        }
        return false;
    }

private:
    enum class CallState {
        RINGING,
        CONNECTED,
        ENDED,
        REJECTED
    };

    struct CallInfo {
        std::string callId;
        std::string roomId;
        CallState state = CallState::RINGING;
        std::chrono::system_clock::time_point createdAt;
    };

    std::string roomId_;
    mutable std::mutex mutex_;
    std::map<std::string, CallInfo> activeCalls_;
};

// ============================================================================
// AliasServiceImpl
// ============================================================================
class AliasServiceImpl : public AliasService {
public:
    explicit AliasServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    void addAlias(const std::string& alias, SimpleCallback callback) override {
        if (alias.empty()) {
            callback(false, "Alias is required");
            return;
        }
        // Validate alias format: must start with #
        if (alias[0] != '#') {
            callback(false, "Alias must start with '#'");
            return;
        }
        // Must contain exactly one ':'
        size_t colonPos = alias.find(':');
        if (colonPos == std::string::npos || alias.find(':', colonPos + 1) != std::string::npos) {
            callback(false, "Invalid alias format: expected #localpart:server");
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (std::find(localAliases_.begin(), localAliases_.end(), alias) != localAliases_.end()) {
            callback(false, "Alias already exists: " + alias);
            return;
        }
        localAliases_.push_back(alias);
        // In a real impl this would call PUT /directory/room/{alias}
        callback(true, "");
    }

    void removeAlias(const std::string& alias, SimpleCallback callback) override {
        if (alias.empty()) {
            callback(false, "Alias is required");
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find(localAliases_.begin(), localAliases_.end(), alias);
        if (it == localAliases_.end()) {
            callback(false, "Alias not found: " + alias);
            return;
        }
        localAliases_.erase(it);
        // In a real impl this would call DELETE /directory/room/{alias}
        callback(true, "");
    }

    void resolveAlias(const std::string& alias,
        std::function<void(bool, const std::string&, const std::string&, const std::vector<std::string>&)> callback) override {
        if (alias.empty()) {
            if (callback) callback(false, "", "Alias is required", {});
            return;
        }

        // In a real impl this would call GET /directory/room/{alias}
        // which returns roomId and servers
        // For now, return the local room ID if the alias is known
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find(localAliases_.begin(), localAliases_.end(), alias);
        if (it != localAliases_.end()) {
            if (callback) callback(true, roomId_, "", {});
            return;
        }

        // If it's the published alias, resolve it
        if (alias == publishedAlias_) {
            if (callback) callback(true, roomId_, "", {});
            return;
        }

        if (callback) callback(false, "", "Alias not found: " + alias, {});
    }

    std::vector<std::string> getLocalAliases() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return localAliases_;
    }

    std::string getPublishedAlias() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return publishedAlias_;
    }

    // ── Internal helpers ──
    void setPublishedAlias(const std::string& alias) {
        std::lock_guard<std::mutex> lock(mutex_);
        publishedAlias_ = alias;
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::vector<std::string> localAliases_;
    std::string publishedAlias_;
};

// ============================================================================
// ReportingServiceImpl
// ============================================================================
class ReportingServiceImpl : public ReportingService {
public:
    explicit ReportingServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    void reportEvent(const std::string& eventId, int score, const std::string& reason,
        SimpleCallback callback) override {
        if (eventId.empty()) {
            callback(false, "Event ID is required for reporting");
            return;
        }
        // Validate score range (-100 to 0)
        if (score > 0 || score < -100) {
            callback(false, "Score must be between -100 and 0");
            return;
        }
        if (reason.empty()) {
            callback(false, "Reason is required for reporting");
            return;
        }

        json body;
        body["reason"] = reason;
        body["score"] = score;

        // In a real impl this would call POST /rooms/{roomId}/report/{eventId}
        callback(true, "");
    }

    void reportRoom(int score, const std::string& reason, SimpleCallback callback) override {
        // Validate score range (-100 to 0)
        if (score > 0 || score < -100) {
            callback(false, "Score must be between -100 and 0");
            return;
        }
        if (reason.empty()) {
            callback(false, "Reason is required for reporting");
            return;
        }

        json body;
        body["reason"] = reason;
        body["score"] = score;

        // In a real impl this would call POST /rooms/{roomId}/report
        callback(true, "");
    }

    void reportUser(const std::string& userId, int score, const std::string& reason,
        SimpleCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required for reporting");
            return;
        }
        // Validate score range (-100 to 0)
        if (score > 0 || score < -100) {
            callback(false, "Score must be between -100 and 0");
            return;
        }
        if (reason.empty()) {
            callback(false, "Reason is required for reporting");
            return;
        }

        json body;
        body["reason"] = reason;
        body["score"] = score;
        body["user_id"] = userId;

        // In a real impl this would call POST /rooms/{roomId}/report/{userId}
        callback(true, "");
    }

private:
    std::string roomId_;
};

// ============================================================================
// RoomVersionServiceImpl
// ============================================================================
class RoomVersionServiceImpl : public RoomVersionService {
public:
    explicit RoomVersionServiceImpl(const std::string& roomId)
        : roomId_(roomId), roomVersion_("1") {
        // Initialize supported versions per Matrix specification
        supportedVersions_ = {
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"
        };
    }

    std::string getRoomVersion() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return roomVersion_;
    }

    void upgradeRoom(const std::string& newVersion, SimpleCallback callback) override {
        if (newVersion.empty()) {
            callback(false, "New version is required");
            return;
        }

        if (!isVersionSupported(newVersion)) {
            callback(false, "Unsupported room version: " + newVersion);
            return;
        }

        // In a real impl this would:
        // 1. Call POST /rooms/{roomId}/upgrade with new_version
        // 2. Create a tombstone event in the old room
        // 3. Return the replacement room ID

        json body;
        body["new_version"] = newVersion;

        std::lock_guard<std::mutex> lock(mutex_);
        roomVersion_ = newVersion;
        callback(true, "");
    }

    std::vector<std::string> getSupportedVersions() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return supportedVersions_;
    }

    bool isVersionSupported(const std::string& version) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::find(supportedVersions_.begin(), supportedVersions_.end(), version)
            != supportedVersions_.end();
    }

    bool canUpgrade() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        // Can upgrade if current version is not the latest supported
        auto it = std::find(supportedVersions_.begin(), supportedVersions_.end(), roomVersion_);
        if (it == supportedVersions_.end()) return false;
        return (it + 1) != supportedVersions_.end();
    }

    // ── Internal helpers ──
    void setRoomVersion(const std::string& version) {
        std::lock_guard<std::mutex> lock(mutex_);
        roomVersion_ = version;
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::string roomVersion_;
    std::vector<std::string> supportedVersions_;
};

// ============================================================================
// RoomAccountDataServiceImpl
// ============================================================================
class RoomAccountDataServiceImpl : public RoomAccountDataService {
public:
    explicit RoomAccountDataServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    void setAccountData(const std::string& type, const std::string& content, SimpleCallback callback) override {
        if (type.empty()) {
            callback(false, "Account data type is required");
            return;
        }

        // Validate content JSON
        try {
            json parsed = json::parse(content);
        } catch (const json::parse_error& e) {
            callback(false, std::string("Invalid content JSON: ") + e.what());
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        accountData_[type] = content;

        // In a real impl this would call PUT /user/{userId}/rooms/{roomId}/account_data/{type}
        callback(true, "");
    }

    std::string getAccountData(const std::string& type) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = accountData_.find(type);
        if (it != accountData_.end()) {
            return it->second;
        }
        return "{}";
    }

    std::map<std::string, std::string> getAllAccountData() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return accountData_;
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::map<std::string, std::string> accountData_; // type -> JSON content
};

// ============================================================================
// ThreadsLocalServiceImpl
// ============================================================================
class ThreadsLocalServiceImpl : public ThreadsLocalService {
public:
    explicit ThreadsLocalServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    void addLocalEcho(const std::string& threadId, const std::shared_ptr<TimelineEvent>& event) override {
        if (!event || threadId.empty()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        localEchoes_[threadId].push_back(event);
    }

    std::vector<std::shared_ptr<TimelineEvent>> getLocalEchos(const std::string& threadId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = localEchoes_.find(threadId);
        if (it != localEchoes_.end()) {
            return it->second;
        }
        return {};
    }

    void removeLocalEcho(const std::string& transactionId) override {
        if (transactionId.empty()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [threadId, echos] : localEchoes_) {
            echos.erase(
                std::remove_if(echos.begin(), echos.end(),
                    [&transactionId](const std::shared_ptr<TimelineEvent>& event) {
                        return event && event->getEventId() == transactionId;
                    }),
                echos.end());
        }
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<TimelineEvent>>> localEchoes_; // threadId -> echos
};

// ============================================================================
// Factory functions
// ============================================================================

std::shared_ptr<RelationService> createRelationService(const std::string& roomId) {
    return std::make_shared<RelationServiceImpl>(roomId);
}

std::shared_ptr<ThreadsService> createThreadsService(const std::string& roomId) {
    return std::make_shared<ThreadsServiceImpl>(roomId);
}

std::shared_ptr<PollHistoryService> createPollHistoryService(const std::string& roomId) {
    return std::make_shared<PollHistoryServiceImpl>(roomId);
}

std::shared_ptr<LocationSharingService> createLocationSharingService(const std::string& roomId) {
    return std::make_shared<LocationSharingServiceImpl>(roomId);
}

std::shared_ptr<BeaconInfoSummaryService> createBeaconInfoSummaryService(const std::string& roomId) {
    return std::make_shared<BeaconInfoSummaryServiceImpl>(roomId);
}

std::shared_ptr<RoomCallService> createRoomCallService(const std::string& roomId) {
    return std::make_shared<RoomCallServiceImpl>(roomId);
}

std::shared_ptr<AliasService> createAliasService(const std::string& roomId) {
    return std::make_shared<AliasServiceImpl>(roomId);
}

std::shared_ptr<ReportingService> createReportingService(const std::string& roomId) {
    return std::make_shared<ReportingServiceImpl>(roomId);
}

std::shared_ptr<RoomVersionService> createRoomVersionService(const std::string& roomId) {
    return std::make_shared<RoomVersionServiceImpl>(roomId);
}

std::shared_ptr<RoomAccountDataService> createRoomAccountDataService(const std::string& roomId) {
    return std::make_shared<RoomAccountDataServiceImpl>(roomId);
}

std::shared_ptr<ThreadsLocalService> createThreadsLocalService(const std::string& roomId) {
    return std::make_shared<ThreadsLocalServiceImpl>(roomId);
}

} // namespace matrix_sdk
