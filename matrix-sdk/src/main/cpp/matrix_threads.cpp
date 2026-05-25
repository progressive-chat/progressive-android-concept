#include "matrix_threads.hpp"
namespace matrix_sdk {

ThreadManager::ThreadManager() = default;
ThreadManager::~ThreadManager() = default;

void ThreadManager::addThread(const std::string& roomId, const ThreadInfo& thread) {
    m_threads[roomId][thread.threadId] = thread;
}

void ThreadManager::removeThread(const std::string& roomId, const std::string& threadId) {
    auto it = m_threads.find(roomId);
    if (it != m_threads.end()) it->second.erase(threadId);
}

std::vector<ThreadInfo> ThreadManager::getThreads(const std::string& roomId) {
    std::vector<ThreadInfo> result;
    auto it = m_threads.find(roomId);
    if (it != m_threads.end()) for (auto& [id, thread] : it->second) result.push_back(thread);
    return result;
}

int ThreadManager::threadCount(const std::string& roomId) const {
    auto it = m_threads.find(roomId);
    return it != m_threads.end() ? it->second.size() : 0;
}

void ThreadManager::incrementReplyCount(const std::string& roomId, const std::string& threadId) {
    auto* t = findThread(roomId, threadId);
    if (t) t->replyCount++;
}

ThreadInfo* ThreadManager::findThread(const std::string& roomId, const std::string& threadId) {
    auto it = m_threads.find(roomId);
    if (it != m_threads.end()) {
        auto jt = it->second.find(threadId);
        if (jt != it->second.end()) return &jt->second;
    }
    return nullptr;
}

} // namespace matrix_sdk
