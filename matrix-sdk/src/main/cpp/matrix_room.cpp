#include "matrix_room.hpp"
#include <algorithm>
#include <android/log.h>

#define LOG_TAG "MatrixRoom"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

MatrixRoom::MatrixRoom(const std::string& roomId) { m_info.roomId = roomId; }
MatrixRoom::~MatrixRoom() = default;

void MatrixRoom::updateFromSync(const json& roomData) {
    if (roomData.contains("unread_notifications")) {
        auto& unread = roomData["unread_notifications"];
        m_info.notificationCount = unread.value("notification_count", 0);
        m_info.highlightCount = unread.value("highlight_count", 0);
    }
    if (roomData.contains("summary")) {
        auto& sum = roomData["summary"];
        m_info.joinedMembers = sum.value("m.joined_member_count", 0);
        m_info.invitedMembers = sum.value("m.invited_member_count", 0);
    }
}

void MatrixRoom::addTimelineEvent(const RoomEvent& event) {
    m_timeline.push_back(event);
    if (event.originServerTs > m_info.lastEventTimestamp) {
        m_info.lastEventTimestamp = event.originServerTs;
    }
    if (m_timeline.size() > 1000) {
        m_timeline.erase(m_timeline.begin(), m_timeline.begin() + 200);
    }
}

void MatrixRoom::addStateEvent(const RoomEvent& event) {
    if (event.type == "m.room.name") {
        m_info.name = event.content.value("name", "");
    } else if (event.type == "m.room.topic") {
        m_info.topic = event.content.value("topic", "");
    } else if (event.type == "m.room.avatar") {
        m_info.avatarUrl = event.content.value("url", "");
    } else if (event.type == "m.room.canonical_alias") {
        m_info.canonicalAlias = event.content.value("alias", "");
    } else if (event.type == "m.room.encryption") {
        m_info.isEncrypted = true;
    } else if (event.type == "m.room.member") {
        RoomMember member;
        member.userId = event.stateKey;
        member.displayName = event.content.value("displayname", "");
        member.avatarUrl = event.content.value("avatar_url", "");
        std::string mship = event.content.value("membership", "leave");
        if (mship == "join") member.membership = RoomMembership::JOIN;
        else if (mship == "invite") member.membership = RoomMembership::INVITE;
        else if (mship == "leave") member.membership = RoomMembership::LEAVE;
        else if (mship == "ban") member.membership = RoomMembership::BAN;
        else if (mship == "knock") member.membership = RoomMembership::KNOCK;
        m_members.push_back(member);
    }
}

std::vector<RoomEvent> MatrixRoom::getTimeline(int limit, const std::string& from) {
    std::vector<RoomEvent> events;
    int start = 0;
    if (!from.empty()) {
        auto it = std::find_if(m_timeline.begin(), m_timeline.end(),
            [&](auto& e) { return e.eventId == from; });
        if (it != m_timeline.end()) start = std::distance(m_timeline.begin(), it) + 1;
    }
    int count = std::min(limit, static_cast<int>(m_timeline.size()) - start);
    for (int i = m_timeline.size() - count; i < static_cast<int>(m_timeline.size()); i++) {
        events.push_back(m_timeline[i]);
    }
    return events;
}

RoomMember* MatrixRoom::findMember(const std::string& userId) {
    for (auto& m : m_members) if (m.userId == userId) return &m;
    return nullptr;
}

std::string MatrixRoom::getDisplayName() const {
    if (!m_info.name.empty()) return m_info.name;
    if (!m_info.canonicalAlias.empty()) return m_info.canonicalAlias;
    return m_info.roomId;
}

std::string MatrixRoom::getAvatarUrl() const {
    return m_info.avatarUrl;
}

void MatrixRoom::setPowerLevels(const json& powerLevels) {
    m_powerLevels = powerLevels;
}

} // namespace matrix_sdk
