#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

enum class EventProcessResult { OK, DUPLICATE, REDACTED, ENCRYPTED, DECRYPTED, IGNORED, ERROR };

struct ProcessedEvent {
    std::string eventId;
    std::string roomId;
    std::string sender;
    std::string type;
    json content;
    uint64_t timestamp = 0;
    uint64_t age = 0;
    EventProcessResult result = EventProcessResult::OK;
    bool isState = false;
    std::string stateKey;
    std::string redacts;
    std::string replacesEvent;
    std::string threadRoot;
};

class EventProcessor {
public:
    EventProcessor(); ~EventProcessor();
    ProcessedEvent process(const std::string& roomId, const json& rawEvent);
    std::vector<ProcessedEvent> processBatch(const std::string& roomId, const json& events);
    bool isDuplicate(const std::string& eventId) const;
    void clearProcessedCache(size_t maxSize = 10000);
    void onEventProcessed(std::function<void(const ProcessedEvent&)> cb);
    void setDecryptor(std::shared_ptr<class EventDecryptor> decryptor);
private:
    std::set<std::string> m_processedIds;
    std::shared_ptr<EventDecryptor> m_decryptor;
    std::function<void(const ProcessedEvent&)> m_callback;
};
} // namespace matrix_sdk
