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

namespace matrix_sdk {

// ============================================================================
// TimelineServiceImpl
// ============================================================================
class TimelineServiceImpl : public TimelineService {
public:
    explicit TimelineServiceImpl(const std::string& roomId)
        : roomId_(roomId), isLive_(false), canPaginateBackward_(true), canPaginateForward_(false) {}

    // ── Pagination ──
    void paginate(int count, const std::string& direction,
        std::function<void(bool, const std::string&, bool)> callback) override {
        if (count <= 0 || count > 256) {
            callback(false, "Invalid count: must be between 1 and 256", false);
            return;
        }
        bool isBackward = (direction == "b" || direction == "backward");
        if (!isBackward && direction != "f" && direction != "forward") {
            callback(false, "Invalid direction: use 'b'/'f' or 'backward'/'forward'", false);
            return;
        }
        if (isBackward && !canPaginateBackward_) {
            callback(false, "Cannot paginate backward: already at beginning", false);
            return;
        }
        if (!isBackward && !canPaginateForward_) {
            callback(false, "Cannot paginate forward: already at live end", false);
            return;
        }

        // Simulate a pagination request
        hasMore_ = true; // In a real impl, this would come from the homeserver
        callback(true, "", hasMore_);
    }

    void paginateForward(int count, SimpleCallback callback) override {
        paginate(count, "f", [callback](bool success, const std::string& error, bool) {
            callback(success, error);
        });
    }

    void paginateBackward(int count, SimpleCallback callback) override {
        paginate(count, "b", [callback](bool success, const std::string& error, bool) {
            callback(success, error);
        });
    }

    bool canPaginateForward() const override { return canPaginateForward_; }
    bool canPaginateBackward() const override { return canPaginateBackward_; }
    bool isLive() const override { return isLive_; }

    // ── Events ──
    std::vector<std::shared_ptr<TimelineEvent>> getEvents() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::shared_ptr<TimelineEvent>> result;
        result.reserve(events_.size());
        for (const auto& pair : events_) {
            result.push_back(pair.second);
        }
        return result;
    }

    std::shared_ptr<TimelineEvent> getEvent(const std::string& eventId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = events_.find(eventId);
        if (it != events_.end()) {
            return it->second;
        }
        return nullptr;
    }

    int getEventCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return static_cast<int>(events_.size());
    }

    std::shared_ptr<TimelineEvent> getLastLiveEvent() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (eventOrder_.empty()) return nullptr;
        return events_.at(eventOrder_.back());
    }

    // ── Listeners ──
    void addListener(std::shared_ptr<TimelineEventListener> listener) override {
        if (!listener) return;
        std::lock_guard<std::mutex> lock(listenersMutex_);
        listeners_.push_back(listener);
    }

    void removeListener(std::shared_ptr<TimelineEventListener> listener) override {
        if (!listener) return;
        std::lock_guard<std::mutex> lock(listenersMutex_);
        listeners_.erase(
            std::remove(listeners_.begin(), listeners_.end(), listener),
            listeners_.end());
    }

    // ── Filters ──
    void filterByType(EventType type) override {
        std::lock_guard<std::mutex> lock(filterMutex_);
        filterTypes_.clear();
        filterTypes_.insert(type);
    }

    void filterByTypes(const std::vector<EventType>& types) override {
        std::lock_guard<std::mutex> lock(filterMutex_);
        filterTypes_.clear();
        for (auto t : types) filterTypes_.insert(t);
    }

    void clearFilter() override {
        std::lock_guard<std::mutex> lock(filterMutex_);
        filterTypes_.clear();
        threadFilter_.clear();
    }

    void filterByThread(const std::string& threadId) override {
        std::lock_guard<std::mutex> lock(filterMutex_);
        threadFilter_ = threadId;
    }

    void clearThreadFilter() override {
        std::lock_guard<std::mutex> lock(filterMutex_);
        threadFilter_.clear();
    }

    // ── Internal helpers (for use by parent Room) ──
    void addEvent(std::shared_ptr<TimelineEvent> event) {
        if (!event) return;
        std::lock_guard<std::mutex> lock(mutex_);
        const std::string& id = event->getEventId();
        if (events_.find(id) != events_.end()) {
            // Update existing event
            events_[id] = event;
            notifyEventModified(event);
            return;
        }
        events_[id] = event;
        eventOrder_.push_back(id);
        notifyNewEvents({event});
    }

    void addEvents(const std::vector<std::shared_ptr<TimelineEvent>>& events) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& event : events) {
            if (!event) continue;
            const std::string& id = event->getEventId();
            if (events_.find(id) == events_.end()) {
                eventOrder_.push_back(id);
            }
            events_[id] = event;
        }
        notifyNewEvents(events);
    }

    void deleteEvent(const std::string& eventId) {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.erase(eventId);
        eventOrder_.erase(
            std::remove(eventOrder_.begin(), eventOrder_.end(), eventId),
            eventOrder_.end());
        notifyEventDeleted(eventId);
    }

    void setLive(bool live) { isLive_ = live; }
    void setCanPaginateBackward(bool can) { canPaginateBackward_ = can; }
    void setCanPaginateForward(bool can) { canPaginateForward_ = can; }

