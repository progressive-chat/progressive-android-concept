#pragma once
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann::json;

enum class RoomMembership {
    INVITE, JOIN, LEAVE, BAN, KNOCK, NONE
};

struct RoomMember {
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    RoomMembership membership = RoomMembership::NONE;
    int powerLevel = 0;
    std::string reason;
};

struct RoomInfo {
    std::string roomId;
    std::string name;
    std::string topic;
    std::string avatarUrl;
    std::string canonicalAlias;
    std::vector<std::string> aliases;
    int joinedMembers = 0;
    int invitedMembers = 0;
    bool isDirect = false;
    bool isEncrypted = false;
    bool isSpace = false;
    bool isPublic = false;
    RoomMembership membership = RoomMembership::NONE;
    std::string joinRule;
    std::string guestAccess;
    std::string historyVisibility;
    int notificationCount = 0;
    int highlightCount = 0;
    uint64_t lastEventTimestamp = 0;
};

struct RoomEvent {
    std::string eventId;
    std::string roomId;
    std::string sender;
    std::string type;
    json content;
    uint64_t originServerTs = 0;
    uint64_t age = 0;
    std::string transactionId;
    std::string stateKey;
    std::string redacts;
    bool isEncrypted = false;
    bool isState = false;
};

class MatrixRoom {
public:
    MatrixRoom(const std::string& roomId);
    ~MatrixRoom();

    const std::string& roomId() const { return m_info.roomId; }
    const RoomInfo& info() const { return m_info; }

    void updateFromSync(const json& roomData);
    void addTimelineEvent(const RoomEvent& event);
    void addStateEvent(const RoomEvent& event);

    std::vector<RoomEvent> getTimeline(int limit = 50, const std::string& from = "");
    std::vector<RoomMember> getMembers() const { return m_members; }
    RoomMember* findMember(const std::string& userId);

    int unreadCount() const { return m_info.notificationCount; }
    int highlightCount() const { return m_info.highlightCount; }

    std::string getDisplayName() const;
    std::string getAvatarUrl() const;

    bool isEncrypted() const { return m_info.isEncrypted; }
    bool isDirect() const { return m_info.isDirect; }

    void setPowerLevels(const json& powerLevels);

private:
    RoomInfo m_info;
    std::vector<RoomEvent> m_timeline;
    std::vector<RoomMember> m_members;
    json m_powerLevels;
};

} // namespace matrix_sdk
