#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QFrame>
#include <QSet>

#include "protocol/protocol_message.hpp"

class MatrixSession;
class MessageBubble;
class LoadingSpinner;

class ThreadViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ThreadViewWidget(const QString &rootMessageId,
                              const QString &roomId,
                              MatrixSession *session,
                              QWidget *parent = nullptr);

signals:
    void replySent(const QString &threadRootId);
    void backRequested();

private slots:
    void fetchRootMessage();
    void fetchThreadReplies();
    void onRootFetched();
    void onRepliesFetched();
    void onSendReply();
    void onReplySent();
    void onSessionMessage(const ProtocolMessage &msg);
    void pollForNewReplies();

private:
    void setupUi();
    void renderRootMessage(const ProtocolMessage &msg);
    void addReplyBubble(const ProtocolMessage &msg);
    void updateReplyCount();
    void setLoading(bool loading);
    QNetworkRequest makeRequest(const QString &path) const;
    ProtocolMessage parseEvent(const QJsonObject &event) const;
    QString generateTxnId();

    QString m_rootMessageId;
    QString m_roomId;
    MatrixSession *m_session;
    QNetworkAccessManager *m_nam;

    // UI
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QPushButton *m_backButton;
    QLabel *m_replyCountLabel;
    QPushButton *m_viewInRoomButton;
    LoadingSpinner *m_spinner;
    QFrame *m_rootFrame;
    QVBoxLayout *m_rootLayout;
    QScrollArea *m_repliesScroll;
    QWidget *m_repliesContent;
    QVBoxLayout *m_repliesLayout;
    QTextEdit *m_replyInput;
    QPushButton *m_sendButton;

    // Data
    QJsonObject m_rootEvent;
    QVector<QJsonObject> m_replyEvents;
    int m_replyCount = 0;
    QSet<QString> m_seenReplies;
    QTimer *m_pollTimer;
    int m_txnCounter = 0;
    QString m_lastFetchedEventId;
};
