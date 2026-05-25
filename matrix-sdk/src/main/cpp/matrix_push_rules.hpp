#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann::json;

enum class PushRuleKind { OVERRIDE, UNDER, SENDER, ROOM, CONTENT };

struct PushCondition {
    std::string kind;
    std::string key;
    std::string pattern;
    bool is = false;
};

struct PushRule {
    std::string ruleId;
    PushRuleKind kind = PushRuleKind::OVERRIDE;
    bool enabled = true;
    bool isDefault = false;
    int priority = 0;
    std::vector<PushCondition> conditions;
    std::vector<std::string> actions;
};

class PushRuleEvaluator {
public:
    PushRuleEvaluator();

    void loadRules(const json& pushRulesJson);
    std::vector<PushRule> getRules(PushRuleKind kind) const;
    std::vector<std::string> evaluate(const std::string& roomId,
                                       const std::string& sender,
                                       const std::string& eventType,
                                       const json& content) const;

    bool shouldNotify(const std::string& roomId, const std::string& sender,
                      const std::string& eventType, const json& content) const;

private:
    std::vector<PushRule> m_overrideRules;
    std::vector<PushRule> m_contentRules;
    std::vector<PushRule> m_roomRules;
    std::vector<PushRule> m_senderRules;
    std::vector<PushRule> m_underrideRules;

    bool matchRule(const PushRule& rule, const std::string& roomId,
                   const std::string& sender, const std::string& eventType,
                   const json& content) const;
    bool matchCondition(const PushCondition& cond, const std::string& roomId,
                        const std::string& sender, const std::string& eventType,
                        const json& content) const;
    std::string globToRegex(const std::string& glob) const;
};

} // namespace matrix_sdk
