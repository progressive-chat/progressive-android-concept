#pragma once
#include <string>
#include <map>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

class EventDecryptor {
public:
    EventDecryptor() = default;
    ~EventDecryptor() = default;
    json decryptEvent(const std::string& roomId, const json& event);
    bool canDecrypt(const std::string& roomId, const json& event) const;
    void clearSessionCache(const std::string& roomId);
    void clearAllSessions();
    size_t getSessionCount() const { return m_sessionCache.size(); }
private:
    std::map<std::string, std::string> m_sessionCache;
};
} // namespace matrix_sdk
