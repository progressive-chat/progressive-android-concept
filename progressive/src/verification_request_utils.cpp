#include "progressive/verification_request_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool verification_request_utils_validate(const std::string& i) { return !i.empty(); }
std::string verification_request_utils_process(const std::string& i) { return i; }
json verification_request_utils_toJson(const std::string& i) { return json::object(); }
}
