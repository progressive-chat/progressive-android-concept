#pragma once

#include <QWidget>
#include <QString>
#include <QVector>
#include <QHash>
#include <QSet>
#include <QMap>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QMenu;
class QNetworkAccessManager;
class QNetworkReply;
class LoadingSpinner;

struct MemberInfo
{
    QString userId;
    QString displayName;
    QString avatarUrl;
    int powerLevel = 0;       // ≥ 100 → admin, ≥ 50 → mod, < 50 → user
    bool isOnline = false;
    QString presence;         // "online", "unavailable", "offline"
};

class RoomMembersList : public QWidget
{
    Q_OBJECT

public:
    explicit RoomMembersList(const QString &roomId, QWidget *parent = nullptr);
    ~RoomMembersList() override = default;

    void refresh();

signals:
    void chatStarted(const QString &userId);

private slots:
    void onSearchTextChanged(const QString &text);
    void onInviteClicked();
    void onMemberClicked(QListWidgetItem *item);
    void onContextMenuRequested(const QPoint &pos);

private:
    struct MemberItemWidget;

    void setupUi();
    void fetchMembers();
    void fetchPowerLevels();
    void fetchPresence(int memberIndex);
    void finishLoading();
    void updateList();
    void updateCountLabel();
    bool isOwnPowerLevelAtLeast(int requiredLevel) const;
    void kickMember(const QString &userId);
    void banMember(const QString &userId);
    void ignoreUser(const QString &userId);
    void toggleModAdmin(const QString &userId, int currentLevel);
    QWidget *createMemberItemWidget(const MemberInfo &member, int index);
    static QColor presenceColor(const QString &presence);
    static QColor powerLevelColor(int powerLevel);
    static QString powerLevelBadge(int powerLevel);

    QString m_roomId;
    int m_ownPowerLevel = 0;

    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_memberList = nullptr;
    QPushButton *m_inviteButton = nullptr;
    QLabel *m_countLabel = nullptr;
    QLabel *m_errorLabel = nullptr;
    LoadingSpinner *m_spinner = nullptr;
    QNetworkAccessManager *m_nam = nullptr;
    QMenu *m_contextMenu = nullptr;

    QVector<MemberInfo> m_members;
    QVector<MemberInfo> m_filteredMembers;
    QHash<QString, int> m_powerLevelCache;
    int m_presencePending = 0;
    bool m_loading = false;
};
