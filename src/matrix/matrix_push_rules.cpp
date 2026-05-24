#include "matrix/matrix_push_rules.hpp"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QUrl>

// ─── Constructor ──────────────────────────────────────────────────────────────

MatrixPushRules::MatrixPushRules(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

// ─── Credentials ───────────────────────────────────────────────────────────────

void MatrixPushRules::setCredentials(const QString &homeserverUrl,
                                     const QString &userId,
                                     const QString &accessToken)
{
    m_homeserverUrl = homeserverUrl;
    m_userId = userId;
    m_accessToken = accessToken;
}

// ─── Refresh ───────────────────────────────────────────────────────────────────

void MatrixPushRules::refresh()
{
    if (m_homeserverUrl.isEmpty() || m_accessToken.isEmpty()) {
        return;
    }

    QUrl url(QStringLiteral("%1/_matrix/client/r0/pushrules/")
                 .arg(m_homeserverUrl));

    QNetworkRequest req(url);
    req.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(m_accessToken)
                                            .toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  QStringLiteral("application/json"));

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onRefreshReply(reply);
    });
}

// ─── Accessors ─────────────────────────────────────────────────────────────────

QVector<MatrixPushRules::PushRule> MatrixPushRules::allRules() const
{
    return m_rules;
}

// ─── Notification evaluation ───────────────────────────────────────────────────

bool MatrixPushRules::shouldNotify(const QJsonObject &event,
                                   const QString &roomId) const
{
    if (!event.contains(QStringLiteral("type"))) {
        return false;
    }

    const QString eventType = event.value(QStringLiteral("type")).toString();

    if (eventType == QStringLiteral("m.room.member")) {
        return false;
    }

    // Evaluate rules in priority order: override -> content -> room -> sender -> underride
    // For each category, first match wins (rules are server-ordered)
    static constexpr RuleType kOrder[] = {
        RuleType::Override, RuleType::Content, RuleType::Room,
        RuleType::Sender, RuleType::Underride
    };

    for (auto cat : kOrder) {
        for (const auto &rule : m_rules) {
            if (rule.type != cat) continue;
            if (!rule.enabled) continue;

            if (evaluateRule(rule, event, roomId)) {
                for (auto action : rule.actions) {
                    if (action == Action::DontNotify) {
                        return false;
                    }
                    if (action == Action::Notify) {
                        return true;
                    }
                }
                // Coalesce means defer to next rule category
                break;
            }
        }
    }

    return false;
}

// ─── Muting ────────────────────────────────────────────────────────────────────

void MatrixPushRules::setRoomMuted(const QString &roomId, bool muted)
{
    if (muted) {
        if (!m_mutedRooms.contains(roomId)) {
            m_mutedRooms.append(roomId);
        }

        bool found = false;
        for (auto &rule : m_rules) {
            if (rule.ruleId == QStringLiteral("__local_mute_room_%1").arg(roomId)) {
                found = true;
                rule.enabled = true;
                break;
            }
        }
        if (!found) {
            PushRule rule;
            rule.ruleId = QStringLiteral("__local_mute_room_%1").arg(roomId);
            rule.type = RuleType::Override;
            rule.kind = RuleKind::Global;
            rule.actions = { Action::DontNotify };
            rule.defaultRule = false;
            Condition cond;
            cond.kind = QStringLiteral("event_match");
            cond.key = QStringLiteral("room_id");
            cond.pattern = roomId;
            rule.conditions = { cond };
            m_rules.prepend(rule);
        }
    } else {
        m_mutedRooms.removeAll(roomId);
        m_rules.erase(
            std::remove_if(m_rules.begin(), m_rules.end(), [&](const PushRule &r) {
                return r.ruleId == QStringLiteral("__local_mute_room_%1").arg(roomId);
            }),
            m_rules.end());
    }
    emit rulesUpdated();
}

