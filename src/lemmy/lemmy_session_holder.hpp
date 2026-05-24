#pragma once

#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>

class LemmySession;

class LemmySessionHolder : public QObject
{
    Q_OBJECT

public:
    static LemmySessionHolder* instance();

    LemmySession* getSession(const QString& instanceUrl) const;
    LemmySession* createSession(const QString& instanceUrl, const QString& username, const QString& password);
    void removeSession(const QString& instanceUrl);

    QVector<LemmySession*> getAllSessions() const;
    void disconnectAll();
    bool hasActiveSessions() const;

signals:
    void sessionAdded(QString url);
    void sessionRemoved(QString url);

private:
    explicit LemmySessionHolder(QObject* parent = nullptr);
    ~LemmySessionHolder() override = default;

    QMap<QString, LemmySession*> sessions_;

    Q_DISABLE_COPY(LemmySessionHolder)
};
