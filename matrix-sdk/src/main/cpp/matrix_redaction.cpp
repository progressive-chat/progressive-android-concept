#include "matrix_redaction.hpp"

namespace matrix_sdk {

RedactionManager::RedactionManager() = default;
RedactionManager::~RedactionManager() = default;

void RedactionManager::redactEvent(const std::string& eventId, const std::string& reason) {
    m_redactions[eventId] = reason;
}

bool RedactionManager::isRedacted(const std::string& eventId) const {
    return m_redactions.find(eventId) != m_redactions.end();
}

std::string RedactionManager::getRedactionReason(const std::string& eventId) const {
    auto it = m_redactions.find(eventId);
    return it != m_redactions.end() ? it->second : "";
}

void RedactionManager::clear() { m_redactions.clear(); }

} // namespace matrix_sdk
