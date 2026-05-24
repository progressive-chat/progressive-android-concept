#ifndef IRC_SESSION_HOLDER_HPP
#define IRC_SESSION_HOLDER_HPP

#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>

#include "../irc/irc_session.hpp"

class IrcSession;

class IrcSessionHolder : public QObject {
    Q_OBJECT

public:
    static IrcSessionHolder* instance();

    IrcSession* getSession(const QString& serverId) const;
    IrcSession* createSession(const IrcServerInfo& info);
    void removeSession(const QString& serverId);
    QVector<IrcSession*> getAllSessions() const;
    void disconnectAll();
    bool hasActiveSessions() const;

signals:
    void sessionAdded(IrcSession* session);
    void sessionRemoved(IrcSession* session);

private:
    explicit IrcSessionHolder(QObject* parent = nullptr);
    ~IrcSessionHolder() override;
    IrcSessionHolder(const IrcSessionHolder&) = delete;
    IrcSessionHolder& operator=(const IrcSessionHolder&) = delete;

    QMap<QString, IrcSession*> sessions_;
};

#endif // IRC_SESSION_HOLDER_HPP