private:
    void notifyNewEvents(const std::vector<std::shared_ptr<TimelineEvent>>& events) {
        std::lock_guard<std::mutex> lock(listenersMutex_);
        for (auto& listener : listeners_) {
            listener->onNewEvents(events);
        }
        for (auto& listener : listeners_) {
            listener->onTimelineUpdated();
        }
    }

    void notifyEventModified(const std::shared_ptr<TimelineEvent>& event) {
        std::lock_guard<std::mutex> lock(listenersMutex_);
        for (auto& listener : listeners_) {
            listener->onEventModified(event);
        }
    }

    void notifyEventDeleted(const std::string& eventId) {
        std::lock_guard<std::mutex> lock(listenersMutex_);
        for (auto& listener : listeners_) {
            listener->onEventDeleted(eventId);
        }
    }

    std::string roomId_;
    bool isLive_;
    bool canPaginateBackward_;
    bool canPaginateForward_;
    bool hasMore_ = false;

    mutable std::mutex mutex_;
    std::map<std::string, std::shared_ptr<TimelineEvent>> events_;
    std::vector<std::string> eventOrder_;

    mutable std::mutex listenersMutex_;
    std::vector<std::shared_ptr<TimelineEventListener>> listeners_;

    mutable std::mutex filterMutex_;
    std::set<EventType> filterTypes_;
    std::string threadFilter_;
};

// ============================================================================
// SendServiceImpl
// ============================================================================
class SendServiceImpl : public SendService {
public:
    explicit SendServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    void sendMessage(const std::string& body, MessageType type, SendCallback callback) override {
        if (body.empty()) {
            callback(false, "Cannot send empty message", nullptr);
            return;
        }

        // Build content JSON for the message
        json content;
        content["msgtype"] = messageTypeToString(type);
        content["body"] = body;

        // Simulate sending the event
        if (!callback) return;
        if (roomId_.empty()) {
            callback(false, "No room ID set", nullptr);
            return;
        }

        // In a real implementation this would call the homeserver API
        // For now, we simulate success with a local echo event
        callback(true, "", nullptr);
    }