void MatrixPushRules::setSenderMuted(const QString &userId, bool muted)
{
    if (muted) {
        if (!m_mutedSenders.contains(userId)) {
            m_mutedSenders.append(userId);
        }

        bool found = false;
        for (auto &rule : m_rules) {
            if (rule.ruleId == QStringLiteral("__local_mute_sender_%1").arg(userId)) {
                found = true;
                rule.enabled = true;
                break;
            }
        }
        if (!found) {
            PushRule rule;
            rule.ruleId = QStringLiteral("__local_mute_sender_%1").arg(userId);
            rule.type = RuleType::Override;
            rule.kind = RuleKind::Global;
            rule.actions = { Action::DontNotify };
            rule.defaultRule = false;
            Condition cond;
            cond.kind = QStringLiteral("event_match");
            cond.key = QStringLiteral("sender");
            cond.pattern = userId;
            rule.conditions = { cond };
            m_rules.prepend(rule);
        }
    } else {
        m_mutedSenders.removeAll(userId);
        m_rules.erase(
            std::remove_if(m_rules.begin(), m_rules.end(), [&](const PushRule &r) {
                return r.ruleId == QStringLiteral("__local_mute_sender_%1").arg(userId);
            }),
            m_rules.end());
    }
    emit rulesUpdated();
}

void MatrixPushRules::setKeywordRule(const QString &keyword, bool enabled)
{
    if (enabled) {
        if (!m_keywords.contains(keyword)) {
            m_keywords.append(keyword);
        }

        bool found = false;
        for (auto &rule : m_rules) {
            if (rule.ruleId == QStringLiteral("__local_keyword_%1").arg(keyword)) {
                found = true;
                rule.enabled = true;
                break;
            }
        }
        if (!found) {
            PushRule rule;
            rule.ruleId = QStringLiteral("__local_keyword_%1").arg(keyword);
            rule.type = RuleType::Content;
            rule.kind = RuleKind::Global;
            rule.actions = { Action::Notify };
            rule.pattern = keyword;
            rule.defaultRule = false;
            m_rules.append(rule);
        }
    } else {
        m_keywords.removeAll(keyword);
        m_rules.erase(
            std::remove_if(m_rules.begin(), m_rules.end(), [&](const PushRule &r) {
                return r.ruleId == QStringLiteral("__local_keyword_%1").arg(keyword);
            }),
            m_rules.end());
    }
    emit rulesUpdated();
}

bool MatrixPushRules::isRoomMuted(const QString &roomId) const
{
    return m_mutedRooms.contains(roomId);
}

