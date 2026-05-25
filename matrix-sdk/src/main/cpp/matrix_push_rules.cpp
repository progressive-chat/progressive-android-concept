#include "matrix_push_rules.hpp"
#include <regex>
#include <algorithm>

namespace matrix_sdk {

PushRuleEvaluator::PushRuleEvaluator() = default;

void PushRuleEvaluator::loadRules(const json& pushRulesJson) {
    auto loadKind = [&](const std::string& key, PushRuleKind kind) {
        if (!pushRulesJson.contains(key)) return;
        for (auto& rule : pushRulesJson[key]) {
            PushRule pr;
            pr.ruleId = rule.value("rule_id", "");
            pr.kind = kind;
            pr.enabled = rule.value("enabled", true);
            pr.isDefault = rule.value("default", false);
            if (rule.contains("conditions")) {
                for (auto& cond : rule["conditions"]) {
                    PushCondition pc;
                    pc.kind = cond.value("kind", "");
                    pc.key = cond.value("key", "");
                    pc.pattern = cond.value("pattern", "");
                    pc.is = cond.value("is", "");
                    pr.conditions.push_back(pc);
                }
            }
            if (rule.contains("actions")) {
                for (auto& action : rule["actions"]) {
                    if (action.is_string()) pr.actions.push_back(action);
                    else if (action.is_object() && action.contains("set_tweak")) {
                        pr.actions.push_back(action["set_tweak"]);
                    }
                }
            }
            switch (kind) {
                case PushRuleKind::OVERRIDE: m_overrideRules.push_back(pr); break;
                case PushRuleKind::CONTENT: m_contentRules.push_back(pr); break;
                case PushRuleKind::ROOM: m_roomRules.push_back(pr); break;
                case PushRuleKind::SENDER: m_senderRules.push_back(pr); break;
                case PushRuleKind::UNDER: m_underrideRules.push_back(pr); break;
            }
        }
    };

    loadKind("override", PushRuleKind::OVERRIDE);
    loadKind("content", PushRuleKind::CONTENT);
    loadKind("room", PushRuleKind::ROOM);
    loadKind("sender", PushRuleKind::SENDER);
    loadKind("underride", PushRuleKind::UNDER);
}

std::vector<PushRule> PushRuleEvaluator::getRules(PushRuleKind kind) const {
    switch (kind) {
        case PushRuleKind::OVERRIDE: return m_overrideRules;
        case PushRuleKind::CONTENT: return m_contentRules;
        case PushRuleKind::ROOM: return m_roomRules;
        case PushRuleKind::SENDER: return m_senderRules;
        case PushRuleKind::UNDER: return m_underrideRules;
    }
    return {};
}

std::vector<std::string> PushRuleEvaluator::evaluate(const std::string& roomId,
                                                       const std::string& sender,
                                                       const std::string& eventType,
                                                       const json& content) const {
    std::vector<std::string> actions;
    auto checkRules = [&](const std::vector<PushRule>& rules) {
        for (auto& rule : rules) {
            if (!rule.enabled) continue;
            if (matchRule(rule, roomId, sender, eventType, content)) {
                if (std::find(rule.actions.begin(), rule.actions.end(), "dont_notify")
                    != rule.actions.end()) {
                    actions.push_back("dont_notify");
                    return true;
                }
                if (std::find(rule.actions.begin(), rule.actions.end(), "notify")
                    != rule.actions.end()) {
                    actions.push_back("notify");
                    return true;
                }
                if (std::find(rule.actions.begin(), rule.actions.end(), "coalesce")
                    != rule.actions.end()) {
                    actions.push_back("coalesce");
                }
            }
        }
        return false;
    };

    if (checkRules(m_overrideRules)) return actions;
    if (checkRules(m_contentRules)) return actions;
    if (checkRules(m_roomRules)) return actions;
    if (checkRules(m_senderRules)) return actions;
    checkRules(m_underrideRules);
    return actions;
}

bool PushRuleEvaluator::shouldNotify(const std::string& roomId, const std::string& sender,
                                      const std::string& eventType, const json& content) const {
    auto actions = evaluate(roomId, sender, eventType, content);
    return std::find(actions.begin(), actions.end(), "notify") != actions.end();
}

bool PushRuleEvaluator::matchRule(const PushRule& rule, const std::string& roomId,
                                    const std::string& sender, const std::string& eventType,
                                    const json& content) const {
    if (rule.conditions.empty()) return true;
    for (auto& cond : rule.conditions) {
        if (!matchCondition(cond, roomId, sender, eventType, content)) return false;
    }
    return true;
}

bool PushRuleEvaluator::matchCondition(const PushCondition& cond, const std::string& roomId,
                                         const std::string& sender, const std::string& eventType,
                                         const json& content) const {
    if (cond.kind == "event_match") {
        if (cond.key == "room_id") return roomId == cond.pattern;
        if (cond.key == "sender") return sender == cond.pattern;
        if (cond.key == "type") return eventType == cond.pattern;
        if (cond.key == "content.body" && content.contains("body")) {
            std::regex re(globToRegex(cond.pattern), std::regex::icase);
            return std::regex_search(content["body"].get<std::string>(), re);
        }
    } else if (cond.kind == "room_member_count") {
        // Would need room state; stub
    } else if (cond.kind == "contains_display_name") {
        // Would need user profile; stub
    }
    return false;
}

std::string PushRuleEvaluator::globToRegex(const std::string& glob) const {
    std::string result = "^";
    for (char c : glob) {
        if (c == '*') result += ".*";
        else if (c == '?') result += ".";
        else if (c == '.' || c == '+' || c == '(' || c == ')' || c == '[' || c == ']' ||
                 c == '{' || c == '}' || c == '\\' || c == '^' || c == '$' || c == '|') {
            result += '\\'; result += c;
        } else {
            result += c;
        }
    }
    result += "$";
    return result;
}

} // namespace matrix_sdk
