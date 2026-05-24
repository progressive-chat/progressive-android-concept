#pragma once

#include <QWidget>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QPropertyAnimation>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QVBoxLayout;
class QScrollArea;

struct PinnedMessageInfo
{
    QString eventId;
    QString senderId;
    QString senderName;
    QString senderAvatar;
    QString bodyPreview;
    QDateTime pinnedAt;
};

class PinnedMessagesWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal slideOffset READ slideOffset WRITE setSlideOffset)

public:
    explicit PinnedMessagesWidget(const QString &roomId, QWidget *parent = nullptr);
    ~PinnedMessagesWidget() override = default;

    void showAnimated();
    void hideAnimated();

    qreal slideOffset() const { return m_slideOffset; }
    void setSlideOffset(qreal offset);

signals:
    void messageSelected(const QString &messageId);
    void dismissed();

private slots:
    void onPinNewClicked();
    void onUnpinClicked(const QString &eventId);
    void onCloseClicked();
    void onItemClicked(QListWidgetItem *item);

private:
    void setupUi();
    void fetchPinnedMessages();
    void fetchPinnedEventDetail(const QString &eventId);
    QWidget *createMessageItemWidget(const PinnedMessageInfo &info);
    static QString truncateBody(const QString &body, int maxLen = 80);
    static QString initialsFromName(const QString &name);
    static QColor colorFromName(const QString &name);
    static QPixmap makeAvatarPixmap(const QString &name, int size, const QColor &bgColor);

    QString m_roomId;
    QListWidget *m_messageList = nullptr;
    QPushButton *m_pinNewButton = nullptr;
    QPushButton *m_closeButton = nullptr;
    QLabel *m_emptyLabel = nullptr;
    QLabel *m_headerLabel = nullptr;
    QNetworkAccessManager *m_nam = nullptr;
    QPropertyAnimation *m_animation = nullptr;
    QScrollArea *m_scrollArea = nullptr;

    QVector<PinnedMessageInfo> m_pinnedMessages;
    qreal m_slideOffset = 1.0;
    bool m_loading = false;
};