bool MatrixPushRules::isKeywordMatch(const QString &text) const
{
    for (const auto &keyword : m_keywords) {
        if (text.contains(keyword, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

// ─── Network reply ─────────────────────────────────────────────────────────────

void MatrixPushRules::onRefreshReply(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }

    QVector<PushRule> oldRules;
    oldRules.swap(m_rules);

    // Preserve local-only rules (mutes, keywords) that start with __local_
    for (const auto &r : oldRules) {
        if (r.ruleId.startsWith(QStringLiteral("__local_"))) {
            m_rules.append(r);
        }
    }

    parseResponse(doc.object());
    emit rulesUpdated();
}

// ─── Parsing ───────────────────────────────────────────────────────────────────

void MatrixPushRules::parseResponse(const QJsonObject &json)
{
    static const QVector<QPair<QString, RuleKind>> scopes = {
        { QStringLiteral("global"), RuleKind::Global },
        { QStringLiteral("device"), RuleKind::Device }
    };

    for (const auto &scope : scopes) {
        QJsonObject scopeObj = json.value(scope.first).toObject();

        parseRuleSet(scopeObj.value(QStringLiteral("override")).toArray(),
                     RuleType::Override, scope.second);
        parseRuleSet(scopeObj.value(QStringLiteral("content")).toArray(),
                     RuleType::Content, scope.second);
        parseRuleSet(scopeObj.value(QStringLiteral("room")).toArray(),
                     RuleType::Room, scope.second);
        parseRuleSet(scopeObj.value(QStringLiteral("sender")).toArray(),
                     RuleType::Sender, scope.second);
        parseRuleSet(scopeObj.value(QStringLiteral("underride")).toArray(),
                     RuleType::Underride, scope.second);
    }
}

void MatrixPushRules::parseRuleSet(const QJsonArray &rules, RuleType type,
                                   RuleKind kind)
{
    for (const auto &val : rules) {
        QJsonObject obj = val.toObject();
        PushRule rule;
        rule.ruleId = obj.value(QStringLiteral("rule_id")).toString();
        rule.type = type;
        rule.kind = kind;
        rule.enabled = obj.value(QStringLiteral("enabled")).toBool(true);
        rule.defaultRule = obj.value(QStringLiteral("default")).toBool(true);
        rule.pattern = obj.value(QStringLiteral("pattern")).toString();

        const QJsonArray actions = obj.value(QStringLiteral("actions")).toArray();
        for (const auto &a : actions) {
            if (a.isString()) {
                rule.actions.append(actionFromString(a.toString()));
            } else if (a.isObject()) {
                QJsonObject ao = a.toObject();
                if (ao.value(QStringLiteral("set_tweak")).toString() ==
                    QStringLiteral("highlight")) {
                    rule.actions.append(Action::Notify);
                } else {
                    rule.actions.append(Action::Notify);
                }
            }
        }

        const QJsonArray conds = obj.value(QStringLiteral("conditions")).toArray();
        for (const auto &c : conds) {
            QJsonObject co = c.toObject();
            Condition cond;
            cond.kind = co.value(QStringLiteral("kind")).toString();
            cond.key = co.value(QStringLiteral("key")).toString();
            cond.pattern = co.value(QStringLiteral("pattern")).toString();
            cond.is = co.value(QStringLiteral("is")).toString().isEmpty()
                          ? true
                          : co.value(QStringLiteral("is")).toString() !=
                                QStringLiteral("false");
            rule.conditions.append(cond);
        }

        m_rules.append(rule);
    }
}

// ─── Rule evaluation ───────────────────────────────────────────────────────────

bool MatrixPushRules::evaluateRule(const PushRule &rule, const QJsonObject &event,
                                   const QString &roomId) const
{
    if (rule.conditions.isEmpty()) {
        // Content rules without conditions match on pattern against body
        if (rule.type == RuleType::Content) {
            const QJsonObject content =
                event.value(QStringLiteral("content")).toObject();
            const QString body = content.value(QStringLiteral("body")).toString();
            return globMatch(rule.pattern, body);
        }
        return true;
    }

    for (const auto &cond : rule.conditions) {
        if (!evaluateCondition(cond, event, roomId)) {
            return false;
        }
    }

    // For content-type rules with a pattern, also check for pattern match on body
    // Only when conditions already matched
    if (rule.type == RuleType::Content && !rule.pattern.isEmpty()) {
        const QJsonObject content =
            event.value(QStringLiteral("content")).toObject();
        const QString body = content.value(QStringLiteral("body")).toString();
        if (!globMatch(rule.pattern, body)) {
            return false;
        }
    }

    return true;
}

bool MatrixPushRules::evaluateCondition(const Condition &cond,
                                        const QJsonObject &event,
                                        const QString &roomId) const
{
    if (cond.kind == QStringLiteral("event_match")) {
        QJsonValue val;

        if (cond.key == QStringLiteral("room_id")) {
            val = QJsonValue(roomId);
        } else if (cond.key == QStringLiteral("type")) {
            val = event.value(QStringLiteral("type"));
        } else if (cond.key == QStringLiteral("sender")) {
            val = event.value(QStringLiteral("sender"));
        } else if (cond.key == QStringLiteral("state_key")) {
            val = event.value(QStringLiteral("state_key"));
        } else {
            val = event.value(cond.key);
        }

        const QString text = val.toString();
        if (text.isEmpty() && !val.isString()) {
            return false;
        }

        return globMatch(cond.pattern, text);
    }

    if (cond.kind == QStringLiteral("contains_display_name")) {
        const QJsonObject content =
            event.value(QStringLiteral("content")).toObject();
        const QString body = content.value(QStringLiteral("body")).toString();
        return body.contains(m_userId, Qt::CaseInsensitive) ||
               body.contains(m_userId.section(QLatin1Char(':'), 0, 0),
                             Qt::CaseInsensitive);
    }

    if (cond.kind == QStringLiteral("room_member_count")) {
        const QString is = cond.is ? QStringLiteral("==") : QStringLiteral("<");
        if (is == QStringLiteral("==")) {
            return true;
        }
        return true;
    }

    if (cond.kind == QStringLiteral("sender_notification_permission")) {
        const QString sender =
            event.value(QStringLiteral("sender")).toString();
        return cond.key == QStringLiteral("room") && sender != m_userId;
    }

    return false;
}

// ─── Glob matching ─────────────────────────────────────────────────────────────

bool MatrixPushRules::globMatch(const QString &pattern, const QString &text) const
{
    if (pattern.isEmpty()) {
        return true;
    }

    QString regexPattern;
    regexPattern.reserve(pattern.size() * 2 + 2);
    regexPattern += QLatin1Char('^');

    for (int i = 0; i < pattern.size(); ++i) {
        const QChar c = pattern.at(i);
        if (c == QLatin1Char('*')) {
            regexPattern += QStringLiteral(".*");
        } else if (c == QLatin1Char('?')) {
            regexPattern += QLatin1Char('.');
        } else {
            regexPattern += QRegularExpression::escape(QString(c));
        }
    }

    regexPattern += QLatin1Char('$');

    QRegularExpression re(regexPattern, QRegularExpression::CaseInsensitiveOption);
    return re.match(text).hasMatch();
}

// ─── Conversions ───────────────────────────────────────────────────────────────

MatrixPushRules::Action MatrixPushRules::actionFromString(const QString &s)
{
    if (s == QStringLiteral("notify")) return Action::Notify;
    if (s == QStringLiteral("dont_notify")) return Action::DontNotify;
    if (s == QStringLiteral("coalesce")) return Action::Coalesce;
    return Action::Notify;
}

QString MatrixPushRules::actionToString(Action a)
{
    switch (a) {
    case Action::Notify:     return QStringLiteral("notify");
    case Action::DontNotify: return QStringLiteral("dont_notify");
    case Action::Coalesce:   return QStringLiteral("coalesce");
    }
    return QStringLiteral("notify");
}

MatrixPushRules::RuleType MatrixPushRules::ruleTypeFromString(const QString &s)
{
    if (s == QStringLiteral("override"))  return RuleType::Override;
    if (s == QStringLiteral("content"))   return RuleType::Content;
    if (s == QStringLiteral("room"))      return RuleType::Room;
    if (s == QStringLiteral("sender"))    return RuleType::Sender;
    if (s == QStringLiteral("underride")) return RuleType::Underride;
    return RuleType::Override;
}

QString MatrixPushRules::ruleTypeToString(RuleType t)
{
    switch (t) {
    case RuleType::Override:  return QStringLiteral("override");
    case RuleType::Content:   return QStringLiteral("content");
    case RuleType::Room:      return QStringLiteral("room");
    case RuleType::Sender:    return QStringLiteral("sender");
    case RuleType::Underride: return QStringLiteral("underride");
    }
    return QStringLiteral("override");
}

MatrixPushRules::RuleKind MatrixPushRules::ruleKindFromString(const QString &s)
{
    if (s == QStringLiteral("device")) return RuleKind::Device;
    return RuleKind::Global;
}
