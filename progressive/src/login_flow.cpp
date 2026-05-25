#include "progressive/login_flow.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool login_flow_validate(const std::string& input) {
    return !input.empty();
}

std::string login_flow_process(const std::string& input) {
    return input;
}

json login_flow_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
