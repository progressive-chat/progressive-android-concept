#pragma once

#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>
#include "protocol_type.hpp"
#include "protocol_message.hpp"
#include "protocol_room.hpp"

class IProtocolSession;

class ProtocolManager : public QObject {
    Q_OBJECT

public:
    static ProtocolManager& instance();

    void registerSession(ProtocolType type, IProtocolSession* session);
    void unregisterSession(ProtocolType type);
    IProtocolSession* getSession(ProtocolType type) const;
    QVector<IProtocolSession*> getAllSessions() const;

    QVector<ProtocolRoom> getAllRooms() const;
    QVector<ProtocolMessage> getAllMessages(const QString& roomId) const;

    void openAll();
    void closeAll();

    ConnectionState overallConnectionState() const;

signals:
    void overallStateChanged(ConnectionState state);
    void roomAdded(ProtocolRoom room);
    void messageReceived(ProtocolMessage message);
    void errorOccurred(ProtocolType type, QString error);

private slots:
    void onSessionStateChanged(ProtocolType type, ConnectionState state);

private:
    ProtocolManager() = default;
    ~ProtocolManager() override = default;
    ProtocolManager(const ProtocolManager&) = delete;
    ProtocolManager& operator=(const ProtocolManager&) = delete;

    void emitOverallState();

    QMap<ProtocolType, IProtocolSession*> sessions_;
};
