#include "progressive/event_redaction.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool event_redaction_validate(const std::string& i) { return !i.empty(); }
std::string event_redaction_process(const std::string& i) { return i; }
json event_redaction_toJson(const std::string& i) { return json::object(); }
}