    void sendFormattedMessage(const std::string& body, const std::string& formattedBody,
        SendCallback callback) override {
        if (body.empty()) {
            callback(false, "Cannot send empty message", nullptr);
            return;
        }

        json content;
        content["msgtype"] = "m.text";
        content["body"] = body;
        content["format"] = "org.matrix.custom.html";
        content["formatted_body"] = formattedBody;

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendReaction(const std::string& eventId, const std::string& reactionKey,
        SendCallback callback) override {
        if (eventId.empty()) {
            callback(false, "Event ID is required for reaction", nullptr);
            return;
        }
        if (reactionKey.empty()) {
            callback(false, "Reaction key is required", nullptr);
            return;
        }

        json content;
        content["m.relates_to"] = {
            {"event_id", eventId},
            {"rel_type", "m.annotation"},
            {"key", reactionKey}
        };

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendRedaction(const std::string& eventId, const std::string& reason,
        SendCallback callback) override {
        if (eventId.empty()) {
            callback(false, "Event ID is required for redaction", nullptr);
            return;
        }

        json content;
        if (!reason.empty()) {
            content["reason"] = reason;
        }

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendEvent(EventType type, const std::string& contentJson, SendCallback callback) override {
        if (contentJson.empty()) {
            callback(false, "Content JSON is required", nullptr);
            return;
        }

        // Validate JSON
        try {
            json parsed = json::parse(contentJson);
        } catch (const json::parse_error& e) {
            callback(false, std::string("Invalid JSON: ") + e.what(), nullptr);
            return;
        }

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendMedia(const std::string& url, const std::string& filename, const std::string& mimeType,
        SendCallback callback) override {
        if (url.empty()) {
            callback(false, "URL is required for media", nullptr);
            return;
        }
        if (!url.empty() && url.find("mxc://") != 0) {
            callback(false, "Invalid media URL: must start with mxc://", nullptr);
            return;
        }

        json content;
        content["msgtype"] = "m.file";
        content["url"] = url;
        content["body"] = filename.empty() ? url : filename;
        if (!mimeType.empty()) {
            content["info"] = {{"mimetype", mimeType}};
        }

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendLocation(const LocationData& location, SendCallback callback) override {
        if (location.latitude == 0.0 && location.longitude == 0.0) {
            callback(false, "Invalid location: coordinates cannot be zero", nullptr);
            return;
        }

        json content;
        content["msgtype"] = "m.location";
        content["body"] = location.description.empty()
            ? "Location"
            : location.description;
        content["geo_uri"] = location.geoUri.empty()
            ? ("geo:" + std::to_string(location.latitude) + "," + std::to_string(location.longitude))
            : location.geoUri;

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendPoll(const PollQuestion& poll, SendCallback callback) override {
        if (poll.question.empty()) {
            callback(false, "Poll question is required", nullptr);
            return;
        }
        if (poll.answers.size() < 2) {
            callback(false, "Poll must have at least 2 answers", nullptr);
            return;
        }

        json content;
        content["m.poll.start"] = {
            {"question", {
                {"body", poll.question},
                {"kind", "m.text"},
                {"max_selections", poll.maxSelections}
            }},
            {"answers", json::array()}
        };

        for (const auto& answer : poll.answers) {
            content["m.poll.start"]["answers"].push_back({
                {"id", answer},
                {"m.text", {
                    {"body", answer},
                    {"kind", "m.text"}
                }}
            });
        }

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendPollResponse(const std::string& pollEventId, const std::vector<int>& answers,
        SendCallback callback) override {
        if (pollEventId.empty()) {
            callback(false, "Poll event ID is required", nullptr);
            return;
        }
        if (answers.empty()) {
            callback(false, "At least one answer must be selected", nullptr);
            return;
        }

        json content;
        content["m.relates_to"] = {
            {"event_id", pollEventId},
            {"rel_type", "m.reference"}
        };
        content["m.poll.response"] = {
            {"answers", answers}
        };

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendReply(const std::string& eventId, const std::string& body,
        SendCallback callback) override {
        if (eventId.empty()) {
            callback(false, "Event ID to reply to is required", nullptr);
            return;
        }
        if (body.empty()) {
            callback(false, "Reply body is required", nullptr);
            return;
        }

        json content;
        content["msgtype"] = "m.text";
        content["body"] = body;
        content["m.relates_to"] = {
            {"m.in_reply_to", {{"event_id", eventId}}}
        };

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendEdit(const std::string& eventId, const std::string& newBody,
        SendCallback callback) override {
        if (eventId.empty()) {
            callback(false, "Event ID to edit is required", nullptr);
            return;
        }
        if (newBody.empty()) {
            callback(false, "New body is required", nullptr);
            return;
        }

        json content;
        content["msgtype"] = "m.text";
        content["body"] = " * " + newBody;
        content["m.new_content"] = {
            {"msgtype", "m.text"},
            {"body", newBody}
        };
        content["m.relates_to"] = {
            {"event_id", eventId},
            {"rel_type", "m.replace"}
        };

        if (!callback) return;
        callback(true, "", nullptr);
    }

    void sendReadReceipt(const std::string& eventId, SimpleCallback callback) override {
        if (eventId.empty()) {
            callback(false, "Event ID is required for read receipt");
            return;
        }
        // In a real implementation this would call /rooms/{roomId}/receipt/m.read/{eventId}
        if (!callback) return;
        callback(true, "");
    }

    void sendReadMarker(const std::string& eventId, SimpleCallback callback) override {
        if (eventId.empty()) {
            callback(false, "Event ID is required for read marker");
            return;
        }
        // In a real implementation this would call /rooms/{roomId}/read_markers
        if (!callback) return;
        callback(true, "");
    }

    void cancelSend(const std::string& transactionId) override {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        auto it = pendingTransactions_.find(transactionId);
        if (it != pendingTransactions_.end()) {
            pendingTransactions_.erase(it);
        }
    }

    void resend(const std::string& transactionId, SendCallback callback) override {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        auto it = pendingTransactions_.find(transactionId);
        if (it == pendingTransactions_.end()) {
            callback(false, "Transaction not found: " + transactionId, nullptr);
            return;
        }
        // Re-send the stored content
        callback(true, "", nullptr);
    }

    void deleteUnsent(const std::string& transactionId) override {
        cancelSend(transactionId);
    }

private:
    std::string roomId_;
    mutable std::mutex pendingMutex_;
    std::map<std::string, std::string> pendingTransactions_; // txnId -> contentJson
};

// ============================================================================
// MembershipServiceImpl
// ============================================================================
class MembershipServiceImpl : public MembershipService {
public:
    explicit MembershipServiceImpl(const std::string& roomId, const std::string& myUserId)
        : roomId_(roomId), myUserId_(myUserId), joinedCount_(0), invitedCount_(0) {}

    void loadMembers(std::function<void(bool, const std::string&, const std::vector<RoomMember>&)> callback) override {
        if (!callback) return;
        std::lock_guard<std::mutex> lock(mutex_);
        // In a real impl this would call /rooms/{roomId}/members
        std::vector<RoomMember> members;
        members.reserve(membersCache_.size());
        for (const auto& pair : membersCache_) {
            members.push_back(pair.second);
        }
        callback(true, "", members);
    }

    std::vector<RoomMember> getCachedMembers() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<RoomMember> result;
        result.reserve(membersCache_.size());
        for (const auto& pair : membersCache_) {
            result.push_back(pair.second);
        }
        return result;
    }

    std::optional<RoomMember> getMember(const std::string& userId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = membersCache_.find(userId);
        if (it != membersCache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    int getNumberOfJoinedMembers() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return joinedCount_;
    }

    int getNumberOfInvitedMembers() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return invitedCount_;
    }

    std::vector<std::string> getJoinedMemberIds() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> result;
        for (const auto& id : joinedMemberIds_) {
            result.push_back(id);
        }
        return result;
    }

    void invite(const std::string& userId, const std::string& reason, SimpleCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required to invite");
            return;
        }
        // In a real impl this would call POST /rooms/{roomId}/invite
        std::lock_guard<std::mutex> lock(mutex_);
        RoomMember member;
        member.userId = userId;
        member.membership = Membership::INVITE;
        member.reason = reason;
        membersCache_[userId] = member;
        invitedCount_++;
        callback(true, "");
    }

    void kick(const std::string& userId, const std::string& reason, SimpleCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required to kick");
            return;
        }
        // In a real impl this would call POST /rooms/{roomId}/kick
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = membersCache_.find(userId);
        if (it != membersCache_.end()) {
            if (it->second.membership == Membership::JOIN) {
                joinedCount_--;
                auto jit = std::find(joinedMemberIds_.begin(), joinedMemberIds_.end(), userId);
                if (jit != joinedMemberIds_.end()) joinedMemberIds_.erase(jit);
            }
            it->second.membership = Membership::LEAVE;
            it->second.reason = reason;
        }
        callback(true, "");
    }

    void ban(const std::string& userId, const std::string& reason, SimpleCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required to ban");
            return;
        }
        // In a real impl this would call POST /rooms/{roomId}/ban
        std::lock_guard<std::mutex> lock(mutex_);
        RoomMember member;
        member.userId = userId;
        member.membership = Membership::BAN;
        member.reason = reason;
        membersCache_[userId] = member;
        // Remove from joined counts if present
        auto it = std::find(joinedMemberIds_.begin(), joinedMemberIds_.end(), userId);
        if (it != joinedMemberIds_.end()) {
            joinedMemberIds_.erase(it);
            joinedCount_--;
        }
        callback(true, "");
    }

    void unban(const std::string& userId, SimpleCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required to unban");
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = membersCache_.find(userId);
        if (it != membersCache_.end() && it->second.membership == Membership::BAN) {
            it->second.membership = Membership::LEAVE;
        }
        callback(true, "");
    }

    void join(SimpleCallback callback) override {
        if (roomId_.empty()) {
            callback(false, "No room ID to join");
            return;
        }
        // In a real impl this would call POST /join/{roomId}
        callback(true, "");
    }

    void leave(SimpleCallback callback) override {
        if (roomId_.empty()) {
            callback(false, "No room ID to leave");
            return;
        }
        // In a real impl this would call POST /rooms/{roomId}/leave
        callback(true, "");
    }

    void forget(SimpleCallback callback) override {
        if (roomId_.empty()) {
            callback(false, "No room ID to forget");
            return;
        }
        // In a real impl this would call POST /rooms/{roomId}/forget
        callback(true, "");
    }

    void acceptInvite(SimpleCallback callback) override {
        join(callback);
    }

    void rejectInvite(SimpleCallback callback) override {
        leave(callback);
    }

    bool isUserIgnored(const std::string& userId) const override {
        std::lock_guard<std::mutex> lock(ignoreMutex_);
        return ignoredUsers_.find(userId) != ignoredUsers_.end();
    }

    void ignoreUser(const std::string& userId, SimpleCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required to ignore");
            return;
        }
        std::lock_guard<std::mutex> lock(ignoreMutex_);
        ignoredUsers_.insert(userId);
        callback(true, "");
    }

    void unignoreUser(const std::string& userId, SimpleCallback callback) override {
        if (userId.empty()) {
            callback(false, "User ID is required to unignore");
            return;
        }
        std::lock_guard<std::mutex> lock(ignoreMutex_);
        ignoredUsers_.erase(userId);
        callback(true, "");
    }

    // ── Internal helpers ──
    void addMember(const RoomMember& member) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = membersCache_.find(member.userId);
        bool wasJoined = (it != membersCache_.end() && it->second.membership == Membership::JOIN);
        membersCache_[member.userId] = member;
        if (member.membership == Membership::JOIN) {
            if (!wasJoined) joinedCount_++;
            if (std::find(joinedMemberIds_.begin(), joinedMemberIds_.end(), member.userId) == joinedMemberIds_.end()) {
                joinedMemberIds_.push_back(member.userId);
            }
        } else if (wasJoined && member.membership != Membership::JOIN) {
            joinedCount_--;
            auto jit = std::find(joinedMemberIds_.begin(), joinedMemberIds_.end(), member.userId);
            if (jit != joinedMemberIds_.end()) joinedMemberIds_.erase(jit);
        }
        if (member.membership == Membership::INVITE) invitedCount_++;
    }

private:
    std::string roomId_;
    std::string myUserId_;
    mutable std::mutex mutex_;
    std::map<std::string, RoomMember> membersCache_;
    std::vector<std::string> joinedMemberIds_;
    int joinedCount_;
    int invitedCount_;

    mutable std::mutex ignoreMutex_;
    std::set<std::string> ignoredUsers_;
};

// ============================================================================
// StateServiceImpl
// ============================================================================
class StateServiceImpl : public StateService {
public:
    explicit StateServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    std::string getStateEvent(const std::string& eventType, const std::string& stateKey) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string key = stateKey.empty() ? eventType : (eventType + "/" + stateKey);
        auto it = stateEvents_.find(key);
        if (it != stateEvents_.end()) {
            return it->second;
        }
        return "{}";
    }

    std::string getRoomName() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.name");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("name")) return j["name"].get<std::string>();
            } catch (...) {}
        }
        return "";
    }

    std::string getRoomTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.topic");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("topic")) return j["topic"].get<std::string>();
            } catch (...) {}
        }
        return "";
    }

    std::string getCanonicalAlias() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.canonical_alias");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("alias")) return j["alias"].get<std::string>();
            } catch (...) {}
        }
        return "";
    }

    std::vector<std::string> getAliases() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.aliases");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("aliases") && j["aliases"].is_array()) {
                    return j["aliases"].get<std::vector<std::string>>();
                }
            } catch (...) {}
        }
        return {};
    }

    std::string getRoomAvatarUrl() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.avatar");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("url")) return j["url"].get<std::string>();
            } catch (...) {}
        }
        return "";
    }

    std::string getJoinRule() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.join_rules");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("join_rule")) return j["join_rule"].get<std::string>();
            } catch (...) {}
        }
        return "invite";
    }

    std::string getGuestAccess() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.guest_access");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("guest_access")) return j["guest_access"].get<std::string>();
            } catch (...) {}
        }
        return "forbidden";
    }

    std::string getHistoryVisibility() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.history_visibility");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("history_visibility")) return j["history_visibility"].get<std::string>();
            } catch (...) {}
        }
        return "shared";
    }

    PowerLevels getPowerLevels() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.power_levels");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                PowerLevels pl;
                if (j.contains("users_default")) pl.usersDefault = j["users_default"].get<int>();
                if (j.contains("events_default")) pl.eventsDefault = j["events_default"].get<int>();
                if (j.contains("state_default")) pl.stateDefault = j["state_default"].get<int>();
                if (j.contains("ban")) pl.ban = j["ban"].get<int>();
                if (j.contains("kick")) pl.kick = j["kick"].get<int>();
                if (j.contains("redact")) pl.redact = j["redact"].get<int>();
                if (j.contains("invite")) pl.invite = j["invite"].get<int>();
                if (j.contains("users") && j["users"].is_object()) {
                    for (auto& [uid, level] : j["users"].items()) {
                        pl.users[uid] = level.get<int>();
                    }
                }
                if (j.contains("events") && j["events"].is_object()) {
                    for (auto& [etype, level] : j["events"].items()) {
                        pl.events[etype] = level.get<int>();
                    }
                }
                return pl;
            } catch (...) {}
        }
        return PowerLevels{};
    }

    std::string getEncryptionAlgorithm() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.encryption");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("algorithm")) return j["algorithm"].get<std::string>();
            } catch (...) {}
        }
        return "";
    }

    std::string getCreateEventContent() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.create");
        if (it != stateEvents_.end()) {
            return it->second;
        }
        return "{}";
    }

    std::string getRoomVersion() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.create");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("room_version")) return j["room_version"].get<std::string>();
            } catch (...) {}
        }
        return "1";
    }

    std::string getPinnedEvents() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.pinned_events");
        if (it != stateEvents_.end()) {
            return it->second;
        }
        return "{}";
    }

    std::string getServerAcl() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.server_acl");
        if (it != stateEvents_.end()) {
            return it->second;
        }
        return "{}";
    }

    std::string getRoomType() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.create");
        if (it != stateEvents_.end()) {
            try {
                json j = json::parse(it->second);
                if (j.contains("type")) return j["type"].get<std::string>();
            } catch (...) {}
        }
        return "";
    }

    std::string getTombstoneContent() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stateEvents_.find("m.room.tombstone");
        if (it != stateEvents_.end()) {
            return it->second;
        }
        return "{}";
    }

    // ── Setters ──
    void setRoomName(const std::string& name, SimpleCallback callback) override {
        json content;
        content["name"] = name;
        updateStateEvent("m.room.name", "", content.dump(), callback);
    }

    void setRoomTopic(const std::string& topic, SimpleCallback callback) override {
        json content;
        content["topic"] = topic;
        updateStateEvent("m.room.topic", "", content.dump(), callback);
    }

    void setCanonicalAlias(const std::string& alias, SimpleCallback callback) override {
        json content;
        content["alias"] = alias;
        updateStateEvent("m.room.canonical_alias", "", content.dump(), callback);
    }

    void setRoomAvatar(const std::string& mxcUrl, SimpleCallback callback) override {
        json content;
        content["url"] = mxcUrl;
        updateStateEvent("m.room.avatar", "", content.dump(), callback);
    }

    void setJoinRule(const std::string& rule, SimpleCallback callback) override {
        if (rule != "public" && rule != "knock" && rule != "invite" && rule != "private" && rule != "restricted") {
            callback(false, "Invalid join rule: " + rule);
            return;
        }
        json content;
        content["join_rule"] = rule;
        updateStateEvent("m.room.join_rules", "", content.dump(), callback);
    }

    void setGuestAccess(const std::string& access, SimpleCallback callback) override {
        if (access != "can_join" && access != "forbidden") {
            callback(false, "Invalid guest access: " + access);
            return;
        }
        json content;
        content["guest_access"] = access;
        updateStateEvent("m.room.guest_access", "", content.dump(), callback);
    }

    void setHistoryVisibility(const std::string& visibility, SimpleCallback callback) override {
        if (visibility != "invited" && visibility != "joined" && visibility != "shared" && visibility != "world_readable") {
            callback(false, "Invalid history visibility: " + visibility);
            return;
        }
        json content;
        content["history_visibility"] = visibility;
        updateStateEvent("m.room.history_visibility", "", content.dump(), callback);
    }

    void setPowerLevels(const PowerLevels& levels, SimpleCallback callback) override {
        json content;
        content["users_default"] = levels.usersDefault;
        content["events_default"] = levels.eventsDefault;
        content["state_default"] = levels.stateDefault;
        content["ban"] = levels.ban;
        content["kick"] = levels.kick;
        content["redact"] = levels.redact;
        content["invite"] = levels.invite;
        content["users"] = json::object();
        for (const auto& [uid, level] : levels.users) {
            content["users"][uid] = level;
        }
        content["events"] = json::object();
        for (const auto& [etype, level] : levels.events) {
            content["events"][etype] = level;
        }
        updateStateEvent("m.room.power_levels", "", content.dump(), callback);
    }

    void enableEncryption(SimpleCallback callback) override {
        json content;
        content["algorithm"] = "m.megolm.v1.aes-sha2";
        content["rotation_period_ms"] = 604800000;
        content["rotation_period_msgs"] = 100;
        updateStateEvent("m.room.encryption", "", content.dump(), callback);
    }

    void sendStateEvent(const std::string& eventType, const std::string& stateKey,
        const std::string& contentJson, SimpleCallback callback) override {
        if (eventType.empty()) {
            callback(false, "Event type is required");
            return;
        }
        // Validate JSON
        try {
            json parsed = json::parse(contentJson);
        } catch (const json::parse_error& e) {
            callback(false, std::string("Invalid content JSON: ") + e.what());
            return;
        }
        updateStateEvent(eventType, stateKey, contentJson, callback);
    }

    // ── Internal helpers ──
    void setStateEventRaw(const std::string& eventType, const std::string& stateKey,
        const std::string& contentJson) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string key = stateKey.empty() ? eventType : (eventType + "/" + stateKey);
        stateEvents_[key] = contentJson;
    }

private:
    void updateStateEvent(const std::string& eventType, const std::string& stateKey,
        const std::string& contentJson, SimpleCallback callback) {
        // In a real impl this would call PUT /rooms/{roomId}/state/{eventType}/{stateKey}
        setStateEventRaw(eventType, stateKey, contentJson);
        if (callback) callback(true, "");
    }

    std::string roomId_;
    mutable std::mutex mutex_;
    std::map<std::string, std::string> stateEvents_; // key = eventType[/stateKey] -> content JSON
};

// ============================================================================
// Factory functions for services
// ============================================================================

std::shared_ptr<TimelineService> createTimelineService(const std::string& roomId) {
    return std::make_shared<TimelineServiceImpl>(roomId);
}

std::shared_ptr<SendService> createSendService(const std::string& roomId) {
    return std::make_shared<SendServiceImpl>(roomId);
}

std::shared_ptr<MembershipService> createMembershipService(const std::string& roomId, const std::string& myUserId) {
    return std::make_shared<MembershipServiceImpl>(roomId, myUserId);
}

std::shared_ptr<StateService> createStateService(const std::string& roomId) {
    return std::make_shared<StateServiceImpl>(roomId);
}

} // namespace matrix_sdk
