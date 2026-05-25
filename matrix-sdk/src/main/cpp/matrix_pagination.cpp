#include "matrix_pagination.hpp"

namespace matrix_sdk {

PaginationManager::PaginationManager() = default;
PaginationManager::~PaginationManager() = default;

PaginationState PaginationManager::getState(const std::string& roomId) const {
    auto it = m_states.find(roomId);
    return it != m_states.end() ? it->second : PaginationState{};
}

void PaginationManager::setState(const std::string& roomId, const PaginationState& state) {
    m_states[roomId] = state;
}

bool PaginationManager::hasMore(const std::string& roomId) const {
    return getState(roomId).hasMore;
}

std::string PaginationManager::getNextBatch(const std::string& roomId) const {
    return getState(roomId).nextBatch;
}

void PaginationManager::reset(const std::string& roomId) {
    m_states[roomId] = {};
}

} // namespace matrix_sdk
