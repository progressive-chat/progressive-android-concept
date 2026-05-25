#include "progressive/poll_vote_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool poll_vote_utils_validate(const std::string& input) { return !input.empty(); }
std::string poll_vote_utils_process(const std::string& input) { return input; }
json poll_vote_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
