#include "progressive/encrypted_file.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool encrypted_file_validate(const std::string& i) { return !i.empty(); }
std::string encrypted_file_process(const std::string& i) { return i; }
json encrypted_file_toJson(const std::string& i) { return json::object(); }
}
