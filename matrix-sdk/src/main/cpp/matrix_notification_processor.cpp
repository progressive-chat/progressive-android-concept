#include "matrix_notification_processor.hpp"
#include "matrix_push_rules.hpp"
#include <android/log.h>

#define LOG_TAG "NotifProcessor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

NotificationProcessor::NotificationProcessor() = default;
NotificationProcessor::~NotificationProcessor() = default;

void NotificationProcessor::processNotification(const std::string& roomId, const json& event) {
    std::string eventId = event.value("event_id", "");
    std::string sender = event.value("sender", "");
    std::string type = event.value("type", "");
    auto content = event.value("content", json::object());

    m_roomUnread[roomId]++;

    bool isHighlight = false;
    if (m_evaluator) {
        isHighlight = m_evaluator->shouldNotify(roomId, sender, type, content);
    }
    if (isHighlight) m_roomHighlights[roomId]++;

    if (m_callback && type == "m.room.message") {
        NotificationInfo info;
        info.eventId = eventId; info.roomId = roomId; info.sender = sender;
        info.body = content.value("body", "");
        info.timestamp = event.value("origin_server_ts", 0ULL);
        info.isHighlight = isHighlight; info.badgeCount = getTotalUnread();
        m_callback(info);
    }
}

int NotificationProcessor::getTotalUnread() const {
    int total = 0;
    for (auto& [roomId, count] : m_roomUnread) total += count;
    return total;
}

int NotificationProcessor::getHighlightCount() const {
    int total = 0;
    for (auto& [roomId, count] : m_roomHighlights) total += count;
    return total;
}

int NotificationProcessor::getRoomUnread(const std::string& roomId) const {
    auto it = m_roomUnread.find(roomId);
    return it != m_roomUnread.end() ? it->second : 0;
}

void NotificationProcessor::clearRoom(const std::string& roomId) {
    m_roomUnread[roomId] = 0;
    m_roomHighlights[roomId] = 0;
}

void NotificationProcessor::onNotification(std::function<void(const NotificationInfo&)> cb) {
    m_callback = std::move(cb);
}

void NotificationProcessor::setPushRuleEvaluator(std::shared_ptr<PushRuleEvaluator> evaluator) {
    m_evaluator = std::move(evaluator);
}

} // namespace matrix_sdk
