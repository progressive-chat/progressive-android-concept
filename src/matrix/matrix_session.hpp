#pragma once

#include <QHash>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QVector>

#include "protocol/protocol_session.hpp"

class QNetworkReply;

class MatrixSession : public IProtocolSession {
    Q_OBJECT

public:
    explicit MatrixSession(const QString &userId,
                           const QString &accessToken,
                           const QString &homeserverUrl,
                           QObject *parent = nullptr);
    ~MatrixSession() override;

    void open() override;
    void close() override;
    void sendMessage(const QString &roomId, const QString &text, ProtocolContentType contentType) override;
    QVector<ProtocolRoom> getRooms() override;
    QVector<ProtocolMessage> getMessages(const QString &roomId, int limit = 50) override;
    void markAsRead(const QString &roomId) override;
    void joinRoom(const QString &roomIdOrAlias) override;
    void leaveRoom(const QString &roomId) override;
    QVector<ProtocolRoom> searchRooms(const QString &query) override;

    ProtocolType protocolType() const override { return ProtocolType::MATRIX; }
    ConnectionState connectionState() const override { return m_state; }
    QString userId() const override { return m_userId; }
    QString displayName() const override { return m_displayName; }

    QString accessToken() const { return m_accessToken; }
    QString homeserverUrl() const { return m_homeserverUrl; }

private:
    void startSyncLoop();
    void doSync();
    void processSyncResponse(const QJsonObject &json);
    void processJoinedRooms(const QJsonObject &roomsObject);
    void processRoomEvents(const QString &roomId, const QJsonArray &events);
    void processTimelineEvents(const QString &roomId, const QJsonArray &events);
    ProtocolMessage parseMessageEvent(const QString &roomId, const QJsonObject &event);
    ProtocolRoom parseRoomInfo(const QString &roomId, const QJsonObject &roomObj);
    QString generateTxnId();
    void setState(ConnectionState state);
    QNetworkRequest makeRequest(const QString &path) const;

    QString m_userId;
    QString m_password;
    QString m_accessToken;
    QString m_homeserverUrl;
    QString m_displayName;
    ConnectionState m_state = ConnectionState::DISCONNECTED;

    QNetworkAccessManager *m_nam;
    QTimer *m_syncTimer;
    bool m_syncActive = false;
    QString m_nextBatch;
    int m_syncTimeoutMs = 30000;
    int m_txnCounter = 0;

    QHash<QString, ProtocolRoom> m_rooms;
    QHash<QString, QVector<ProtocolMessage>> m_messages;
};
