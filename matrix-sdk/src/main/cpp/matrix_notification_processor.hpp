#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct NotificationInfo {
    std::string eventId; std::string roomId; std::string sender;
    std::string roomName; std::string senderName; std::string body;
    uint64_t timestamp = 0; bool isHighlight = false; bool isDirect = false;
    int badgeCount = 0;
};

class NotificationProcessor {
public:
    NotificationProcessor(); ~NotificationProcessor();
    void processNotification(const std::string& roomId, const json& event);
    int getTotalUnread() const;
    int getHighlightCount() const;
    int getRoomUnread(const std::string& roomId) const;
    void clearRoom(const std::string& roomId);
    void onNotification(std::function<void(const NotificationInfo&)> cb);
    void setPushRuleEvaluator(std::shared_ptr<class PushRuleEvaluator> evaluator);
private:
    std::map<std::string, int> m_roomUnread;
    std::map<std::string, int> m_roomHighlights;
    std::shared_ptr<PushRuleEvaluator> m_evaluator;
    std::function<void(const NotificationInfo&)> m_callback;
};
} // namespace matrix_sdk
