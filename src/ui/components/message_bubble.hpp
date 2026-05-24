#pragma once

#include <QWidget>
#include "protocol/protocol_message.hpp"

class QLabel;
class QHBoxLayout;
class QVBoxLayout;
class AvatarWidget;

class MessageBubble : public QWidget
{
    Q_OBJECT

public:
    explicit MessageBubble(const ProtocolMessage &msg, QWidget *parent = nullptr);

    void setSentByMe(bool sent);
    bool isSentByMe() const;

    QSize sizeHint() const override;

    static constexpr int kMaxBubbleWidth = 400;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void rebuild(const ProtocolMessage &msg);
    void rebuildSystem(const QString &text);
    void rebuildNormal();

    static QString formatTimestamp(qint64 timestamp);
    static QString simpleMarkdownToHtml(const QString &text);

    ProtocolMessage m_message;
    bool m_sentByMe;

    // Shared
    QVBoxLayout *m_mainLayout;

    // System
    QLabel *m_systemLabel;

    // Normal
    QWidget *m_normalRow;
    QHBoxLayout *m_normalLayout;
    AvatarWidget *m_avatar;
    QWidget *m_contentColumn;
    QVBoxLayout *m_contentLayout;
    QLabel *m_senderLabel;
    QWidget *m_bubbleBackground;
    QVBoxLayout *m_bubbleLayout;
    QLabel *m_bodyLabel;
    QLabel *m_timestampLabel;
    QLabel *m_editedLabel;
    QLabel *m_encryptedLabel;
    QHBoxLayout *m_metadataLayout;
};
