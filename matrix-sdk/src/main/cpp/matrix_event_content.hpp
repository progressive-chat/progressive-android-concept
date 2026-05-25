#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include <functional>

namespace matrix_sdk {

using json = nlohmann::json;

// ── Event relationship types ──

struct EventRelation {
    std::string type; // "m.annotation", "m.reference", "m.replace", "m.thread"
    std::string eventId; std::string key;
    bool isFallingBack = false;
};

// ── Event content types for display ──

struct DisplayableContent {
    std::string body; std::string formattedBody; std::string format;
    std::string msgType; std::string url; std::string thumbnailUrl;
    std::string mimeType; std::string filename; int64_t size = 0;
    int width = 0, height = 0; int duration = 0;
    bool isEncrypted = false; bool isRedacted = false;
};

// ── Timeline event ──

struct TimelineEvent {
    std::string eventId; std::string roomId; std::string sender;
    std::string type; uint64_t timestamp = 0; uint64_t age = 0;
    json rawContent; DisplayableContent display;
    bool isState = false; std::string stateKey;
    std::string redacts; std::string replacesEvent;
    EventRelation relation; bool canReact = true;
};

// ── Timeline chunk ──

struct TimelineChunk {
    std::string startToken; std::string endToken;
    std::vector<TimelineEvent> events;
    bool isLive = false; bool hasMore = true; bool isInitial = false;
};

// ── Display helper ──

class DisplayableContentExtractor {
public:
    static DisplayableContent extract(const json& content) {
        DisplayableContent dc;
        dc.body = content.value("body", "");
        dc.formattedBody = content.value("formatted_body", "");
        dc.format = content.value("format", "");
        dc.msgType = content.value("msgtype", "m.text");
        dc.url = content.value("url", "");
        dc.thumbnailUrl = content.value("info", json::object()).value("thumbnail_url", "");
        dc.mimeType = content.value("info", json::object()).value("mimetype", "");
        dc.filename = content.value("filename", content.value("body", ""));
        dc.size = content.value("info", json::object()).value("size", 0);
        dc.width = content.value("info", json::object()).value("w", 0);
        dc.height = content.value("info", json::object()).value("h", 0);
        dc.duration = content.value("info", json::object()).value("duration", 0);
        return dc;
    }

    static std::string getSenderDisplayName(const TimelineEvent& event,
                                              const std::map<std::string, std::string>& displayNames) {
        auto it = displayNames.find(event.sender);
        return it != displayNames.end() ? it->second : event.sender;
    }

    static bool isEdited(const TimelineEvent& event) {
        return event.relation.type == "m.replace";
    }

    static bool isReply(const TimelineEvent& event) {
        return event.rawContent.contains("m.relates_to") &&
               event.rawContent["m.relates_to"].contains("m.in_reply_to");
    }

    static std::string getReplyEventId(const TimelineEvent& event) {
        if (isReply(event)) {
            return event.rawContent["m.relates_to"]["m.in_reply_to"]["event_id"];
        }
        return "";
    }
};

// ── Timeline builder from sync data ──

class TimelineBuilder {
public:
    TimelineBuilder() = default;

    TimelineChunk buildFromSync(const json& roomData, const std::string& roomId) {
        TimelineChunk chunk;
        chunk.isLive = true;
        chunk.isInitial = true;

        if (roomData.contains("timeline")) {
            auto& timeline = roomData["timeline"];
            chunk.startToken = timeline.value("prev_batch", "");
            chunk.hasMore = timeline.value("limited", false);

            if (timeline.contains("events")) {
                for (auto& event : timeline["events"]) {
                    TimelineEvent te = parseEvent(event, roomId);
                    chunk.events.push_back(te);
                }
            }
        }
        return chunk;
    }

    TimelineChunk buildFromPagination(const json& response, const std::string& roomId) {
        TimelineChunk chunk;
        chunk.isLive = false;
        chunk.startToken = response.value("start", "");
        chunk.endToken = response.value("end", "");
        chunk.hasMore = !chunk.endToken.empty();

        if (response.contains("chunk")) {
            for (auto& event : response["chunk"]) {
                TimelineEvent te = parseEvent(event, roomId);
                chunk.events.push_back(te);
            }
        }
        return chunk;
    }

    TimelineEvent parseEvent(const json& event, const std::string& roomId) {
        TimelineEvent te;
        te.eventId = event.value("event_id", "");
        te.roomId = roomId;
        te.sender = event.value("sender", "");
        te.type = event.value("type", "");
        te.timestamp = event.value("origin_server_ts", 0ULL);
        te.rawContent = event.value("content", json::object());
        te.display = DisplayableContentExtractor::extract(te.rawContent);
        te.isState = event.contains("state_key");
        te.stateKey = event.value("state_key", "");
        te.redacts = event.value("redacts", "");
        te.canReact = te.type == "m.room.message";

        // Parse relations
        if (te.rawContent.contains("m.relates_to")) {
            auto& rel = te.rawContent["m.relates_to"];
            te.relation.type = rel.value("rel_type", "");
            te.relation.eventId = rel.value("event_id", "");
            te.relation.key = rel.value("key", "");
            te.replacesEvent = te.relation.type == "m.replace" ? te.relation.eventId : "";
        }

        // Age from unsigned data
        if (event.contains("unsigned")) {
            te.age = event["unsigned"].value("age", 0ULL);
        }

        return te;
    }
};

// ── Timeline differ ──

class TimelineDiffer {
public:
    struct DiffResult {
        std::vector<TimelineEvent> added;
        std::vector<TimelineEvent> removed;
        std::vector<TimelineEvent> modified;
    };

