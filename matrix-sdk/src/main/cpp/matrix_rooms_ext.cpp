#include "matrix_client.hpp"
#include "matrix_room.hpp"
#include "matrix_session.hpp"
#include <android/log.h>
#include <algorithm>
#include <sstream>

#define LOG_TAG "MatrixRooms"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

// ==== Room join rules helpers ====

static std::string joinRuleToString(const std::string& rule) {
    if (rule == "public") return "Anyone can join";
    if (rule == "knock") return "Anyone can ask to join";
    if (rule == "invite") return "Invite only";
    if (rule == "private") return "Private";
    if (rule == "restricted") return "Restricted";
    return rule;
}

static std::string guestAccessToString(const std::string& access) {
    if (access == "can_join") return "Guests can join";
    if (access == "forbidden") return "No guests";
    return access;
}

static std::string historyVisibilityToString(const std::string& vis) {
    if (vis == "world_readable") return "Anyone can read history";
    if (vis == "shared") return "Members can read history";
    if (vis == "invited") return "Members since invite can read history";
    if (vis == "joined") return "Members since join can read history";
    return vis;
}

// ==== Room suggestion scoring ====

struct RoomSuggestion {
    std::string roomId; std::string name; int score = 0;
    std::string reason;
};

class RoomSuggester {
public:
    static std::vector<RoomSuggestion> suggest(
        const std::vector<MatrixRoom*>& rooms, const std::string& query) {
        std::vector<RoomSuggestion> suggestions;

        std::string q = query;
        std::transform(q.begin(), q.end(), q.begin(), ::tolower);

        for (auto* room : rooms) {
            if (!room) continue;
            RoomSuggestion s;
            s.roomId = room->roomId();
            s.name = room->getDisplayName();
            int score = 0;

            std::string nameLower = s.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            // Exact name match
            if (nameLower == q) score += 100;
            // Name starts with query
            else if (nameLower.find(q) == 0) score += 80;
            // Name contains query
            else if (nameLower.find(q) != std::string::npos) score += 50;
            // Room ID match
            else if (room->roomId().find(q) != std::string::npos) score += 30;

            // Boost for active rooms
            if (room->unreadCount() > 0) score += 20;
            if (room->highlightCount() > 0) score += 30;
            if (room->isDirect()) score += 10;

            s.score = score;
            suggestions.push_back(s);
        }

        // Sort by score descending
        std::sort(suggestions.begin(), suggestions.end(),
            [](auto& a, auto& b) { return a.score > b.score; });

        // Filter zero-score
        suggestions.erase(std::remove_if(suggestions.begin(), suggestions.end(),
            [](auto& s) { return s.score == 0; }), suggestions.end());

        return suggestions;
    }
};

// ==== Room sorting strategies ====

enum class RoomSortOrder {
    RECENT,      // By last message time
    ALPHABETICAL, // A-Z
    UNREAD,      // Unread count
    PRIORITY,    // Highlight -> unread -> recent
    ACTIVITY,    // By activity level
    MANUAL       // Custom order
};

class RoomSorter {
public:
    static void sort(std::vector<MatrixRoom*>& rooms, RoomSortOrder order) {
        switch (order) {
            case RoomSortOrder::RECENT:
                std::sort(rooms.begin(), rooms.end(), [](auto* a, auto* b) {
                    return a && b && a->info().lastEventTimestamp > b->info().lastEventTimestamp;
                });
                break;
            case RoomSortOrder::ALPHABETICAL:
                std::sort(rooms.begin(), rooms.end(), [](auto* a, auto* b) {
                    return a && b && a->getDisplayName() < b->getDisplayName();
                });
                break;
            case RoomSortOrder::UNREAD:
                std::sort(rooms.begin(), rooms.end(), [](auto* a, auto* b) {
                    return a && b && a->unreadCount() > b->unreadCount();
                });
                break;
            case RoomSortOrder::PRIORITY:
                std::sort(rooms.begin(), rooms.end(), [](auto* a, auto* b) {
                    if (!a || !b) return false;
                    int pa = a->highlightCount() * 1000 + a->unreadCount();
                    int pb = b->highlightCount() * 1000 + b->unreadCount();
                    if (pa != pb) return pa > pb;
                    return a->info().lastEventTimestamp > b->info().lastEventTimestamp;
                });
                break;
            case RoomSortOrder::ACTIVITY:
                // Sort by recent activity but favor rooms with unread
                std::sort(rooms.begin(), rooms.end(), [](auto* a, auto* b) {
                    if (!a || !b) return false;
                    bool aHasUnread = a->unreadCount() > 0;
                    bool bHasUnread = b->unreadCount() > 0;
                    if (aHasUnread != bHasUnread) return aHasUnread;
                    return a->info().lastEventTimestamp > b->info().lastEventTimestamp;
                });
                break;
            case RoomSortOrder::MANUAL:
                break;
        }
    }
};

// ==== Room section categorizer ====

enum class RoomSection { FAVORITES, DIRECT_MESSAGES, ROOMS, SPACES, LOW_PRIORITY, INVITES, ARCHIVED };

