#pragma once
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct Reaction { std::string key; int count = 0; bool reactedBySelf = false; };
struct ReactionEvent { std::string eventId; std::string sender; std::string key; uint64_t timestamp = 0; };

class ReactionAggregator {
public:
    ReactionAggregator(); ~ReactionAggregator();
    void addReaction(const std::string& eventId, const ReactionEvent& reaction);
    void removeReaction(const std::string& eventId, const std::string& sender, const std::string& key);
    std::vector<Reaction> getReactions(const std::string& eventId, const std::string& selfUserId = "");
    void clear();
private:
    std::map<std::string, std::vector<ReactionEvent>> m_reactions;
};
} // namespace matrix_sdk
