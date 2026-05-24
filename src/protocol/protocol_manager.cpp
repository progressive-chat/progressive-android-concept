#include "protocol_manager.hpp"
#include "protocol_session.hpp"

ProtocolManager& ProtocolManager::instance()
{
    static ProtocolManager mgr;
    return mgr;
}

void ProtocolManager::registerSession(ProtocolType type, IProtocolSession* session)
{
    if (!session || sessions_.contains(type))
        return;

    sessions_.insert(type, session);

    connect(session, &IProtocolSession::connectionStateChanged,
            this, &ProtocolManager::onSessionStateChanged);
    connect(session, &IProtocolSession::messageReceived,
            this, &ProtocolManager::messageReceived);
    connect(session, &IProtocolSession::roomUpdated,
            this, &ProtocolManager::roomAdded);
    connect(session, &IProtocolSession::errorOccurred,
            this, [this, type](const QString& error) {
                emit errorOccurred(type, error);
            });
}

void ProtocolManager::unregisterSession(ProtocolType type)
{
    if (!sessions_.contains(type))
        return;

    IProtocolSession* session = sessions_.take(type);
    disconnect(session, nullptr, this, nullptr);
    emitOverallState();
}

IProtocolSession* ProtocolManager::getSession(ProtocolType type) const
{
    return sessions_.value(type, nullptr);
}

QVector<IProtocolSession*> ProtocolManager::getAllSessions() const
{
    QVector<IProtocolSession*> result;
    result.reserve(sessions_.size());
    for (auto* session : sessions_)
        result.append(session);
    return result;
}

QVector<ProtocolRoom> ProtocolManager::getAllRooms() const
{
    QVector<ProtocolRoom> result;
    for (auto* session : sessions_) {
        if (session)
            result.append(session->getRooms());
    }
    return result;
}

QVector<ProtocolMessage> ProtocolManager::getAllMessages(const QString& roomId) const
{
    for (auto* session : sessions_) {
        if (!session)
            continue;
        const auto rooms = session->getRooms();
        for (const auto& room : rooms) {
            if (room.id == roomId)
                return session->getMessages(roomId);
        }
    }
    return {};
}

void ProtocolManager::openAll()
{
    for (auto* session : sessions_) {
        if (session)
            session->open();
    }
}

void ProtocolManager::closeAll()
{
    for (auto* session : sessions_) {
        if (session)
            session->close();
    }
}

ConnectionState ProtocolManager::overallConnectionState() const
{
    if (sessions_.isEmpty())
        return ConnectionState::DISCONNECTED;

    bool anyDisconnected = false;
    bool allConnected = true;

    for (auto* session : sessions_) {
        if (!session) {
            anyDisconnected = true;
            allConnected = false;
            continue;
        }
        const auto state = session->connectionState();
        if (state == ConnectionState::DISCONNECTED || state == ConnectionState::ERROR) {
            anyDisconnected = true;
            allConnected = false;
        } else if (state != ConnectionState::CONNECTED && state != ConnectionState::REGISTERED) {
            allConnected = false;
        }
    }

    if (anyDisconnected)
        return ConnectionState::DISCONNECTED;
    if (allConnected)
        return ConnectionState::CONNECTED;
    return ConnectionState::CONNECTING;
}

void ProtocolManager::onSessionStateChanged(ProtocolType type, ConnectionState state)
{
    Q_UNUSED(type)
    Q_UNUSED(state)
    emitOverallState();
}

void ProtocolManager::emitOverallState()
{
    emit overallStateChanged(overallConnectionState());
}
