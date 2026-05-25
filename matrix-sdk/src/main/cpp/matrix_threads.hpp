#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct ThreadInfo {
    std::string threadId; std::string rootEventId; std::string roomId;
    std::string rootMessage; std::string rootSender;
    int replyCount = 0; uint64_t lastReplyTs = 0; bool hasUnread = false;
};

class ThreadManager {
public:
    ThreadManager(); ~ThreadManager();
    void addThread(const std::string& roomId, const ThreadInfo& thread);
    void removeThread(const std::string& roomId, const std::string& threadId);
    std::vector<ThreadInfo> getThreads(const std::string& roomId);
    int threadCount(const std::string& roomId) const;
    void incrementReplyCount(const std::string& roomId, const std::string& threadId);
    ThreadInfo* findThread(const std::string& roomId, const std::string& threadId);
private:
    std::map<std::string, std::map<std::string, ThreadInfo>> m_threads;
};
} // namespace matrix_sdk
