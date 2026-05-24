#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include "protocol_type.hpp"
#include "protocol_message.hpp"
#include "protocol_room.hpp"

class IProtocolSession : public QObject {
    Q_OBJECT

public:
    virtual ~IProtocolSession() = default;

    virtual void open() = 0;
    virtual void close() = 0;
    virtual void sendMessage(const QString &roomId, const QString &text, ProtocolContentType contentType) = 0;
    virtual QVector<ProtocolRoom> getRooms() = 0;
    virtual QVector<ProtocolMessage> getMessages(const QString &roomId, int limit = 50) = 0;
    virtual void markAsRead(const QString &roomId) = 0;
    virtual void joinRoom(const QString &roomIdOrAlias) = 0;
    virtual void leaveRoom(const QString &roomId) = 0;
    virtual QVector<ProtocolRoom> searchRooms(const QString &query) = 0;

    virtual ProtocolType protocolType() const = 0;
    virtual ConnectionState connectionState() const = 0;
    virtual QString userId() const = 0;
    virtual QString displayName() const = 0;

signals:
    void connectionStateChanged(ProtocolType type, ConnectionState state);
    void messageReceived(ProtocolMessage message);
    void roomUpdated(ProtocolRoom room);
    void errorOccurred(QString error);
};
