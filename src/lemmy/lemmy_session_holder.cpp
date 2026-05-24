#include "lemmy/lemmy_session_holder.hpp"
#include "lemmy/lemmy_session.hpp"

LemmySessionHolder* LemmySessionHolder::instance()
{
    static LemmySessionHolder holder;
    return &holder;
}

LemmySessionHolder::LemmySessionHolder(QObject* parent)
    : QObject(parent)
{
}

LemmySession* LemmySessionHolder::getSession(const QString& instanceUrl) const
{
    return sessions_.value(instanceUrl, nullptr);
}

LemmySession* LemmySessionHolder::createSession(const QString& instanceUrl, const QString& username, const QString& password)
{
    if (sessions_.contains(instanceUrl))
        return sessions_.value(instanceUrl);

    auto* session = new LemmySession(instanceUrl, username, password);
    sessions_.insert(instanceUrl, session);
    emit sessionAdded(instanceUrl);
    return session;
}

void LemmySessionHolder::removeSession(const QString& instanceUrl)
{
    auto* session = sessions_.take(instanceUrl);
    if (session) {
        session->close();
        emit sessionRemoved(instanceUrl);
        session->deleteLater();
    }
}

QVector<LemmySession*> LemmySessionHolder::getAllSessions() const
{
    QVector<LemmySession*> result;
    result.reserve(sessions_.size());
    for (auto* session : sessions_)
        result.append(session);
    return result;
}

void LemmySessionHolder::disconnectAll()
{
    for (auto* session : sessions_) {
        if (session)
            session->close();
    }
}

bool LemmySessionHolder::hasActiveSessions() const
{
    for (auto* session : sessions_) {
        if (session && session->connectionState() != ConnectionState::DISCONNECTED)
            return true;
    }
    return false;
}
