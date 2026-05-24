#pragma once

#include <QWidget>
#include <QVector>
#include "protocol/protocol_message.hpp"
#include "protocol/protocol_room.hpp"
#include "protocol/protocol_type.hpp"

class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QScrollBar;
class QTextEdit;
class QPushButton;
class QLabel;
class QTimer;

class ChatDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatDetailWidget(QWidget *parent = nullptr);
    ~ChatDetailWidget() override;

public slots:
    void loadRoom(const ProtocolRoom &room);
    void addMessage(const ProtocolMessage &msg);

signals:
    void sendMessage(const QString &roomId, const QString &text, ProtocolContentType contentType);
    void backRequested();

private slots:
    void onSendClicked();
    void onEmojiClicked();
    void onInputChanged();
    void onTypingTimeout();
    void onScrollChanged(int value);
    void onLoadEarlierClicked();

private:
    void setupUi();
    QWidget *createMessageBubble(const ProtocolMessage &msg);
    void appendSystemMessage(const QString &text, qint64 timestamp);
    void appendTimestampDivider(qint64 timestamp);
    bool shouldShowTimestampDivider(qint64 current, qint64 previous) const;
    QString formatTimestamp(qint64 timestamp) const;
    QString initialsFromName(const QString &name) const;
    QString avatarColorForSender(const QString &senderId) const;
    QString simpleMarkdownToHtml(const QString &text) const;
    void scrollToBottom();
    bool isNearBottom() const;

    QScrollArea *m_scrollArea;
    QWidget *m_messageContainer;
    QVBoxLayout *m_messageLayout;
    QPushButton *m_loadEarlierButton;
    QLabel *m_typingIndicator;
    QTextEdit *m_inputEdit;
    QPushButton *m_sendButton;
    QPushButton *m_emojiButton;
    QTimer *m_typingTimer;

    QString m_currentRoomId;
    ProtocolType m_protocolType;
    QString m_currentUserId;
    qint64 m_lastDisplayedTimestamp;
};
