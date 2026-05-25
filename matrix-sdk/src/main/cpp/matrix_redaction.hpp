#pragma once
#include <string>
#include <set>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

class RedactionManager {
public:
    RedactionManager(); ~RedactionManager();
    void redactEvent(const std::string& eventId, const std::string& reason = "");
    bool isRedacted(const std::string& eventId) const;
    std::string getRedactionReason(const std::string& eventId) const;
    void clear();
private:
    std::map<std::string, std::string> m_redactions;
};
} // namespace matrix_sdk