class RoomSectionCategorizer {
public:
    static RoomSection categorize(MatrixRoom* room) {
        if (!room) return RoomSection::ARCHIVED;

        auto membership = room->info().membership;
        if (membership == RoomMembership::INVITE) return RoomSection::INVITES;
        if (membership == RoomMembership::LEAVE) return RoomSection::ARCHIVED;

        if (room->isDirect()) return RoomSection::DIRECT_MESSAGES;
        if (room->info().isSpace) return RoomSection::SPACES;

        return RoomSection::ROOMS;
    }

    static std::string sectionName(RoomSection section) {
        switch (section) {
            case RoomSection::FAVORITES: return "Favorites";
            case RoomSection::DIRECT_MESSAGES: return "People";
            case RoomSection::ROOMS: return "Rooms";
            case RoomSection::SPACES: return "Spaces";
            case RoomSection::LOW_PRIORITY: return "Low Priority";
            case RoomSection::INVITES: return "Invites";
            case RoomSection::ARCHIVED: return "Archived";
        }
        return "Unknown";
    }

    static int sectionOrder(RoomSection section) {
        switch (section) {
            case RoomSection::FAVORITES: return 0;
            case RoomSection::INVITES: return 1;
            case RoomSection::DIRECT_MESSAGES: return 2;
            case RoomSection::ROOMS: return 3;
            case RoomSection::SPACES: return 4;
            case RoomSection::LOW_PRIORITY: return 5;
            case RoomSection::ARCHIVED: return 6;
        }
        return 99;
    }
};

// ==== Room encryption status ====

struct RoomEncryptionInfo {
    bool isEncrypted = false;
    std::string algorithm;
    int rotationPeriodMsgs = 0;
    int rotationPeriodMs = 0;
};

class RoomEncryptionHelper {
public:
    static RoomEncryptionInfo parseEncryptionEvent(const json& content) {
        RoomEncryptionInfo info;
        info.isEncrypted = true;
        info.algorithm = content.value("algorithm", "m.megolm.v1.aes-sha2");
        info.rotationPeriodMsgs = content.value("rotation_period_msgs", 100);
        info.rotationPeriodMs = content.value("rotation_period_ms", 604800000);
        return info;
    }
};

// ==== Room summary formatter ====

class RoomSummaryFormatter {
public:
    static std::string formatLastMessage(const MatrixRoom* room) {
        if (!room) return "";
        // Would get last timeline event and format it
        return "";
    }

    static std::string formatMemberCount(const MatrixRoom* room) {
        if (!room) return "0";
        return std::to_string(room->info().joinedMembers);
    }

    static std::string formatRoomTopic(const MatrixRoom* room) {
        if (!room) return "";
        return room->info().topic;
    }

    static bool shouldShowTypingIndicator(MatrixRoom* room) {
        if (!room) return false;
        return room->info().joinedMembers > 1;
    }
};

// ==== Room counter ====

class RoomCounter {
public:
    struct Counts {
        int totalJoined = 0;
        int totalInvited = 0;
        int totalLeft = 0;
        int totalUnread = 0;
        int totalHighlight = 0;
        int totalDirect = 0;
        int totalSpaces = 0;
        int totalFavorites = 0;
    };

    static Counts count(const std::vector<MatrixRoom*>& rooms) {
        Counts c;
        for (auto* room : rooms) {
            if (!room) continue;
            auto m = room->info().membership;
            if (m == RoomMembership::JOIN) c.totalJoined++;
            else if (m == RoomMembership::INVITE) c.totalInvited++;
            else if (m == RoomMembership::LEAVE) c.totalLeft++;

            c.totalUnread += room->unreadCount();
            c.totalHighlight += room->highlightCount();
            if (room->isDirect()) c.totalDirect++;
            if (room->info().isSpace) c.totalSpaces++;
        }
        return c;
    }

    static json toJson(const Counts& counts) {
        return {
            {"total_joined", counts.totalJoined},
            {"total_invited", counts.totalInvited},
            {"total_left", counts.totalLeft},
            {"total_unread", counts.totalUnread},
            {"total_highlights", counts.totalHighlight},
            {"total_direct", counts.totalDirect},
            {"total_spaces", counts.totalSpaces},
            {"total_favorites", counts.totalFavorites}
        };
    }
};

// ==== Room previewer ====

class RoomPreviewer {
public:
    struct Preview {
        std::string roomId; std::string name; std::string topic;
        std::string avatarUrl; int memberCount = 0;
        bool isEncrypted = false; bool canJoin = true;
        std::string joinRule; std::string error;
    };

    static Preview preview(const std::string& roomIdOrAlias,
                           const std::string& viaServer = "") {
        Preview p;
        p.roomId = roomIdOrAlias;
        p.name = roomIdOrAlias;
        p.canJoin = true;
        return p;
    }
};

// ==== Room health checker ====

class RoomHealthChecker {
public:
    struct HealthStatus {
        bool ok = true;
        std::string issue;
        std::string suggestion;
    };

    static HealthStatus check(MatrixRoom* room) {
        HealthStatus status;
        if (!room) { status.ok = false; status.issue = "Room is null"; return status; }
        if (room->roomId().empty()) { status.ok = false; status.issue = "Room ID is empty"; return status; }
        return status;
    }
};

} // namespace matrix_sdk
