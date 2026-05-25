#include "matrix_event_processor.hpp"
#include "matrix_event_decryptor.hpp"
#include <android/log.h>
#include <set>

#define LOG_TAG "EventProcessor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

EventProcessor::EventProcessor() = default;
EventProcessor::~EventProcessor() = default;

ProcessedEvent EventProcessor::process(const std::string& roomId, const json& rawEvent) {
    ProcessedEvent pe;
    pe.roomId = roomId;
    pe.eventId = rawEvent.value("event_id", "");
    pe.sender = rawEvent.value("sender", "");
    pe.type = rawEvent.value("type", "");
    pe.content = rawEvent.value("content", json::object());
    pe.timestamp = rawEvent.value("origin_server_ts", 0ULL);
    pe.age = rawEvent.value("unsigned", json::object()).value("age", 0ULL);
    pe.isState = rawEvent.contains("state_key");
    pe.stateKey = rawEvent.value("state_key", "");
    pe.redacts = rawEvent.value("redacts", "");

    if (pe.eventId.empty()) { pe.result = EventProcessResult::ERROR; return pe; }
    if (isDuplicate(pe.eventId)) { pe.result = EventProcessResult::DUPLICATE; return pe; }

    m_processedIds.insert(pe.eventId);
    if (m_processedIds.size() > 10000) {
        auto it = m_processedIds.begin();
        m_processedIds.erase(it);
    }

    if (pe.type == "m.room.redaction") { pe.result = EventProcessResult::REDACTED; }
    else if (pe.type == "m.room.encrypted") {
        if (m_decryptor && m_decryptor->canDecrypt(roomId, rawEvent)) {
            json decrypted = m_decryptor->decryptEvent(roomId, rawEvent);
            if (decrypted.contains("decrypted")) pe.result = EventProcessResult::DECRYPTED;
        } else { pe.result = EventProcessResult::ENCRYPTED; }
    } else { pe.result = EventProcessResult::OK; }

    if (m_callback) m_callback(pe);
    return pe;
}

std::vector<ProcessedEvent> EventProcessor::processBatch(const std::string& roomId,
                                                           const json& events) {
    std::vector<ProcessedEvent> results;
    if (!events.is_array()) return results;
    for (auto& event : events) {
        results.push_back(process(roomId, event));
    }
    return results;
}

bool EventProcessor::isDuplicate(const std::string& eventId) const {
    return m_processedIds.find(eventId) != m_processedIds.end();
}

void EventProcessor::clearProcessedCache(size_t maxSize) {
    if (m_processedIds.size() > maxSize) m_processedIds.clear();
}

void EventProcessor::onEventProcessed(std::function<void(const ProcessedEvent&)> cb) {
    m_callback = std::move(cb);
}

void EventProcessor::setDecryptor(std::shared_ptr<EventDecryptor> decryptor) {
    m_decryptor = std::move(decryptor);
}

} // namespace matrix_sdk
