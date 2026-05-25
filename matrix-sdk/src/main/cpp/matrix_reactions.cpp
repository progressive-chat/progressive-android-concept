#include "matrix_reactions.hpp"
#include <algorithm>

namespace matrix_sdk {

ReactionAggregator::ReactionAggregator() = default;
ReactionAggregator::~ReactionAggregator() = default;

void ReactionAggregator::addReaction(const std::string& eventId, const ReactionEvent& reaction) {
    m_reactions[eventId].push_back(reaction);
}

void ReactionAggregator::removeReaction(const std::string& eventId, const std::string& sender, const std::string& key) {
    auto it = m_reactions.find(eventId);
    if (it != m_reactions.end()) {
        it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
            [&](auto& r) { return r.sender == sender && r.key == key; }), it->second.end());
    }
}

std::vector<Reaction> ReactionAggregator::getReactions(const std::string& eventId, const std::string& selfUserId) {
    std::vector<Reaction> result;
    std::map<std::string, Reaction> grouped;
    auto it = m_reactions.find(eventId);
    if (it != m_reactions.end()) {
        for (auto& r : it->second) {
            auto& gr = grouped[r.key];
            gr.key = r.key;
            gr.count++;
            if (r.sender == selfUserId) gr.reactedBySelf = true;
        }
    }
    for (auto& [key, r] : grouped) result.push_back(r);
    return result;
}

void ReactionAggregator::clear() { m_reactions.clear(); }

} // namespace matrix_sdk
