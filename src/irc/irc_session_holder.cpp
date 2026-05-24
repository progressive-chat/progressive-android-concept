#include "irc_session_holder.hpp"
#include "irc_session.hpp"

IrcSessionHolder* IrcSessionHolder::instance()
{
    static IrcSessionHolder* inst = new IrcSessionHolder();
    return inst;
}

IrcSessionHolder::IrcSessionHolder(QObject* parent)
    : QObject(parent)
{
}

IrcSessionHolder::~IrcSessionHolder()
{
    disconnectAll();
    qDeleteAll(sessions_);
}

IrcSession* IrcSessionHolder::getSession(const QString& serverId) const
{
    return sessions_.value(serverId, nullptr);
}

IrcSession* IrcSessionHolder::createSession(const IrcServerInfo& info)
{
    const QString id = info.serverId();
    if (sessions_.contains(id)) {
        return sessions_.value(id);
    }

    auto* session = new IrcSession(info, this);
    sessions_.insert(id, session);
    emit sessionAdded(session);
    return session;
}

void IrcSessionHolder::removeSession(const QString& serverId)
{
    auto* session = sessions_.take(serverId);
    if (session) {
        session->close();
        emit sessionRemoved(session);
        session->deleteLater();
    }
}

QVector<IrcSession*> IrcSessionHolder::getAllSessions() const
{
    QVector<IrcSession*> result;
    result.reserve(sessions_.size());
    for (auto* session : sessions_) {
        result.append(session);
    }
    return result;
}

void IrcSessionHolder::disconnectAll()
{
    for (auto* session : sessions_) {
        session->close();
    }
}

bool IrcSessionHolder::hasActiveSessions() const
{
    for (auto* session : sessions_) {
        if (session->connectionState() == ConnectionState::CONNECTED
            || session->connectionState() == ConnectionState::REGISTERED) {
            return true;
        }
    }
    return false;
}
