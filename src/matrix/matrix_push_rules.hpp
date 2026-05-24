#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>

class QNetworkReply;

class MatrixPushRules : public QObject {
    Q_OBJECT

public:
    enum class RuleType { Override, Content, Room, Sender, Underride };
    enum class RuleKind { Global, Device };
    enum class Action { Notify, DontNotify, Coalesce };

    struct Condition {
        QString kind;
        QString key;
        QString pattern;
        bool is = true;
        bool operator==(const Condition &o) const {
            return kind == o.kind && key == o.key && pattern == o.pattern && is == o.is;
        }
    };

    struct PushRule {
        QString ruleId;
        RuleType type = RuleType::Override;
        RuleKind kind = RuleKind::Global;
        QVector<Action> actions;
        QString pattern;
        QVector<Condition> conditions;
        bool enabled = true;
        bool defaultRule = true;
    };

    explicit MatrixPushRules(QObject *parent = nullptr);

    void setCredentials(const QString &homeserverUrl,
                        const QString &userId,
                        const QString &accessToken);

    void refresh();
    QVector<PushRule> allRules() const;

    bool shouldNotify(const QJsonObject &event, const QString &roomId) const;

    void setRoomMuted(const QString &roomId, bool muted);
    void setSenderMuted(const QString &userId, bool muted);
    void setKeywordRule(const QString &keyword, bool enabled);

    bool isRoomMuted(const QString &roomId) const;
    bool isKeywordMatch(const QString &text) const;

signals:
    void rulesUpdated();

private slots:
    void onRefreshReply(QNetworkReply *reply);

private:
    void parseResponse(const QJsonObject &json);
    void parseRuleSet(const QJsonArray &rules, RuleType type, RuleKind kind);
    bool evaluateRule(const PushRule &rule, const QJsonObject &event,
                      const QString &roomId) const;
    bool evaluateCondition(const Condition &cond, const QJsonObject &event,
                           const QString &roomId) const;
    bool globMatch(const QString &pattern, const QString &text) const;

    static Action actionFromString(const QString &s);
    static QString actionToString(Action a);
    static RuleType ruleTypeFromString(const QString &s);
    static QString ruleTypeToString(RuleType t);
    static RuleKind ruleKindFromString(const QString &s);

    QNetworkAccessManager *m_nam = nullptr;
    QString m_homeserverUrl;
    QString m_userId;
    QString m_accessToken;

    QVector<PushRule> m_rules;

    QStringList m_mutedRooms;
    QStringList m_mutedSenders;
    QStringList m_keywords;
};