    DiffResult diff(const std::vector<TimelineEvent>& oldTimeline,
                     const std::vector<TimelineEvent>& newTimeline) {
        DiffResult result;
        std::set<std::string> oldIds, newIds;

        for (auto& e : oldTimeline) oldIds.insert(e.eventId);
        for (auto& e : newTimeline) newIds.insert(e.eventId);

        for (auto& e : newTimeline) {
            if (oldIds.find(e.eventId) == oldIds.end()) {
                result.added.push_back(e);
            } else {
                result.modified.push_back(e);
            }
        }

        for (auto& e : oldTimeline) {
            if (newIds.find(e.eventId) == newIds.end()) {
                result.removed.push_back(e);
            }
        }

        return result;
    }
};

// ── Timeline deduplicator ──

class TimelineDeduplicator {
public:
    TimelineDeduplicator(size_t maxSize = 10000) : m_maxSize(maxSize) {}

    bool isDuplicate(const std::string& eventId) {
        return m_seen.find(eventId) != m_seen.end();
    }

    void markSeen(const std::string& eventId) {
        m_seen.insert(eventId);
        if (m_seen.size() > m_maxSize) {
            auto it = m_seen.begin();
            m_seen.erase(it);
        }
    }

    void clear() { m_seen.clear(); }
    size_t size() const { return m_seen.size(); }

private:
    std::set<std::string> m_seen;
    size_t m_maxSize;
};

// ── Message grouping ──

struct MessageGroup {
    std::string sender;
    std::vector<TimelineEvent> events;
    bool showAvatar = true;
    bool showTimestamp = true;
    uint64_t startTimestamp = 0;
};

class MessageGrouper {
public:
    MessageGrouper() = default;

    std::vector<MessageGroup> group(const std::vector<TimelineEvent>& events,
                                      int64_t groupWindowMs = 300000) {
        std::vector<MessageGroup> groups;

        for (auto& event : events) {
            if (groups.empty() ||
                groups.back().sender != event.sender ||
                (event.timestamp - groups.back().startTimestamp) > static_cast<uint64_t>(groupWindowMs)) {
                MessageGroup group;
                group.sender = event.sender;
                group.startTimestamp = event.timestamp;
                group.events.push_back(event);
                groups.push_back(group);
            } else {
                groups.back().events.push_back(event);
                groups.back().showAvatar = false;
            }
        }

        return groups;
    }
};

// ── Timeline utilities ──

namespace timeline_utils {

    // Check if two events should be merged visually
    bool shouldMerge(const TimelineEvent& a, const TimelineEvent& b,
                     int64_t mergeWindowMs = 300000) {
        if (a.sender != b.sender) return false;
        if (a.type == "m.room.member" || b.type == "m.room.member") return false;
        if (a.isState || b.isState) return false;
        int64_t diff = static_cast<int64_t>(b.timestamp) - static_cast<int64_t>(a.timestamp);
        return diff >= 0 && diff < mergeWindowMs;
    }

    // Get the best display name for an event
    std::string getDisplayName(const TimelineEvent& event,
                                 const std::map<std::string, std::string>& names) {
        auto it = names.find(event.sender);
        if (it != names.end() && !it->second.empty()) return it->second;
        // Fallback: extract localpart from MXID
        if (event.sender.size() > 1 && event.sender[0] == '@') {
            size_t colon = event.sender.find(':');
            if (colon != std::string::npos) {
                return event.sender.substr(1, colon - 1);
            }
        }
        return event.sender;
    }

    // Format timestamp for display
    std::string formatTimestamp(uint64_t ts) {
        auto seconds = ts / 1000;
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        auto diff = now - static_cast<int64_t>(seconds);

        if (diff < 60) return "Just now";
        if (diff < 3600) return std::to_string(diff / 60) + "m ago";
        if (diff < 86400) return std::to_string(diff / 3600) + "h ago";
        if (diff < 604800) return std::to_string(diff / 86400) + "d ago";
        return std::to_string(diff / 604800) + "w ago";
    }

} // namespace timeline_utils

// ── Event content builder ──

class EventContentBuilder {
public:
    static json buildTextMessage(const std::string& body, const std::string& formatted = "") {
        json content;
        content["msgtype"] = "m.text";
        content["body"] = body;
        if (!formatted.empty()) {
            content["format"] = "org.matrix.custom.html";
            content["formatted_body"] = formatted;
        }
        return content;
    }

    static json buildReply(const std::string& body, const std::string& replyToEventId,
                           const json& replyToContent, const std::string& formatted = "") {
        json content = buildTextMessage(body, formatted);
        json relates;
        relates["m.in_reply_to"]["event_id"] = replyToEventId;
        content["m.relates_to"] = relates;
        return content;
    }

    static json buildReaction(const std::string& eventId, const std::string& key) {
        json content;
        content["m.relates_to"]["rel_type"] = "m.annotation";
        content["m.relates_to"]["event_id"] = eventId;
        content["m.relates_to"]["key"] = key;
        return content;
    }

    static json buildEdit(const std::string& newBody, const std::string& originalEventId,
                          const std::string& formatted = "") {
        json content = buildTextMessage(newBody, formatted);
        content["m.new_content"] = content;
        content["m.relates_to"]["rel_type"] = "m.replace";
        content["m.relates_to"]["event_id"] = originalEventId;
        return content;
    }
};

} // namespace matrix_sdk
