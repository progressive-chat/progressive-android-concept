#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QStringList>
#include "../protocol/protocol_session.hpp"
#include "../protocol/protocol_message.hpp"
#include "../protocol/protocol_room.hpp"
#include "../protocol/protocol_type.hpp"

#include <QtNetwork/QSslError>

class QTcpSocket;
class QSslSocket;
class QTimer;

struct IrcServerInfo {
    QString host;
    quint16 port = 6667;
    bool useTls = false;
    QString nick;
    QString user;
    QString realName;
    QString password;

    QString serverId() const {
        return host + QStringLiteral(":") + QString::number(port);
    }
};

class IrcSession : public IProtocolSession {
    Q_OBJECT

public:
    explicit IrcSession(const IrcServerInfo &info, QObject *parent = nullptr);
    ~IrcSession() override;

    void open() override;
    void close() override;
    void sendMessage(const QString &roomId, const QString &text, ProtocolContentType contentType) override;
    QVector<ProtocolRoom> getRooms() override;
    QVector<ProtocolMessage> getMessages(const QString &roomId, int limit = 50) override;
    void markAsRead(const QString &roomId) override;
    void joinRoom(const QString &roomIdOrAlias) override;
    void leaveRoom(const QString &roomId) override;
    QVector<ProtocolRoom> searchRooms(const QString &query) override;

    ProtocolType protocolType() const override;
    ConnectionState connectionState() const override;
    QString userId() const override;
    QString displayName() const override;

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError();
    void onSslErrors(const QList<QSslError> &errors);

private:
    void sendRaw(const QString &line);
    void parseLine(const QString &line);
    void setConnectionState(ConnectionState state);

    void handlePrivmsg(const QString &prefix, const QStringList &params);
    void handleJoin(const QString &prefix, const QStringList &params);
    void handlePart(const QString &prefix, const QStringList &params);
    void handleQuit(const QString &prefix, const QStringList &params);
    void handleNotice(const QString &prefix, const QStringList &params);
    void handleMode(const QString &prefix, const QStringList &params);
    void handleKick(const QString &prefix, const QStringList &params);
    void handleTopic(const QString &prefix, const QStringList &params);
    void handleNick(const QString &prefix, const QStringList &params);
    void handleNumeric(int numeric, const QString &prefix, const QStringList &params);
    void handlePing(const QStringList &params);

    QString extractNick(const QString &prefix) const;
    QString extractUser(const QString &prefix) const;
    QString extractHost(const QString &prefix) const;

    ProtocolRoom *findRoom(const QString &channel);
    void addMessageToRoom(const QString &roomId, const ProtocolMessage &msg);

    IrcServerInfo serverInfo_;
    QTcpSocket *socket_ = nullptr;
    QTimer *pingTimer_ = nullptr;

    QString readBuffer_;
    QString currentNick_;

    QMap<QString, ProtocolRoom> rooms_;
    QMap<QString, QVector<ProtocolMessage>> messages_;

    ConnectionState state_ = ConnectionState::DISCONNECTED;
    bool registered_ = false;
};
