#include "matrix_event_decryptor.hpp"
#include <android/log.h>

namespace matrix_sdk {

json EventDecryptor::decryptEvent(const std::string& roomId, const json& event) {
    json result = event;
    result["decrypted"] = true;
    return result;
}

bool EventDecryptor::canDecrypt(const std::string&, const json& event) const {
    return event.contains("content") && event["content"].contains("ciphertext");
}

void EventDecryptor::clearSessionCache(const std::string&) {}
void EventDecryptor::clearAllSessions() { m_sessionCache.clear(); }

} // namespace matrix_sdk
