#pragma once

#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>

#include "protocol/protocol_session.hpp"
#include "protocol/protocol_room.hpp"
#include "protocol/protocol_message.hpp"

class LemmyClient;

class LemmySession : public IProtocolSession
{
    Q_OBJECT

public:
    explicit LemmySession(const QString &instanceUrl,
                          const QString &username,
                          const QString &password,
                          QObject *parent = nullptr);
    ~LemmySession() override;

    void open() override;
    void close() override;
    void sendMessage(const QString &roomId, const QString &text, ProtocolContentType contentType) override;
    QVector<ProtocolRoom> getRooms() override;
    QVector<ProtocolMessage> getMessages(const QString &roomId, int limit = 50) override;
    void markAsRead(const QString &roomId) override;
    void joinRoom(const QString &roomIdOrAlias) override;
    void leaveRoom(const QString &roomId) override;
    QVector<ProtocolRoom> searchRooms(const QString &query) override;

    ProtocolType protocolType() const override { return ProtocolType::LEMMY; }
    ConnectionState connectionState() const override { return m_state; }
    QString userId() const override { return m_username; }
    QString displayName() const override { return m_username; }

private:
    void refreshCommunities();
    void refreshPosts(qint64 communityId);
    void setState(ConnectionState state);

    LemmyClient *m_client;
    QString m_instanceUrl;
    QString m_username;
    QString m_password;
    ConnectionState m_state = ConnectionState::DISCONNECTED;

    QMap<qint64, ProtocolRoom> m_rooms;
    QMap<qint64, QVector<ProtocolMessage>> m_messages;
};
