#pragma once

#include <QWidget>
#include <QVector>
#include <QString>

class QTabWidget;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QTimer;
class QNetworkAccessManager;
class QProgressBar;

struct LemmyNotification
{
    enum NotifType { Reply, Mention, PrivateMessage, Unknown };

    NotifType type = Unknown;
    qint64 id = 0;
    QString creatorName;
    QString creatorAvatarUrl;
    qint64 creatorId = 0;
    QString contentPreview;
    qint64 timestampSec = 0;
    bool read = false;

    qint64 postId = 0;
    qint64 commentId = 0;
    qint64 privateMessageId = 0;
    QString communityName;
};

class LemmyNotificationsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LemmyNotificationsWidget(QWidget *parent = nullptr);
    ~LemmyNotificationsWidget() override;

    void setInstanceUrl(const QString &url);
    void setAuthToken(const QString &jwt);
    void startAutoRefresh();

    int totalUnreadCount() const;

signals:
    void notificationClicked(const QString &type, qint64 id);

public slots:
    void refreshNotifications();

private slots:
    void onTabChanged(int index);
    void onNotificationActivated(QListWidgetItem *item);
    void onMarkAllReadClicked();

private:
    void setupUi();
    void fetchMentions();
    void fetchReplies();
    void fetchPrivateMessages();
    void mergeAndPopulateAllTab();
    void populateTab(int tabIndex, const QVector<LemmyNotification> &notifications);
    void updateTabBadges();
    void markAsRead(const LemmyNotification &notif);

    QWidget *createNotificationItemWidget(const LemmyNotification &notif, int dataIndex);

    QString m_instanceUrl;
    QString m_jwt;

    QTabWidget *m_tabWidget;
    QListWidget *m_allList;
    QListWidget *m_repliesList;
    QListWidget *m_mentionsList;
    QListWidget *m_messagesList;
    QPushButton *m_markAllReadButton;
    QProgressBar *m_loadingBar;
    QLabel *m_emptyLabel;
    QTimer *m_refreshTimer;
    QNetworkAccessManager *m_nam;

    QVector<LemmyNotification> m_allNotifications;
    QVector<LemmyNotification> m_replyNotifications;
    QVector<LemmyNotification> m_mentionNotifications;
    QVector<LemmyNotification> m_messageNotifications;

    enum ItemDataRole {
        NotifIndexRole = Qt::UserRole + 1,
        NotifTypeRole
    };
};
