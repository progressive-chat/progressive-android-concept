#pragma once

#include <QDialog>
#include <QElapsedTimer>
#include <QMap>
#include <QString>

class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;
class QAbstractSocket;
class IrcSession;

class IrcConnectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit IrcConnectionDialog(IrcSession *session, QWidget *parent = nullptr);
    ~IrcConnectionDialog() override;

private slots:
    void onUpdateDuration();
    void onUpdateStats();
    void onChangeNick();
    void onSendRaw();
    void onDisconnect();
    void onReconnect();
    void onCopyServerInfo();
    void onConnectionStateChanged();
    void onSocketBytesWritten(qint64 bytes);
    void onLatencyReply();

private:
    void setupUi();
    void refreshInfo();
    QString formatDuration() const;
    QString formatBytes(qint64 bytes) const;

    IrcSession *m_session = nullptr;
    QAbstractSocket *m_socket = nullptr;

    // --- Server info labels ---
    QLabel *m_serverLabel = nullptr;
    QLabel *m_durationLabel = nullptr;
    QLabel *m_sslLabel = nullptr;
    QLabel *m_nickLabel = nullptr;
    QLabel *m_usernameLabel = nullptr;
    QLabel *m_realnameLabel = nullptr;
    QLabel *m_channelsCountLabel = nullptr;

    // --- Network stats labels ---
    QLabel *m_bytesSentLabel = nullptr;
    QLabel *m_bytesReceivedLabel = nullptr;
    QLabel *m_latencyLabel = nullptr;

    // --- Server feature labels ---
    QLabel *m_chantypesLabel = nullptr;
    QLabel *m_prefixLabel = nullptr;
    QLabel *m_chanmodesLabel = nullptr;
    QLabel *m_networkLabel = nullptr;
    QLabel *m_maxchannelsLabel = nullptr;
    QLabel *m_nicklenLabel = nullptr;

    // --- Controls ---
    QLineEdit *m_nickEdit = nullptr;
    QPushButton *m_changeNickBtn = nullptr;
    QLineEdit *m_rawCommandEdit = nullptr;
    QPushButton *m_sendRawBtn = nullptr;
    QPushButton *m_disconnectBtn = nullptr;
    QPushButton *m_reconnectBtn = nullptr;
    QPushButton *m_copyInfoBtn = nullptr;

    // --- Timers ---
    QTimer *m_durationTimer = nullptr;
    QTimer *m_statsTimer = nullptr;
    QTimer *m_latencyTimer = nullptr;
    QElapsedTimer m_connectedSince;
    QElapsedTimer m_latencyTimerElapsed;

    // --- Stats accumulators ---
    quint64 m_bytesSent = 0;
    quint64 m_bytesReceived = 0;

    // --- Server ISUPPORT features ---
    QMap<QString, QString> m_features;
};
