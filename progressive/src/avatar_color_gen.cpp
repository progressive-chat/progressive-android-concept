#include "progressive/avatar_color_gen.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool avatar_color_gen_validate(const std::string& i) { return !i.empty(); }
std::string avatar_color_gen_process(const std::string& i) { return i; }
json avatar_color_gen_toJson(const std::string& i) { return json::object(); }
}
