#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct PaginationState {
    std::string from; std::string to; std::string nextBatch;
    bool hasMore = false; int totalItems = 0;
};

class PaginationManager {
public:
    PaginationManager(); ~PaginationManager();
    PaginationState getState(const std::string& roomId) const;
    void setState(const std::string& roomId, const PaginationState& state);
    bool hasMore(const std::string& roomId) const;
    std::string getNextBatch(const std::string& roomId) const;
    void reset(const std::string& roomId);
private:
    std::map<std::string, PaginationState> m_states;
};
} // namespace matrix_sdk
