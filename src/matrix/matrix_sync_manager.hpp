#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

class MatrixSession;
class QNetworkAccessManager;
class QNetworkReply;

class MatrixSyncManager : public QObject
{
    Q_OBJECT

public:
    explicit MatrixSyncManager(MatrixSession& session, QObject* parent = nullptr);
    ~MatrixSyncManager() override;

    void startSync();
    void stopSync();

    void setHomeserverUrl(const QString& url);
    void setAccessToken(const QString& token);

    QString nextBatch() const;

signals:
    void messageReceived(const QString& roomId, const QJsonObject& event);
    void roomJoined(const QString& roomId, const QJsonObject& roomData);
    void roomInvited(const QString& roomId, const QJsonObject& inviteData);
    void presenceChanged(const QString& userId, const QJsonObject& presenceData);
    void deviceMessageReceived(const QString& type, const QJsonObject& message);

private slots:
    void onSyncTimerFired();
    void onSyncReplyFinished();

private:
    void doSync();
    void processSyncResponse(const QJsonObject& response);
    void processJoinedRooms(const QJsonObject& rooms);
    void processInvitedRooms(const QJsonObject& rooms);
    void processLeftRooms(const QJsonObject& rooms);
    void processAccountData(const QJsonArray& events);
    void processPresence(const QJsonArray& events);
    void processToDevice(const QJsonObject& toDevice);
    void scheduleRetry();
    void resetBackoff();
    QJsonObject buildSyncFilter() const;

    MatrixSession& m_session;
    QNetworkAccessManager* m_nam;
    QTimer* m_syncTimer;
    QString m_homeserverUrl;
    QString m_accessToken;
    QString m_nextBatch;
    int m_retryDelayMs;
    int m_currentBackoffMs;
    bool m_running;
    QNetworkReply* m_currentReply;
};
